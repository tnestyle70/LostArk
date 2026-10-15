# Data 솔루션 표시 통일 PLAN

## 현재 상태와 적용 범위

현재 루트 `Data`의 일부 파일은 Client의 가상 `96.DataFiles` 필터에 `None`으로 연결돼 있다.
사용자는 솔루션 직속에서 물리 폴더 이름과 같은 `Data` 트리를 보고자 한다. 이번 변경은 파일을
이동하거나 게시하지 않고 `Framework.sln`의 SolutionFolder/SolutionItems로 탐색 링크를 통일한다.
`Client/Bin/DataFiles`, `Server/Bin/DataFiles`, Resources와 빌드 출력은 이 트리에 넣지 않는다.
루트 Data에도 일부 파생 자료가 있으므로 표시가 모든 파일의 직접 편집 권한을 뜻하지는 않는다.

기존 Client-only 등록 규칙은 이번 명시 요청에 맞게 갱신한다. 공용 문서 변경은 root 담당이며
AGENTS.md, CLAUDE.md, TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md, BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md,
발탄인수인계서.md, Tools/Build/README.md의 현재 안내를 바꾼다. 과거 10-07 감사 RESULT는 유지한다.

## 새 도구와 자료 흐름

`Tools/Build/data_solution.py`는 Git 추적 파일과 무시되지 않은 Data 파일을 열거하고,
생성 도구가 아직 쓰지 않은 후보 경로를 합친다. writer lock, tmp, backup, previous는 탐색에서만
제외한다. 경로 집합은 중복 제거를 맡고 정렬된 목록은 출력 순서를 고정하며, 폴더별 파일 목록과
상대 경로로 만든 uuid5는 같은 폴더의 ID를 다음 갱신에도 유지한다.

`stage_data_solution(root, paths)`는 `(Path, before bytes, after bytes)` 목록만 반환하고 파일을
쓰지 않는다. 기존 생성 도구는 반환값을 자기 후보 transaction에 합치고 commit 직전 원본 비교와
기존 backup/원자 교체/rollback을 유지한다. CLI는 한 sln의 원본을 재확인한 뒤 같은 폴더의 임시
파일을 원자 교체한다. `--check`는 쓰지 않고 최신 상태면 0, 갱신이 필요하면 1을 반환한다.

관리하는 Data 폴더와 부모 연결만 교체한다. 실제 프로젝트 GUID, 빌드 구성, 다른 솔루션 항목과
섹션은 보존한다. GUID 충돌·해석할 수 없는 관리 블록·동시 변경은 오류로 멈추며 기존 파일을 유지한다.
Client의 root Data None 항목과 96.DataFiles 정의/연결만 제거한다. 기존 None에는 복사·빌드
메타데이터가 없으며 새 C++ 파일이나 C++ 프로젝트 등록 변경은 없다.

## 반영 순서와 검증

1. 아래 공용 도구 전체 코드와 sln/project/filter 완전 교체본을 준비하고 기존 파일 hash를 기록한다.
2. 생성 도구의 기존 등록 함수 API는 유지하며 반환 후보를 Framework.sln으로 옮긴다. 각 기존
   transaction의 freshness·rollback을 유지하고 source-contract 검사를 새 탐색 계약으로 바꾼다.
3. 반영 직전 baseline bytes를 다시 확인하고 후보를 적용한다. 소스 Data와 runtime 파일은 바꾸지 않는다.
4. 솔루션 파싱, 상대 링크의 실제 파일 존재, 중복 0, 비Data 프로젝트/빌드 맵 보존, XML 유효성과
   Data None/96 필터 외 모든 XML item·속성 보존, 두 번째 생성의 멱등성과 --check를 확인한다.
5. 격리 fixture로 신규/삭제 파일, 후보 경로, 임시 파일 제외, 비Data SolutionItems 보존,
   실패 입력과 동시 수정 거부를 확인한다. 가능하면 MSBuild SolutionFile.Parse와 getProperty만
   실행한다. 이 변경 검증은 빌드·Client 실행·UI 조작을 수행하지 않는다.

## Tools/Build/data_solution.py 전체 코드

변경 종류: 새 파일. CLI와 생성 도구가 함께 쓰는 순수 후보 생성 경계다.

```python
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

```

## Framework.sln 완전 교체본

변경 종류: 기존 파일 전체 교체. 위치: 파일의 처음부터 마지막 줄까지. 무관한 기존 내용은 보존한다.

```text

Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 18
VisualStudioVersion = 18.10.12120.281 insiders
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Client", "Client\Default\Client.vcxproj", "{1A5672C6-54B4-4779-99E7-84EF85171388}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Engine", "Engine\Default\Engine.vcxproj", "{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Shared", "Shared\Default\Shared.vcxproj", "{F4CCF815-6D51-412F-A76E-84D2F1D05571}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "NetworkProtocolHarness", "Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj", "{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "CharacterSelectIsolationHarness", "Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj", "{D9E5A74B-70B1-41DF-8871-118120CBC7CD}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Server", "Server\Default\Server.vcxproj", "{053788E7-C377-4811-8AFE-BC23D9BE4AE7}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "PhysicsContractHarness", "Tools\PhysicsContractHarness\Default\PhysicsContractHarness.vcxproj", "{AB5658F8-984D-4826-8F05-5044E1B12BAF}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "WModelGeometryContractHarness", "Tools\WModelGeometryContractHarness\Default\WModelGeometryContractHarness.vcxproj", "{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "PointLightFalloffContractHarness", "Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj", "{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "BossToolTests", "Tools\BossToolTests\Default\BossToolTests.vcxproj", "{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Data", "Data", "{5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Actors", "Actors", "{485FA94D-8E70-5F3F-8221-DDCD06A724DC}"
	ProjectSection(SolutionItems) = preProject
		Data\Actors\BossCatalog.json = Data\Actors\BossCatalog.json
		Data\Actors\CharacterCatalog.json = Data\Actors\CharacterCatalog.json
		Data\Actors\EquipmentLoadoutPresets.json = Data\Actors\EquipmentLoadoutPresets.json
		Data\Actors\EquipmentPresentationCatalog.json = Data\Actors\EquipmentPresentationCatalog.json
		Data\Actors\MaharakaResidents.source.json = Data\Actors\MaharakaResidents.source.json
		Data\Actors\MonsterCatalog.json = Data\Actors\MonsterCatalog.json
		Data\Actors\NpcCatalog.json = Data\Actors\NpcCatalog.json
		Data\Actors\SquareHoleInstruments.json = Data\Actors\SquareHoleInstruments.json
		Data\Actors\VehicleCatalog.json = Data\Actors\VehicleCatalog.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "AI", "AI", "{1BB87F58-65F6-568F-9191-AF20466FFE4A}"
	ProjectSection(SolutionItems) = preProject
		Data\AI\MaharakaWaterpangAI.json = Data\AI\MaharakaWaterpangAI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Animation", "Animation", "{029209C1-5EC5-55C5-B168-71F24E43D964}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Authored", "Authored", "{7B5F7812-C61A-5748-886D-DE465DDD2C87}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Artist", "Artist", "{3510878C-9EE6-5921-8C3E-412CC687F2FC}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\Artist\Artist.animevents = Data\Animation\Authored\Artist\Artist.animevents
		Data\Animation\Authored\Artist\Artist.interactionbindings.json = Data\Animation\Authored\Artist\Artist.interactionbindings.json
		Data\Animation\Authored\Artist\Artist.projectiles.json = Data\Animation\Authored\Artist\Artist.projectiles.json
		Data\Animation\Authored\Artist\Artist.skillbindings.json = Data\Animation\Authored\Artist\Artist.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "DimensionMaster", "DimensionMaster", "{F2E070DE-E1E8-5763-97F2-324A6887A25C}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents = Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents
		Data\Animation\Authored\DimensionMaster\DimensionMaster.interactionbindings.json = Data\Animation\Authored\DimensionMaster\DimensionMaster.interactionbindings.json
		Data\Animation\Authored\DimensionMaster\DimensionMaster.projectiles.json = Data\Animation\Authored\DimensionMaster\DimensionMaster.projectiles.json
		Data\Animation\Authored\DimensionMaster\DimensionMaster.skillbindings.json = Data\Animation\Authored\DimensionMaster\DimensionMaster.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "GuardianKnight", "GuardianKnight", "{E1444538-B294-5CA1-BD6C-0876C599EC31}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\GuardianKnight\GuardianKnight.animevents = Data\Animation\Authored\GuardianKnight\GuardianKnight.animevents
		Data\Animation\Authored\GuardianKnight\GuardianKnight.interactionbindings.json = Data\Animation\Authored\GuardianKnight\GuardianKnight.interactionbindings.json
		Data\Animation\Authored\GuardianKnight\GuardianKnight.skillbindings.json = Data\Animation\Authored\GuardianKnight\GuardianKnight.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "GunSlinger", "GunSlinger", "{672FAC17-1563-5D99-9113-D875472EF00E}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\GunSlinger\GunSlinger.animevents = Data\Animation\Authored\GunSlinger\GunSlinger.animevents
		Data\Animation\Authored\GunSlinger\GunSlinger.interactionbindings.json = Data\Animation\Authored\GunSlinger\GunSlinger.interactionbindings.json
		Data\Animation\Authored\GunSlinger\GunSlinger.skillbindings.json = Data\Animation\Authored\GunSlinger\GunSlinger.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KoukuSaydon", "KoukuSaydon", "{E43A8D01-3CF6-5D80-917A-5E57F7620B2C}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\KoukuSaydon\Clown.interactionbindings.json = Data\Animation\Authored\KoukuSaydon\Clown.interactionbindings.json
		Data\Animation\Authored\KoukuSaydon\KoukuSaydon.patternbindings.json = Data\Animation\Authored\KoukuSaydon\KoukuSaydon.patternbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_05.actionbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_05.actionbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_05.patternbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_05.patternbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_06.actionbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_06.actionbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_06.patternbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_06.patternbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_07.actionbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_07.actionbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCT_07.patternbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCT_07.patternbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCZ_00.actionbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCZ_00.actionbindings.json
		Data\Animation\Authored\KoukuSaydon\MN_RPCZ_00.patternbindings.json = Data\Animation\Authored\KoukuSaydon\MN_RPCZ_00.patternbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LanceMaster", "LanceMaster", "{C6586D75-3D30-5265-9E12-EBDDE052F569}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\LanceMaster\LanceMaster.animevents = Data\Animation\Authored\LanceMaster\LanceMaster.animevents
		Data\Animation\Authored\LanceMaster\LanceMaster.interactionbindings.json = Data\Animation\Authored\LanceMaster\LanceMaster.interactionbindings.json
		Data\Animation\Authored\LanceMaster\LanceMaster.projectiles.json = Data\Animation\Authored\LanceMaster\LanceMaster.projectiles.json
		Data\Animation\Authored\LanceMaster\LanceMaster.skillbindings.json = Data\Animation\Authored\LanceMaster\LanceMaster.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Slayer", "Slayer", "{484244C8-9459-58F3-83CC-016BA027239F}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\Slayer\Slayer.animevents = Data\Animation\Authored\Slayer\Slayer.animevents
		Data\Animation\Authored\Slayer\Slayer.interactionbindings.json = Data\Animation\Authored\Slayer\Slayer.interactionbindings.json
		Data\Animation\Authored\Slayer\Slayer.skillbindings.json = Data\Animation\Authored\Slayer\Slayer.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Valtan", "Valtan", "{26886C0C-025B-54D9-85B6-591B251C85F6}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\Valtan\Valtan.actionbindings.json = Data\Animation\Authored\Valtan\Valtan.actionbindings.json
		Data\Animation\Authored\Valtan\Valtan.actionbindings.receipt.json = Data\Animation\Authored\Valtan\Valtan.actionbindings.receipt.json
		Data\Animation\Authored\Valtan\Valtan.combatobjectsoundcues.json = Data\Animation\Authored\Valtan\Valtan.combatobjectsoundcues.json
		Data\Animation\Authored\Valtan\Valtan.pattern-occurrence-v2-migration.receipt.json = Data\Animation\Authored\Valtan\Valtan.pattern-occurrence-v2-migration.receipt.json
		Data\Animation\Authored\Valtan\Valtan.patternbindings.json = Data\Animation\Authored\Valtan\Valtan.patternbindings.json
		Data\Animation\Authored\Valtan\Valtan.patterneffectcues.json = Data\Animation\Authored\Valtan\Valtan.patterneffectcues.json
		Data\Animation\Authored\Valtan\Valtan.patterneffects.json = Data\Animation\Authored\Valtan\Valtan.patterneffects.json
		Data\Animation\Authored\Valtan\Valtan.patterneffectv1aliases.json = Data\Animation\Authored\Valtan\Valtan.patterneffectv1aliases.json
		Data\Animation\Authored\Valtan\Valtan.patternpreview.json = Data\Animation\Authored\Valtan\Valtan.patternpreview.json
		Data\Animation\Authored\Valtan\Valtan.patternshakecues.json = Data\Animation\Authored\Valtan\Valtan.patternshakecues.json
		Data\Animation\Authored\Valtan\Valtan.patternsoundcues.json = Data\Animation\Authored\Valtan\Valtan.patternsoundcues.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Vehicle_9523", "Vehicle_9523", "{2DAB4577-F73D-5A87-AE1D-B5F96BCF9CCF}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\Vehicle_9523\Vehicle_9523.boneclips.json = Data\Animation\Authored\Vehicle_9523\Vehicle_9523.boneclips.json
		Data\Animation\Authored\Vehicle_9523\Vehicle_9523.modelactions.json = Data\Animation\Authored\Vehicle_9523\Vehicle_9523.modelactions.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Warlord", "Warlord", "{27581604-0006-54AD-8451-23924C4866B8}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Authored\Warlord\Warlord.animevents = Data\Animation\Authored\Warlord\Warlord.animevents
		Data\Animation\Authored\Warlord\Warlord.interactionbindings.json = Data\Animation\Authored\Warlord\Warlord.interactionbindings.json
		Data\Animation\Authored\Warlord\Warlord.projectiles.json = Data\Animation\Authored\Warlord\Warlord.projectiles.json
		Data\Animation\Authored\Warlord\Warlord.skillbindings.json = Data\Animation\Authored\Warlord\Warlord.skillbindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "CancelWindows", "CancelWindows", "{3B2E1995-9C50-55D2-B28D-078CE382FA77}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\CancelWindows\Artist.cancelwindows.json = Data\Animation\CancelWindows\Artist.cancelwindows.json
		Data\Animation\CancelWindows\DimensionMaster.cancelwindows.json = Data\Animation\CancelWindows\DimensionMaster.cancelwindows.json
		Data\Animation\CancelWindows\GuardianKnight.cancelwindows.json = Data\Animation\CancelWindows\GuardianKnight.cancelwindows.json
		Data\Animation\CancelWindows\LanceMaster.cancelwindows.json = Data\Animation\CancelWindows\LanceMaster.cancelwindows.json
		Data\Animation\CancelWindows\Warlord.cancelwindows.json = Data\Animation\CancelWindows\Warlord.cancelwindows.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "HitShapes", "HitShapes", "{9F1E4AEF-9194-506F-B1B9-67733D2628B9}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\HitShapes\Artist.hitshapes.json = Data\Animation\HitShapes\Artist.hitshapes.json
		Data\Animation\HitShapes\DimensionMaster.hitshapes.json = Data\Animation\HitShapes\DimensionMaster.hitshapes.json
		Data\Animation\HitShapes\GuardianKnight.hitshapes.json = Data\Animation\HitShapes\GuardianKnight.hitshapes.json
		Data\Animation\HitShapes\GunSlinger.hitshapes.json = Data\Animation\HitShapes\GunSlinger.hitshapes.json
		Data\Animation\HitShapes\LanceMaster.hitshapes.json = Data\Animation\HitShapes\LanceMaster.hitshapes.json
		Data\Animation\HitShapes\Slayer.hitshapes.json = Data\Animation\HitShapes\Slayer.hitshapes.json
		Data\Animation\HitShapes\Warlord.hitshapes.json = Data\Animation\HitShapes\Warlord.hitshapes.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Reference", "Reference", "{E3D17799-341C-58B1-AD9A-12131A5C70A1}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Artist", "Artist", "{4B8552BE-4F68-504B-9B48-05DFEF3BDA37}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\Artist\Artist.animnotify = Data\Animation\Reference\Artist\Artist.animnotify
		Data\Animation\Reference\Artist\Artist.basehits = Data\Animation\Reference\Artist\Artist.basehits
		Data\Animation\Reference\Artist\Artist.clipmap = Data\Animation\Reference\Artist\Artist.clipmap
		Data\Animation\Reference\Artist\Artist.clipseq = Data\Animation\Reference\Artist\Artist.clipseq
		Data\Animation\Reference\Artist\Artist.hitrepeats = Data\Animation\Reference\Artist\Artist.hitrepeats
		Data\Animation\Reference\Artist\Artist.projectiles = Data\Animation\Reference\Artist\Artist.projectiles
		Data\Animation\Reference\Artist\Artist.skilltiming = Data\Animation\Reference\Artist\Artist.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "DimensionMaster", "DimensionMaster", "{CF8105F5-B159-5425-A3E5-DC6C5CE420F9}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\DimensionMaster\DimensionMaster.animnotify = Data\Animation\Reference\DimensionMaster\DimensionMaster.animnotify
		Data\Animation\Reference\DimensionMaster\DimensionMaster.basehits = Data\Animation\Reference\DimensionMaster\DimensionMaster.basehits
		Data\Animation\Reference\DimensionMaster\DimensionMaster.clipmap = Data\Animation\Reference\DimensionMaster\DimensionMaster.clipmap
		Data\Animation\Reference\DimensionMaster\DimensionMaster.clipseq = Data\Animation\Reference\DimensionMaster\DimensionMaster.clipseq
		Data\Animation\Reference\DimensionMaster\DimensionMaster.hitrepeats = Data\Animation\Reference\DimensionMaster\DimensionMaster.hitrepeats
		Data\Animation\Reference\DimensionMaster\DimensionMaster.projectiles = Data\Animation\Reference\DimensionMaster\DimensionMaster.projectiles
		Data\Animation\Reference\DimensionMaster\DimensionMaster.skilltiming = Data\Animation\Reference\DimensionMaster\DimensionMaster.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "GuardianKnight", "GuardianKnight", "{F86E4F27-13C8-57FE-A818-A44FC30E00B7}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\GuardianKnight\GuardianKnight.animnotify = Data\Animation\Reference\GuardianKnight\GuardianKnight.animnotify
		Data\Animation\Reference\GuardianKnight\GuardianKnight.basehits = Data\Animation\Reference\GuardianKnight\GuardianKnight.basehits
		Data\Animation\Reference\GuardianKnight\GuardianKnight.clipmap = Data\Animation\Reference\GuardianKnight\GuardianKnight.clipmap
		Data\Animation\Reference\GuardianKnight\GuardianKnight.clipseq = Data\Animation\Reference\GuardianKnight\GuardianKnight.clipseq
		Data\Animation\Reference\GuardianKnight\GuardianKnight.hitrepeats = Data\Animation\Reference\GuardianKnight\GuardianKnight.hitrepeats
		Data\Animation\Reference\GuardianKnight\GuardianKnight.skilltiming = Data\Animation\Reference\GuardianKnight\GuardianKnight.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "GunSlinger", "GunSlinger", "{B9AA23DB-3A62-5018-914E-1553E3682B54}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\GunSlinger\GunSlinger.animnotify = Data\Animation\Reference\GunSlinger\GunSlinger.animnotify
		Data\Animation\Reference\GunSlinger\GunSlinger.clipmap = Data\Animation\Reference\GunSlinger\GunSlinger.clipmap
		Data\Animation\Reference\GunSlinger\GunSlinger.clipseq = Data\Animation\Reference\GunSlinger\GunSlinger.clipseq
		Data\Animation\Reference\GunSlinger\GunSlinger.skilltiming = Data\Animation\Reference\GunSlinger\GunSlinger.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KoukuSaydon", "KoukuSaydon", "{5032393B-C7F9-5F0A-A986-126A5A6B52AD}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\KoukuSaydon\KoukuSaydon.cinematicreference.json = Data\Animation\Reference\KoukuSaydon\KoukuSaydon.cinematicreference.json
		Data\Animation\Reference\KoukuSaydon\MN_RPCT_05.actionreference.json = Data\Animation\Reference\KoukuSaydon\MN_RPCT_05.actionreference.json
		Data\Animation\Reference\KoukuSaydon\MN_RPCT_06.actionreference.json = Data\Animation\Reference\KoukuSaydon\MN_RPCT_06.actionreference.json
		Data\Animation\Reference\KoukuSaydon\MN_RPCT_07.actionreference.json = Data\Animation\Reference\KoukuSaydon\MN_RPCT_07.actionreference.json
		Data\Animation\Reference\KoukuSaydon\MN_RPCZ_00.actionreference.json = Data\Animation\Reference\KoukuSaydon\MN_RPCZ_00.actionreference.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LanceMaster", "LanceMaster", "{736C79C4-3DE8-5052-8293-8DCEDA91D403}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\LanceMaster\LanceMaster.animnotify = Data\Animation\Reference\LanceMaster\LanceMaster.animnotify
		Data\Animation\Reference\LanceMaster\LanceMaster.basehits = Data\Animation\Reference\LanceMaster\LanceMaster.basehits
		Data\Animation\Reference\LanceMaster\LanceMaster.clipmap = Data\Animation\Reference\LanceMaster\LanceMaster.clipmap
		Data\Animation\Reference\LanceMaster\LanceMaster.clipseq = Data\Animation\Reference\LanceMaster\LanceMaster.clipseq
		Data\Animation\Reference\LanceMaster\LanceMaster.hitrepeats = Data\Animation\Reference\LanceMaster\LanceMaster.hitrepeats
		Data\Animation\Reference\LanceMaster\LanceMaster.projectiles = Data\Animation\Reference\LanceMaster\LanceMaster.projectiles
		Data\Animation\Reference\LanceMaster\LanceMaster.skilltiming = Data\Animation\Reference\LanceMaster\LanceMaster.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Slayer", "Slayer", "{53E67D93-5977-53A3-B92C-D1220E6D86CB}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\Slayer\Slayer.animnotify = Data\Animation\Reference\Slayer\Slayer.animnotify
		Data\Animation\Reference\Slayer\Slayer.clipmap = Data\Animation\Reference\Slayer\Slayer.clipmap
		Data\Animation\Reference\Slayer\Slayer.clipseq = Data\Animation\Reference\Slayer\Slayer.clipseq
		Data\Animation\Reference\Slayer\Slayer.skilltiming = Data\Animation\Reference\Slayer\Slayer.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Valtan", "Valtan", "{29C7BA96-5088-53C3-AEA0-18DF71436FE6}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\Valtan\Valtan.animevents = Data\Animation\Reference\Valtan\Valtan.animevents
		Data\Animation\Reference\Valtan\Valtan.animnotify = Data\Animation\Reference\Valtan\Valtan.animnotify
		Data\Animation\Reference\Valtan\Valtan.clipcuts = Data\Animation\Reference\Valtan\Valtan.clipcuts
		Data\Animation\Reference\Valtan\Valtan.clipmap = Data\Animation\Reference\Valtan\Valtan.clipmap
		Data\Animation\Reference\Valtan\Valtan.clipseq = Data\Animation\Reference\Valtan\Valtan.clipseq
		Data\Animation\Reference\Valtan\Valtan.skilltiming = Data\Animation\Reference\Valtan\Valtan.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Warlord", "Warlord", "{4D55ABB7-6016-50AB-BDC3-83751F679519}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\Reference\Warlord\Warlord.animnotify = Data\Animation\Reference\Warlord\Warlord.animnotify
		Data\Animation\Reference\Warlord\Warlord.basehits = Data\Animation\Reference\Warlord\Warlord.basehits
		Data\Animation\Reference\Warlord\Warlord.clipmap = Data\Animation\Reference\Warlord\Warlord.clipmap
		Data\Animation\Reference\Warlord\Warlord.clipseq = Data\Animation\Reference\Warlord\Warlord.clipseq
		Data\Animation\Reference\Warlord\Warlord.hitrepeats = Data\Animation\Reference\Warlord\Warlord.hitrepeats
		Data\Animation\Reference\Warlord\Warlord.projectiles = Data\Animation\Reference\Warlord\Warlord.projectiles
		Data\Animation\Reference\Warlord\Warlord.skilltiming = Data\Animation\Reference\Warlord\Warlord.skilltiming
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "RootMotion", "RootMotion", "{532DCCBC-1D56-54C9-9BA5-84980169EC9A}"
	ProjectSection(SolutionItems) = preProject
		Data\Animation\RootMotion\Artist.rootmotion.json = Data\Animation\RootMotion\Artist.rootmotion.json
		Data\Animation\RootMotion\DimensionMaster.rootmotion.json = Data\Animation\RootMotion\DimensionMaster.rootmotion.json
		Data\Animation\RootMotion\GuardianKnight.rootmotion.json = Data\Animation\RootMotion\GuardianKnight.rootmotion.json
		Data\Animation\RootMotion\LanceMaster.rootmotion.json = Data\Animation\RootMotion\LanceMaster.rootmotion.json
		Data\Animation\RootMotion\Valtan.rootmotion.json = Data\Animation\RootMotion\Valtan.rootmotion.json
		Data\Animation\RootMotion\Warlord.rootmotion.json = Data\Animation\RootMotion\Warlord.rootmotion.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Balance", "Balance", "{04A3B383-33FC-5B88-B264-2205B896651B}"
	ProjectSection(SolutionItems) = preProject
		Data\Balance\BossProfiles.json = Data\Balance\BossProfiles.json
		Data\Balance\DamageProfiles.json = Data\Balance\DamageProfiles.json
		Data\Balance\GuardianKnightEmber.json = Data\Balance\GuardianKnightEmber.json
		Data\Balance\MonsterProfiles.json = Data\Balance\MonsterProfiles.json
		Data\Balance\NumericSourceBindings.json = Data\Balance\NumericSourceBindings.json
		Data\Balance\PlayerProfiles.json = Data\Balance\PlayerProfiles.json
		Data\Balance\PlayerSkills.json = Data\Balance\PlayerSkills.json
		Data\Balance\PlayerSkillTargeting.json = Data\Balance\PlayerSkillTargeting.json
		Data\Balance\ValtanBossParts.json = Data\Balance\ValtanBossParts.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Profiles", "Profiles", "{4CC1AC32-F0E5-56B5-A3DE-3306AABE3301}"
	ProjectSection(SolutionItems) = preProject
		Data\Balance\Profiles\Retail.balanceprofile.json = Data\Balance\Profiles\Retail.balanceprofile.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Reference", "Reference", "{2CB9D6B9-80F6-5F54-9170-EFECAC1EEAE2}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Official", "Official", "{C1EB8271-A950-5494-9139-8B5122774DB7}"
	ProjectSection(SolutionItems) = preProject
		Data\Balance\Reference\Official\2026-08-05.balance-provenance.receipt.json = Data\Balance\Reference\Official\2026-08-05.balance-provenance.receipt.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Camera", "Camera", "{F7E333D9-9033-5E32-B8C3-332A19F21ADE}"
	ProjectSection(SolutionItems) = preProject
		Data\Camera\Bern.camera.json = Data\Camera\Bern.camera.json
		Data\Camera\CharacterSelect.camera.json = Data\Camera\CharacterSelect.camera.json
		Data\Camera\ClassSelection.cinematics.json = Data\Camera\ClassSelection.cinematics.json
		Data\Camera\ColosseumIntro.cutscene.json = Data\Camera\ColosseumIntro.cutscene.json
		Data\Camera\ColosseumMatchStart.json = Data\Camera\ColosseumMatchStart.json
		Data\Camera\ColosseumVictory.cutscene.json = Data\Camera\ColosseumVictory.cutscene.json
		Data\Camera\KoukuSaydon.camera.json = Data\Camera\KoukuSaydon.camera.json
		Data\Camera\Valtan.camera.json = Data\Camera\Valtan.camera.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Compositions", "Compositions", "{43DF313D-CACC-5B6E-936D-09C870B6DE8C}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Bosses", "Bosses", "{E9B9715E-E544-5424-AB66-A68B8503C3C5}"
	ProjectSection(SolutionItems) = preProject
		Data\Compositions\Bosses\KoukuSaydonGate1.bosscomposition.json = Data\Compositions\Bosses\KoukuSaydonGate1.bosscomposition.json
		Data\Compositions\Bosses\Valtan.bosscomposition.json = Data\Compositions\Bosses\Valtan.bosscomposition.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Sequences", "Sequences", "{FD63E535-93AF-5856-B06C-AD1E7C602A91}"
	ProjectSection(SolutionItems) = preProject
		Data\Compositions\Sequences\KoukuSaydonArena.sequencer.json = Data\Compositions\Sequences\KoukuSaydonArena.sequencer.json
		Data\Compositions\Sequences\KoukuSaydonSequenceComposition.json = Data\Compositions\Sequences\KoukuSaydonSequenceComposition.json
		Data\Compositions\Sequences\ValtanArena.sequencer.json = Data\Compositions\Sequences\ValtanArena.sequencer.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Customizing", "Customizing", "{B7442E27-10F7-5B9E-9E4B-22EB6E2B06E9}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "FaceSliders", "FaceSliders", "{83B3A406-9FEE-5264-8F2B-D2F84FCC9527}"
	ProjectSection(SolutionItems) = preProject
		Data\Customizing\FaceSliders\DK.facesliders.json = Data\Customizing\FaceSliders\DK.facesliders.json
		Data\Customizing\FaceSliders\FT.facesliders.json = Data\Customizing\FaceSliders\FT.facesliders.json
		Data\Customizing\FaceSliders\SP.facesliders.json = Data\Customizing\FaceSliders\SP.facesliders.json
		Data\Customizing\FaceSliders\SP_M.facesliders.json = Data\Customizing\FaceSliders\SP_M.facesliders.json
		Data\Customizing\FaceSliders\WR.facesliders.json = Data\Customizing\FaceSliders\WR.facesliders.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Effects", "Effects", "{59E6F36E-AA3E-5AB6-91AA-389AA6955725}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\EffectAuditionCatalog.json = Data\Effects\EffectAuditionCatalog.json
		Data\Effects\EffectCatalog.json = Data\Effects\EffectCatalog.json
		Data\Effects\EffectResourceTree.json = Data\Effects\EffectResourceTree.json
		Data\Effects\ValtanFullRestoreAnimations.json = Data\Effects\ValtanFullRestoreAnimations.json
		Data\Effects\ValtanPatternAuthoringEffects.json = Data\Effects\ValtanPatternAuthoringEffects.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Authored", "Authored", "{802F9714-C0A7-5172-AC1E-F20BCC23B277}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\Authored\effect.artist.skill.31000.ba1.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba1.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba1.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba2.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba2.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba2.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba3.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba3.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba3.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba3.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba4.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba4.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31000.ba4.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31000.ba4.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31050.clip1.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31050.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31050.clip1.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31050.clip1.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31050.clip2.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31050.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31050.clip2.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31050.clip2.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31200.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31200.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31200.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31200.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31210.ba1.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31210.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31210.ba1.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31210.ba1.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31210.ba4.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31210.ba4.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31210.ba4.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31210.ba4.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31420.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31420.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31420.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31420.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31430.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31430.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31430.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31430.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31460.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31460.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31460.linear-reveal.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31460.linear-reveal.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31470.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31470.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31470.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31470.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31480.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31480.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31480.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31480.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31490.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31490.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31490.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31490.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31910.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31910.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31910.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31910.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31930.clip1.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31930.clip1.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31930.clip2.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31930.clip2.unified.effect.json
		Data\Effects\Authored\effect.artist.skill.31930.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31930.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31950.full.restore.effect.json = Data\Effects\Authored\effect.artist.skill.31950.full.restore.effect.json
		Data\Effects\Authored\effect.artist.skill.31950.unified.effect.json = Data\Effects\Authored\effect.artist.skill.31950.unified.effect.json
		Data\Effects\Authored\effect.bern.anchor.marker.marker.full.restore.effect.json = Data\Effects\Authored\effect.bern.anchor.marker.marker.full.restore.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_a_h_waterwave_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_a_h_waterwave_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_b_waterfall_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_b_waterfall_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_b_waterfall_002.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_b_waterfall_002.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_c_fallmist_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_c_fallmist_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fall_w1_h1_002.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fall_w1_h1_002.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallmist_w3_002.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallmist_w3_002.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w1_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w1_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w2_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w2_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w3_001.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w3_001.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w3_007.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_06.water.par_d_fallsplash_w3_007.effect.json
		Data\Effects\Authored\effect.bern.environment.bfx_low_16.water.par_g_magicpond_01.effect.json = Data\Effects\Authored\effect.bern.environment.bfx_low_16.water.par_g_magicpond_01.effect.json
		Data\Effects\Authored\effect.bern.harbor.dockingvolume.marker.full.restore.effect.json = Data\Effects\Authored\effect.bern.harbor.dockingvolume.marker.full.restore.effect.json
		Data\Effects\Authored\effect.classselect.artist.bfx_high_00.etc.par_d_dropink_01.effect.json = Data\Effects\Authored\effect.classselect.artist.bfx_high_00.etc.par_d_dropink_01.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_cm_03.trail.par_d_mptrail_03.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_cm_03.trail.par_d_mptrail_03.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_b_crane_01.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_b_crane_01.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_b_crane_04.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_b_crane_04.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_d_water_001_pr.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_cm_03.water.par_d_water_001_pr.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_05.par_y_sdm_moondrawing_01_03.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_05.par_y_sdm_moondrawing_01_03.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_06.par_d_sdm_graffiti_05_rib.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_06.par_d_sdm_graffiti_05_rib.effect.json
		Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_06.par_d_sdm_pond_02.effect.json = Data\Effects\Authored\effect.classselect.artist.fx_pc_sdm_06.par_d_sdm_pond_02.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a.fx.par_o_butterflydream_03_04_cine.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a.fx.par_o_butterflydream_03_04_cine.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.fx_n_sdm_flower_cine.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.fx_n_sdm_flower_cine.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_c_lightflare_09_cine.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_c_lightflare_09_cine.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_d_dropink_01_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_d_dropink_01_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_d_sdm_pond_71_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_d_sdm_pond_71_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_n_sdm_circle_01_01_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_n_sdm_circle_01_01_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_o_butterflydream_02_03_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_o_butterflydream_02_03_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_w_sdm_four_02_1_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_w_sdm_four_02_1_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_w_sdm_four_02_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_w_sdm_four_02_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_y_sdm_moondrawing_01_01_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_y_sdm_moondrawing_01_01_sp.effect.json
		Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_y_sdm_moondrawing_01_02_sp.effect.json = Data\Effects\Authored\effect.classselect.artist.scene_a_lobby.sp.par_y_sdm_moondrawing_01_02_sp.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.psc1728.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.psc1728.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.psc1729.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.bfx_low_13.leaf.par_l_sakura_01.psc1729.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_00.par_j_swp_jump_00_1.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_00.par_j_swp_jump_00_1.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_00.par_j_swp_jump_00_2.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_00.par_j_swp_jump_00_2.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_01.par_w_swp_jump_01_7.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_01.par_w_swp_jump_01_7.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_05.par_k_swp_instance_cam_03.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_05.par_k_swp_instance_cam_03.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_05.par_k_swp_tentdevider_flare_00_01.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.fx_pc_swp_05.par_k_swp_tentdevider_flare_00_01.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_j_swp_willowrend_swing_dt.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_j_swp_willowrend_swing_dt.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_q_shieldbreak_dt.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_q_shieldbreak_dt.effect.json
		Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_q_shieldbreak_dt.psc1724.effect.json = Data\Effects\Authored\effect.classselect.dimensionmaster.scene_d.fx.par_q_shieldbreak_dt.psc1724.effect.json
		Data\Effects\Authored\effect.classselect.guardianknight.fx_pc_ddk_04.par_m_ddk_guillotinespin_trail_01.effect.json = Data\Effects\Authored\effect.classselect.guardianknight.fx_pc_ddk_04.par_m_ddk_guillotinespin_trail_01.effect.json
		Data\Effects\Authored\effect.classselect.guardianknight.fx_post.fx_par.par_c_zoomblur_01.effect.json = Data\Effects\Authored\effect.classselect.guardianknight.fx_post.fx_par.par_c_zoomblur_01.effect.json
		Data\Effects\Authored\effect.classselect.guardianknight.fx_q_w_03.fx_par_01.par_q_pcselect_floatingember_01.effect.json = Data\Effects\Authored\effect.classselect.guardianknight.fx_q_w_03.fx_par_01.par_q_pcselect_floatingember_01.effect.json
		Data\Effects\Authored\effect.classselect.guardianknight.fx_q_w_03.fx_par_01.par_q_watersplash_01.effect.json = Data\Effects\Authored\effect.classselect.guardianknight.fx_q_w_03.fx_par_01.par_q_watersplash_01.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_high_01._pcselect.par_d_pcselect_fighteraura_01.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_high_01._pcselect.par_d_pcselect_fighteraura_01.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_high_01._pcselect.par_d_pcselect_fighteraura_01.psc1135.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_high_01._pcselect.par_d_pcselect_fighteraura_01.psc1135.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1557.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1557.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1568.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1568.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1601.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_00.dust.par_d_pcselect_duststop_01.psc1601.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1246.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1246.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1379.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1379.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1646.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001.psc1646.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_03.lensflare.par_c_lightflare_07.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bfx_low_03.lensflare.par_c_lightflare_07.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_03.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_03.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_12_1.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_12_1.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_13.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.bg_pcselect02.particle.par_d_pcselect_13.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_cm_00.dust.par_d_dust_007_pr.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_cm_00.dust.par_d_dust_007_pr.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_cm_00.dust.par_e_underdust_03_2.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_cm_00.dust.par_e_underdust_03_2.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_pc_fif_01.par_j_fif_ironfist_02_2.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_pc_fif_01.par_j_fif_ironfist_02_2.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_pc_fif_01.par_s_fif_earthqs_exp_01_2.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_pc_fif_01.par_s_fif_earthqs_exp_01_2.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrass.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrass.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrass.psc1190.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrass.psc1190.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1068.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1068.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1079.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1079.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1113.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01.psc1113.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1057.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1057.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1091.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1091.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1102.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1102.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1146.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_01.psc1146.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1124.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1124.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1235.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1235.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1257.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.fx_q_w_01.fx_par.par_q_attackgrassstop_01_02.psc1257.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.dr.par_d_pcselect_floatingember_02_cine.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.dr.par_d_pcselect_floatingember_02_cine.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_j_vs_dusts_01_cine.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_j_vs_dusts_01_cine.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.psc913.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.psc913.effect.json
		Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.psc968.effect.json = Data\Effects\Authored\effect.classselect.lancemaster.scene_a_lobby.ft.par_o_fif_roundup_01_02_cine.psc968.effect.json
		Data\Effects\Authored\effect.classselect.warlord.bfx_high_01._pcselect.par_d_pcselect_floatingember_01.effect.json = Data\Effects\Authored\effect.classselect.warlord.bfx_high_01._pcselect.par_d_pcselect_floatingember_01.effect.json
		Data\Effects\Authored\effect.classselect.warlord.bfx_high_01.sparkling.par_d_kayangel_spark_02.effect.json = Data\Effects\Authored\effect.classselect.warlord.bfx_high_01.sparkling.par_d_kayangel_spark_02.effect.json
		Data\Effects\Authored\effect.classselect.warlord.bfx_low_05.snow.par_c_blizzard_002.effect.json = Data\Effects\Authored\effect.classselect.warlord.bfx_low_05.snow.par_c_blizzard_002.effect.json
		Data\Effects\Authored\effect.classselect.warlord.fx_pc_gst_00.par_k_gst_airstrike_exp_01.effect.json = Data\Effects\Authored\effect.classselect.warlord.fx_pc_gst_00.par_k_gst_airstrike_exp_01.effect.json
		Data\Effects\Authored\effect.classselect.warlord.fx_pc_msm_01.par_k_msm_arcanebolter_exp_08_01.effect.json = Data\Effects\Authored\effect.classselect.warlord.fx_pc_msm_01.par_k_msm_arcanebolter_exp_08_01.effect.json
		Data\Effects\Authored\effect.classselect.warlord.fx_pc_msm_02.par_c_firehorse_03_1.effect.json = Data\Effects\Authored\effect.classselect.warlord.fx_pc_msm_02.par_c_firehorse_03_1.effect.json
		Data\Effects\Authored\effect.classselect.warlord.scene_a_lobby.bk.par_j_groundspark_001_cine.effect.json = Data\Effects\Authored\effect.classselect.warlord.scene_a_lobby.bk.par_j_groundspark_001_cine.effect.json
		Data\Effects\Authored\effect.classselect.warlord.scene_a_lobby.bk.par_q_firesh_bk_cine.effect.json = Data\Effects\Authored\effect.classselect.warlord.scene_a_lobby.bk.par_q_firesh_bk_cine.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba0.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba0.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba0.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba0.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba1.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba2.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050010.ba3.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050100.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050100.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050100.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050100.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050100.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050100.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050100.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050100.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip2.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip3.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050120.clip3.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050160.clip4.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050180.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050180.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050180.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050180.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050180.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050180.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a1.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a2.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a2.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a3.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a3.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a4.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050210.a4.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050220.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050220.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050220.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050220.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.single-glass-canary.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.single-glass-canary.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050230.water-burst.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050230.water-burst.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip1.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip1.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip1.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip2.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050240.clip2.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050500.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050500.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050520.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050520.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050520.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050520.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050520.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050520.unified.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050540.full.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050540.full.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050540.tuning.restore.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050540.tuning.restore.effect.json
		Data\Effects\Authored\effect.dimensionmaster.skill.2050540.unified.effect.json = Data\Effects\Authored\effect.dimensionmaster.skill.2050540.unified.effect.json
		Data\Effects\Authored\effect.esther.bahuntur.cameo.npc_sk_breathofarcturus.full.restore.effect.json = Data\Effects\Authored\effect.esther.bahuntur.cameo.npc_sk_breathofarcturus.full.restore.effect.json
		Data\Effects\Authored\effect.esther.inanna.cameo.npc_sk_magicshield.full.restore.effect.json = Data\Effects\Authored\effect.esther.inanna.cameo.npc_sk_magicshield.full.restore.effect.json
		Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_01.full.restore.effect.json = Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_01.full.restore.effect.json
		Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_02.full.restore.effect.json = Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_02.full.restore.effect.json
		Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_03.full.restore.effect.json = Data\Effects\Authored\effect.esther.inanna.npc_att_battle_1_03.full.restore.effect.json
		Data\Effects\Authored\effect.esther.ninave.cameo.npc_sk_parkunas.full.restore.effect.json = Data\Effects\Authored\effect.esther.ninave.cameo.npc_sk_parkunas.full.restore.effect.json
		Data\Effects\Authored\effect.esther.silian.npc_att_battle_7_01.full.restore.effect.json = Data\Effects\Authored\effect.esther.silian.npc_att_battle_7_01.full.restore.effect.json
		Data\Effects\Authored\effect.esther.silian.npc_evt1_sk_swordofchampion_bk.full.restore.effect.json = Data\Effects\Authored\effect.esther.silian.npc_evt1_sk_swordofchampion_bk.full.restore.effect.json
		Data\Effects\Authored\effect.esther.wei.cameo.npc_sk_dochul.full.restore.effect.json = Data\Effects\Authored\effect.esther.wei.cameo.npc_sk_dochul.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49000.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49000.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49000.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49000.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49000.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49000.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49001.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49001.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49001.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49001.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49001.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49001.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49020.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49020.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49021.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49021.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49021.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49021.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49021.clip.4.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49021.clip.4.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49040.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49040.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49041.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49041.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49100.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49100.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49110.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49110.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49110.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49110.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49120.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49120.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49120.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49120.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49130.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49130.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49130.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49130.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49150.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49150.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49150.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49150.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49200.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49200.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49200.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49200.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49200.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49200.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49210.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49210.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49210.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49210.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49210.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49210.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49220.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49220.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49220.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49220.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49220.source.49290.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49220.source.49290.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49220.source.49290.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49220.source.49290.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49230.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49230.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49230.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49230.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49260.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49260.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49260.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49260.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49260.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49260.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49270.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49270.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49270.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49270.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49270.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49270.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49330.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49330.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49330.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49330.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49330.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49330.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49400.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49400.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49400.clip.1.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49400.clip.1.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49400.clip.2.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49400.clip.2.full.restore.effect.json
		Data\Effects\Authored\effect.guardianknight.skill.49420.clip.0.full.restore.effect.json = Data\Effects\Authored\effect.guardianknight.skill.49420.clip.0.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.albion.bluecircle.warning.effect.json = Data\Effects\Authored\effect.kouku.albion.bluecircle.warning.effect.json
		Data\Effects\Authored\effect.kouku.albion.bluecircle.warning.impact.runtime.effect.json = Data\Effects\Authored\effect.kouku.albion.bluecircle.warning.impact.runtime.effect.json
		Data\Effects\Authored\effect.kouku.albion.cross.electric.impact.effect.json = Data\Effects\Authored\effect.kouku.albion.cross.electric.impact.effect.json
		Data\Effects\Authored\effect.kouku.albion.fourfan.impact.effect.json = Data\Effects\Authored\effect.kouku.albion.fourfan.impact.effect.json
		Data\Effects\Authored\effect.kouku.albion.fourfan.warning.effect.json = Data\Effects\Authored\effect.kouku.albion.fourfan.warning.effect.json
		Data\Effects\Authored\effect.kouku.albion.fourfan.warning.single.effect.json = Data\Effects\Authored\effect.kouku.albion.fourfan.warning.single.effect.json
		Data\Effects\Authored\effect.kouku.albion.frontthree.electric.impact.effect.json = Data\Effects\Authored\effect.kouku.albion.frontthree.electric.impact.effect.json
		Data\Effects\Authored\effect.kouku.albion.thunderstorm.impact.effect.json = Data\Effects\Authored\effect.kouku.albion.thunderstorm.impact.effect.json
		Data\Effects\Authored\effect.kouku.bingo.blackhole.center.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.blackhole.center.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.blackhole.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.blackhole.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.bomb.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.bomb.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.encore.blackhole.beam.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.encore.blackhole.beam.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.encore.glass.screen.effect.json = Data\Effects\Authored\effect.kouku.bingo.encore.glass.screen.effect.json
		Data\Effects\Authored\effect.kouku.bingo.encore.glass.spark.effect.json = Data\Effects\Authored\effect.kouku.bingo.encore.glass.spark.effect.json
		Data\Effects\Authored\effect.kouku.bingo.encore.glass.spark0.effect.json = Data\Effects\Authored\effect.kouku.bingo.encore.glass.spark0.effect.json
		Data\Effects\Authored\effect.kouku.bingo.hammer.aura.effect.json = Data\Effects\Authored\effect.kouku.bingo.hammer.aura.effect.json
		Data\Effects\Authored\effect.kouku.bingo.hammer.warning.original.effect.json = Data\Effects\Authored\effect.kouku.bingo.hammer.warning.original.effect.json
		Data\Effects\Authored\effect.kouku.bingo.medusa.attack.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.medusa.attack.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.medusa.face.full.restore.effect.json = Data\Effects\Authored\effect.kouku.bingo.medusa.face.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.bingo.skull.flip.effect.json = Data\Effects\Authored\effect.kouku.bingo.skull.flip.effect.json
		Data\Effects\Authored\effect.kouku.bingo.skull.red.effect.json = Data\Effects\Authored\effect.kouku.bingo.skull.red.effect.json
		Data\Effects\Authored\effect.kouku.bingo.skull.white.effect.json = Data\Effects\Authored\effect.kouku.bingo.skull.white.effect.json
		Data\Effects\Authored\effect.kouku.card.match.bind.floor.effect.json = Data\Effects\Authored\effect.kouku.card.match.bind.floor.effect.json
		Data\Effects\Authored\effect.kouku.card.match.bind.release.effect.json = Data\Effects\Authored\effect.kouku.card.match.bind.release.effect.json
		Data\Effects\Authored\effect.kouku.card.match.clover.effect.json = Data\Effects\Authored\effect.kouku.card.match.clover.effect.json
		Data\Effects\Authored\effect.kouku.card.match.contact.recovery.effect.json = Data\Effects\Authored\effect.kouku.card.match.contact.recovery.effect.json
		Data\Effects\Authored\effect.kouku.card.match.diamond.effect.json = Data\Effects\Authored\effect.kouku.card.match.diamond.effect.json
		Data\Effects\Authored\effect.kouku.card.match.dice.diamond.explosion.effect.json = Data\Effects\Authored\effect.kouku.card.match.dice.diamond.explosion.effect.json
		Data\Effects\Authored\effect.kouku.card.match.dice.full.restore.effect.json = Data\Effects\Authored\effect.kouku.card.match.dice.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.card.match.emit.effect.json = Data\Effects\Authored\effect.kouku.card.match.emit.effect.json
		Data\Effects\Authored\effect.kouku.card.match.emit.full.restore.effect.json = Data\Effects\Authored\effect.kouku.card.match.emit.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.card.match.explosion.effect.json = Data\Effects\Authored\effect.kouku.card.match.explosion.effect.json
		Data\Effects\Authored\effect.kouku.card.match.heart.effect.json = Data\Effects\Authored\effect.kouku.card.match.heart.effect.json
		Data\Effects\Authored\effect.kouku.card.match.prepare.full.restore.effect.json = Data\Effects\Authored\effect.kouku.card.match.prepare.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.card.match.spade.effect.json = Data\Effects\Authored\effect.kouku.card.match.spade.effect.json
		Data\Effects\Authored\effect.kouku.card.spinning.clover.effect.json = Data\Effects\Authored\effect.kouku.card.spinning.clover.effect.json
		Data\Effects\Authored\effect.kouku.card.spinning.diamond.effect.json = Data\Effects\Authored\effect.kouku.card.spinning.diamond.effect.json
		Data\Effects\Authored\effect.kouku.card.spinning.emitter.effect.json = Data\Effects\Authored\effect.kouku.card.spinning.emitter.effect.json
		Data\Effects\Authored\effect.kouku.card.spinning.heart.effect.json = Data\Effects\Authored\effect.kouku.card.spinning.heart.effect.json
		Data\Effects\Authored\effect.kouku.card.spinning.spade.effect.json = Data\Effects\Authored\effect.kouku.card.spinning.spade.effect.json
		Data\Effects\Authored\effect.kouku.cardmaze.lmb.effect.json = Data\Effects\Authored\effect.kouku.cardmaze.lmb.effect.json
		Data\Effects\Authored\effect.kouku.cardmaze.q.effect.json = Data\Effects\Authored\effect.kouku.cardmaze.q.effect.json
		Data\Effects\Authored\effect.kouku.clown.polymorph.e.effect.json = Data\Effects\Authored\effect.kouku.clown.polymorph.e.effect.json
		Data\Effects\Authored\effect.kouku.clown.polymorph.q.effect.json = Data\Effects\Authored\effect.kouku.clown.polymorph.q.effect.json
		Data\Effects\Authored\effect.kouku.clown.polymorph.w.effect.json = Data\Effects\Authored\effect.kouku.clown.polymorph.w.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.ball.drop.split.effect.json = Data\Effects\Authored\effect.kouku.common.circus.ball.drop.split.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.circle.warning.effect.json = Data\Effects\Authored\effect.kouku.common.circus.circle.warning.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.innerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.common.circus.innerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.innerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.common.circus.innerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.outerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.common.circus.outerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.common.circus.outerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.common.circus.outerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.common.counter.ring.effect.json = Data\Effects\Authored\effect.kouku.common.counter.ring.effect.json
		Data\Effects\Authored\effect.kouku.common.flame.wave.decal.effect.json = Data\Effects\Authored\effect.kouku.common.flame.wave.decal.effect.json
		Data\Effects\Authored\effect.kouku.common.flame.wave.full.effect.json = Data\Effects\Authored\effect.kouku.common.flame.wave.full.effect.json
		Data\Effects\Authored\effect.kouku.common.flame.wave.full.koukusaydon.effect.json = Data\Effects\Authored\effect.kouku.common.flame.wave.full.koukusaydon.effect.json
		Data\Effects\Authored\effect.kouku.common.flame.wave.pillar.effect.json = Data\Effects\Authored\effect.kouku.common.flame.wave.pillar.effect.json
		Data\Effects\Authored\effect.kouku.common.flame.wave.warning.effect.json = Data\Effects\Authored\effect.kouku.common.flame.wave.warning.effect.json
		Data\Effects\Authored\effect.kouku.common.spinning.card.casting.effect.json = Data\Effects\Authored\effect.kouku.common.spinning.card.casting.effect.json
		Data\Effects\Authored\effect.kouku.common.spinning.card.flying.effect.json = Data\Effects\Authored\effect.kouku.common.spinning.card.flying.effect.json
		Data\Effects\Authored\effect.kouku.common.spinning.card.overhead.effect.json = Data\Effects\Authored\effect.kouku.common.spinning.card.overhead.effect.json
		Data\Effects\Authored\effect.kouku.common.spinning.card.throw.effect.json = Data\Effects\Authored\effect.kouku.common.spinning.card.throw.effect.json
		Data\Effects\Authored\effect.kouku.common.trumpet.card.floor.effect.json = Data\Effects\Authored\effect.kouku.common.trumpet.card.floor.effect.json
		Data\Effects\Authored\effect.kouku.common.trumpet.radial.lasers.effect.json = Data\Effects\Authored\effect.kouku.common.trumpet.radial.lasers.effect.json
		Data\Effects\Authored\effect.kouku.common.trumpet.suit.floor.effect.json = Data\Effects\Authored\effect.kouku.common.trumpet.suit.floor.effect.json
		Data\Effects\Authored\effect.kouku.dance.aura.root.effect.json = Data\Effects\Authored\effect.kouku.dance.aura.root.effect.json
		Data\Effects\Authored\effect.kouku.dance.full.root.effect.json = Data\Effects\Authored\effect.kouku.dance.full.root.effect.json
		Data\Effects\Authored\effect.kouku.dance.light.root.effect.json = Data\Effects\Authored\effect.kouku.dance.light.root.effect.json
		Data\Effects\Authored\effect.kouku.fear.screen.full.restore.effect.json = Data\Effects\Authored\effect.kouku.fear.screen.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.firecross.impact.effect.json = Data\Effects\Authored\effect.kouku.firecross.impact.effect.json
		Data\Effects\Authored\effect.kouku.firecross.impact.full.effect.json = Data\Effects\Authored\effect.kouku.firecross.impact.full.effect.json
		Data\Effects\Authored\effect.kouku.firecross.impact.light.effect.json = Data\Effects\Authored\effect.kouku.firecross.impact.light.effect.json
		Data\Effects\Authored\effect.kouku.firecross.impact.line.effect.json = Data\Effects\Authored\effect.kouku.firecross.impact.line.effect.json
		Data\Effects\Authored\effect.kouku.firecross.wand.decal.effect.json = Data\Effects\Authored\effect.kouku.firecross.wand.decal.effect.json
		Data\Effects\Authored\effect.kouku.firecross.wand.spin.effect.json = Data\Effects\Authored\effect.kouku.firecross.wand.spin.effect.json
		Data\Effects\Authored\effect.kouku.firecross.warning.effect.json = Data\Effects\Authored\effect.kouku.firecross.warning.effect.json
		Data\Effects\Authored\effect.kouku.firecross.warning.line.effect.json = Data\Effects\Authored\effect.kouku.firecross.warning.line.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.a.impact.effect.json = Data\Effects\Authored\effect.kouku.firegrid.a.impact.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.a.warning.effect.json = Data\Effects\Authored\effect.kouku.firegrid.a.warning.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.b.impact.effect.json = Data\Effects\Authored\effect.kouku.firegrid.b.impact.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.b.warning.effect.json = Data\Effects\Authored\effect.kouku.firegrid.b.warning.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.c.impact.effect.json = Data\Effects\Authored\effect.kouku.firegrid.c.impact.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.c.warning.effect.json = Data\Effects\Authored\effect.kouku.firegrid.c.warning.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.line.impact.effect.json = Data\Effects\Authored\effect.kouku.firegrid.line.impact.effect.json
		Data\Effects\Authored\effect.kouku.firegrid.line.warning.effect.json = Data\Effects\Authored\effect.kouku.firegrid.line.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate1.4219801.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.4219801.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.4219877.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.4219877.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.festival.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.festival.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.p4.scene-image-collapse.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.p4.scene-image-collapse.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.1.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.1.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.2.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.2.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.context.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.context.effect.json
		Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.context.p4-late.effect.json = Data\Effects\Authored\effect.kouku.gate1.authored.portal-arrival.context.p4-late.effect.json
		Data\Effects\Authored\effect.kouku.gate1.backstep.afterimage.effect.json = Data\Effects\Authored\effect.kouku.gate1.backstep.afterimage.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.circle.impact.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.circle.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.circle.warning.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.circle.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.impact.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.innerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.impact.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.gate1.blade-dance.outerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.gate1.cardrain.drop.effect.json = Data\Effects\Authored\effect.kouku.gate1.cardrain.drop.effect.json
		Data\Effects\Authored\effect.kouku.gate1.charge_counter.afterimage.effect.json = Data\Effects\Authored\effect.kouku.gate1.charge_counter.afterimage.effect.json
		Data\Effects\Authored\effect.kouku.gate1.charge_counter.fx.effect.json = Data\Effects\Authored\effect.kouku.gate1.charge_counter.fx.effect.json
		Data\Effects\Authored\effect.kouku.gate1.charge_counter.group.effect.json = Data\Effects\Authored\effect.kouku.gate1.charge_counter.group.effect.json
		Data\Effects\Authored\effect.kouku.gate1.circus.ball.launch.effect.json = Data\Effects\Authored\effect.kouku.gate1.circus.ball.launch.effect.json
		Data\Effects\Authored\effect.kouku.gate1.circus.ball.upper.effect.json = Data\Effects\Authored\effect.kouku.gate1.circus.ball.upper.effect.json
		Data\Effects\Authored\effect.kouku.gate1.circus.gun.muzzle.effect.json = Data\Effects\Authored\effect.kouku.gate1.circus.gun.muzzle.effect.json
		Data\Effects\Authored\effect.kouku.gate1.circus.rainbow.drop.effect.json = Data\Effects\Authored\effect.kouku.gate1.circus.rainbow.drop.effect.json
		Data\Effects\Authored\effect.kouku.gate1.circus.rainbow.impact.effect.json = Data\Effects\Authored\effect.kouku.gate1.circus.rainbow.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.festival.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.festival.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.fireworks.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.fireworks.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.gold-trails.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.gold-trails.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.portal-suction.centered.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.portal-suction.centered.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.portal-suction.scene-collapse.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.portal-suction.scene-collapse.effect.json
		Data\Effects\Authored\effect.kouku.gate1.intro.scene-image-collapse.effect.json = Data\Effects\Authored\effect.kouku.gate1.intro.scene-image-collapse.effect.json
		Data\Effects\Authored\effect.kouku.gate1.popup.movable-lights.effect.json = Data\Effects\Authored\effect.kouku.gate1.popup.movable-lights.effect.json
		Data\Effects\Authored\effect.kouku.gate1.popup.source-material-carriers.effect.json = Data\Effects\Authored\effect.kouku.gate1.popup.source-material-carriers.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.boundary.group.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.boundary.group.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.explosion.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.explosion.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.laser.group.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.laser.group.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.shield.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.shield.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.star.boundary.group.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.star.boundary.group.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.star.draw.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.star.draw.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate1.stagger.star.group.effect.json = Data\Effects\Authored\effect.kouku.gate1.stagger.star.group.effect.json
		Data\Effects\Authored\effect.kouku.gate2.4219776.stage1.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.4219776.stage1.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate2.4219776.stage2.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.4219776.stage2.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate2.bigsaydon.circle.warning.effect.json = Data\Effects\Authored\effect.kouku.gate2.bigsaydon.circle.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate2.bigsaydon.wind.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.bigsaydon.wind.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate2.bigsaydon.yellow-eye-warning.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.bigsaydon.yellow-eye-warning.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate2.bigsaydon.yellow-gaze.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.bigsaydon.yellow-gaze.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate2.entrance.drop.ball.effect.json = Data\Effects\Authored\effect.kouku.gate2.entrance.drop.ball.effect.json
		Data\Effects\Authored\effect.kouku.gate2.entrance.rope.trail.effect.json = Data\Effects\Authored\effect.kouku.gate2.entrance.rope.trail.effect.json
		Data\Effects\Authored\effect.kouku.gate2.fanfare.circle.six-rays.effect.json = Data\Effects\Authored\effect.kouku.gate2.fanfare.circle.six-rays.effect.json
		Data\Effects\Authored\effect.kouku.gate2.fanfare.front.music.effect.json = Data\Effects\Authored\effect.kouku.gate2.fanfare.front.music.effect.json
		Data\Effects\Authored\effect.kouku.gate2.fanfare.rectangle.warning.effect.json = Data\Effects\Authored\effect.kouku.gate2.fanfare.rectangle.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate2.intro.lights.effect.json = Data\Effects\Authored\effect.kouku.gate2.intro.lights.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.inhand.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.inhand.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.0.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.0.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.1.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.1.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.2.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.target.flight.2.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.target.impact.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.target.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate2.juggling.three.balls.effect.json = Data\Effects\Authored\effect.kouku.gate2.juggling.three.balls.effect.json
		Data\Effects\Authored\effect.kouku.gate2.safezone.blue.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate2.safezone.blue.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.electric.threeway.authored.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.electric.threeway.authored.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.flame.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.flame.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.full.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.full.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.ground.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.ground.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.ring.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.ring.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.ring.end.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.ring.end.effect.json
		Data\Effects\Authored\effect.kouku.gate3.backstep.ring.flame.effect.json = Data\Effects\Authored\effect.kouku.gate3.backstep.ring.flame.effect.json
		Data\Effects\Authored\effect.kouku.gate3.clone.breath.effect.json = Data\Effects\Authored\effect.kouku.gate3.clone.breath.effect.json
		Data\Effects\Authored\effect.kouku.gate3.doll.flame.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.doll.flame.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.doll.flame.object-sustain15.effect.json = Data\Effects\Authored\effect.kouku.gate3.doll.flame.object-sustain15.effect.json
		Data\Effects\Authored\effect.kouku.gate3.doll.flame.shared.effect.json = Data\Effects\Authored\effect.kouku.gate3.doll.flame.shared.effect.json
		Data\Effects\Authored\effect.kouku.gate3.firebreath.shared.effect.json = Data\Effects\Authored\effect.kouku.gate3.firebreath.shared.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.boss.pentagram.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.boss.pentagram.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.center.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.center.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.center.pentagram.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.center.pentagram.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.center.portal.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.center.portal.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.circus.ball.aura.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.circus.ball.aura.effect.json
		Data\Effects\Authored\effect.kouku.gate3.mario.entry.portal.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.mario.entry.portal.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.light.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.light.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.warning.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.warning.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.wave.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.rainbow.fire.wave.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.rainbow.grid.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.rainbow.grid.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.ritual.hand.trail.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.ritual.hand.trail.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.ritual.staff.tip.trail.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.ritual.staff.tip.trail.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.airstrike.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.airstrike.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.airstrike.missile.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.airstrike.missile.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.ball.drop.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.ball.drop.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.ball.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.ball.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.ball.red.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.ball.red.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.drop.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.drop.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.explosion.blue.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.explosion.blue.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.fuse.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.bomb.fuse.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.bullet.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.bullet.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact01.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact01.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact03.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.circle.impact03.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.circle.warning.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.circle.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.circle.warning.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.circle.warning.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.donut.warning.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.donut.warning.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.fire.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.fire.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.create.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.create.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.end.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.end.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.ground.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.ground.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.loop.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.loop.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.muzzle.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.muzzle.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.muzzle.world.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.muzzle.world.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.shell.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.shell.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.signature.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.signature.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.gun.signature.world.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.gun.signature.world.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.innerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.napalm.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.napalm.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.warning.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.warning.expand.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.outerdonut.warning.expand.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.rectangle.impact.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.rectangle.impact.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.rectangle.warning.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.rectangle.warning.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.saydon.disappear.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.saydon.disappear.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.sector.warning.shot.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.sector.warning.shot.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.end.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.end.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.end.ground.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.end.ground.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.fixed.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.fixed.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.fixed.ground.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.fixed.ground.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.tracking.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.tracking.effect.json
		Data\Effects\Authored\effect.kouku.gate3.showtime.target.tracking.ground.effect.json = Data\Effects\Authored\effect.kouku.gate3.showtime.target.tracking.ground.effect.json
		Data\Effects\Authored\effect.kouku.gate3.staff.flame.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.staff.flame.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.gate3.threeway.breath.full.restore.effect.json = Data\Effects\Authored\effect.kouku.gate3.threeway.breath.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.magic.paper.dove.group.effect.json = Data\Effects\Authored\effect.kouku.magic.paper.dove.group.effect.json
		Data\Effects\Authored\effect.kouku.mario.ball.pop.blue.effect.json = Data\Effects\Authored\effect.kouku.mario.ball.pop.blue.effect.json
		Data\Effects\Authored\effect.kouku.mario.ball.pop.red.effect.json = Data\Effects\Authored\effect.kouku.mario.ball.pop.red.effect.json
		Data\Effects\Authored\effect.kouku.mario.ball.pop.yellow.effect.json = Data\Effects\Authored\effect.kouku.mario.ball.pop.yellow.effect.json
		Data\Effects\Authored\effect.kouku.mario.blade.instant-death.full.restore.effect.json = Data\Effects\Authored\effect.kouku.mario.blade.instant-death.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.mario.blade.normal.full.restore.effect.json = Data\Effects\Authored\effect.kouku.mario.blade.normal.full.restore.effect.json
		Data\Effects\Authored\effect.kouku.mario.clown.hammer.end.effect.json = Data\Effects\Authored\effect.kouku.mario.clown.hammer.end.effect.json
		Data\Effects\Authored\effect.kouku.mario.flyingball.hit.effect.json = Data\Effects\Authored\effect.kouku.mario.flyingball.hit.effect.json
		Data\Effects\Authored\effect.kouku.mario.marker.blue.effect.json = Data\Effects\Authored\effect.kouku.mario.marker.blue.effect.json
		Data\Effects\Authored\effect.kouku.mario.marker.red.effect.json = Data\Effects\Authored\effect.kouku.mario.marker.red.effect.json
		Data\Effects\Authored\effect.kouku.mario.marker.yellow.effect.json = Data\Effects\Authored\effect.kouku.mario.marker.yellow.effect.json
		Data\Effects\Authored\effect.kouku.mario.player.hammer.impact.effect.json = Data\Effects\Authored\effect.kouku.mario.player.hammer.impact.effect.json
		Data\Effects\Authored\effect.kouku.pizza.explosion.group.effect.json = Data\Effects\Authored\effect.kouku.pizza.explosion.group.effect.json
		Data\Effects\Authored\effect.kouku.pizza.explosion.group.source-restored.effect.json = Data\Effects\Authored\effect.kouku.pizza.explosion.group.source-restored.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene01b.efseqact_matinee_0.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene01b.efseqact_matinee_0.1.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.1.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.2.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.2.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.3.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.3.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.4.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.4.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.5.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.5.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10_arrival.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10_arrival.1.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10_arrival.2.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10_arrival.2.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.1.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.2.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.2.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene04a.efseqact_matinee_1.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene04a.efseqact_matinee_1.1.effect.json
		Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene04a.efseqact_matinee_2.1.effect.json = Data\Effects\Authored\effect.kouku.sequence.lv_lut_midnightc_ed_scene04a.efseqact_matinee_2.1.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_00.dust.par_g_boxdust_001.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_00.dust.par_g_boxdust_001.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_01.etc.par_k_drain_big_01_l.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_01.etc.par_k_drain_big_01_l.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_01.explosion.par_q_fireworks_01_loop.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_01.explosion.par_q_fireworks_01_loop.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_02.glow.par_m_glow_001.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_02.glow.par_m_glow_001.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_03.leaf.par_m_paperleaf_01.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_03.leaf.par_m_paperleaf_01.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_11.aura.par_g_ghostship_01.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_11.aura.par_g_ghostship_01.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_15.smoke.par_q_magicshow_01.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_15.smoke.par_q_magicshow_01.effect.json
		Data\Effects\Authored\effect.kouku.source.bfx_low_21.etc.par_q_cutystar_01.effect.json = Data\Effects\Authored\effect.kouku.source.bfx_low_21.etc.par_q_cutystar_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_00.hit.par_d_groggyhit_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_00.hit.par_d_groggyhit_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_00.hit.par_k_raidgroggy_hit_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_00.hit.par_k_raidgroggy_hit_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefaint_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefaint_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_002.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_002.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_003.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_b_statefear_003.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_c_silence_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_c_silence_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_e_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_e_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_l_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_l_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_s_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_damageup_01_s_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_e.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_e.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_l.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_l.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_s.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_01_s.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_02_e.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_defence_02_e.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_powerdown_01_l.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_d_buff_powerdown_01_l.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_e_debuffdark_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_e_debuffdark_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_e_dotfire_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_e_dotfire_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_j_exmove_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_j_exmove_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_l_change_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_04.state.par_l_change_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_confusion_red_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_confusion_red_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_confusion_red_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_confusion_red_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_timer_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_d_timer_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_buff_target_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_buff_target_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_rpct_05_auramadness.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_rpct_05_auramadness.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_rpct_05_auramadness_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_rpct_05_auramadness_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_voldaik_pcformnpc_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_voldaik_pcformnpc_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_voldaik_pcformnpc_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_g_voldaik_pcformnpc_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_l_rpct_camblood_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_08.buff.par_l_rpct_camblood_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_bs_10.hit.par_d_beamhit_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_bs_10.hit.par_d_beamhit_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_c_dust_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_c_dust_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_001_1.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_001_1.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_001_pr.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_001_pr.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_002_pr.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_002_pr.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_007_pr.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_d_dust_007_pr.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_e_underdust_03_2.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_e_underdust_03_2.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_e_underdustloop_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_00.dust.par_e_underdustloop_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_01.debris.par_d_debris_003.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_01.debris.par_d_debris_003.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_01.decal.par_d_decal_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_01.decal.par_d_decal_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_01.distortion.par_mp_concavedis_z_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_01.distortion.par_mp_concavedis_z_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_01.distortion.par_mp_concavedis_z_08.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_01.distortion.par_mp_concavedis_z_08.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.ice.par_e_shot_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.ice.par_e_shot_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.ice.par_g_icebomb_01_pr.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.ice.par_g_icebomb_01_pr.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_m_light_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_m_light_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01_loop.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01_loop.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01_sine.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_01_sine.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_05_l.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_mp_light_05_l.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_02_off.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_02_off.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_02_on.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_02.light.par_x_light_02_on.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_03.spawn.par_d_spawn_010.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_03.spawn.par_d_spawn_010.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_03.spreadenergy.par_c_ring_001.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_03.spreadenergy.par_c_ring_001.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_03.swing.par_mp_swing_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_03.swing.par_mp_swing_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_04.sys.par_t_singlemode_guide_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_04.sys.par_t_singlemode_guide_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_cm_04.sys.par_t_singlemode_guide_01_s.effect.json = Data\Effects\Authored\effect.kouku.source.fx_cm_04.sys.par_t_singlemode_guide_01_s.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_boeh_00.par_o_boeh_atk_08_dust_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_boeh_00.par_o_boeh_atk_08_dust_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_prj01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_prj01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_prj01_exp2.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_prj01_exp2.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_sk12_13_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_boss_01.rpct.par_d_rpct_sk12_13_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_cdebz_00.par_v_cdebz_sight_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_cdebz_00.par_v_cdebz_sight_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_cmdgr_03-1.par_y_cmdgr_03-1_spawn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_cmdgr_03-1.par_y_cmdgr_03-1_spawn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_cuas_01.par_o_cuas_atk_06_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_cuas_01.par_o_cuas_atk_06_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_isak_00.par_k_isak_buff_mosedamage_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_isak_00.par_k_isak_buff_mosedamage_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_lrcr_02.par_l_lrcr_02_sk_02_3.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_lrcr_02.par_l_lrcr_02_sk_02_3.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppab_00.par_s_ppab_electrocution_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppab_00.par_s_ppab_electrocution_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppcc_00.par_k_ppcc_ballsblow_exp_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppcc_00.par_k_ppcc_ballsblow_exp_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppcc_00.par_l_ppcc_sk_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppcc_00.par_l_ppcc_sk_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_handswing_trail_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_handswing_trail_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_throwhoop_prj_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_throwhoop_prj_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_ball_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_ball_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_exp_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_ptck_00.par_o_ptck_atk_04_dust_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_ptck_00.par_o_ptck_atk_04_dust_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rhcn_01.par_x_rhcn_saprkloop_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rhcn_01.par_x_rhcn_saprkloop_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_03.par_d_rpct03_sk14_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_03.par_d_rpct03_sk14_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_03.par_d_rpct03_sk14_31.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_03.par_d_rpct03_sk14_31.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_aura_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_aura_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_aura_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_aura_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_deco_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_deco_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_prj_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_card_prj_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_dead_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_dead_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_aura_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_aura_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_aura_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_aura_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_fingerpoint_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_fingerpoint_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_hand_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_hand_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_light_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_dance_light_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_drug_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_drug_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_decal_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_fire_decal_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firewave_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firewave_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_hat.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_hat.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_hit_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_hit_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_mouse_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_mouse_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_mouse_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_mouse_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_randombox_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_smoke_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_smoke_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_smoke_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_smoke_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_decal_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_decal_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_spin_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_wand_spin_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_dust_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_dust_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_eye_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_eye_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_eye_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_eye_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_shield_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_shield_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_1.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_1.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_4.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_4.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_6.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_02_6.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_10_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_10_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_11_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_11_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_12.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_12.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_13.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_13.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_6_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_6_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_7_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_7_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_8_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_8_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_9_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_9_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_04_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_4.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_4.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_5.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_5.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_7.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_7.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_8.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_8.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_9_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_06_9_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_1.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_1.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_10_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_10_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_11_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_11_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_12_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_12_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_3.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_3.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_6_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_6_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_7_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_7_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_8_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_8_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_9_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_9_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_07_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_10_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_10_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_11_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_11_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_6_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_6_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_7_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_7_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_8_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_8_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_9_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_9_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_08_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_10_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_10_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_11_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_11_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_12_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_12_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_13_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_13_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_14_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_14_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_15_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_15_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_16_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_16_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_6_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_6_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_7_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_7_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_8_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_8_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_9.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_9.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_09_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_5_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_5_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_6_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_6_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_7_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_7_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_8_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_8_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_2_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_2_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_3_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_3_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_4_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_4_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_timebomb_01_2.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_timebomb_01_2.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_04_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_trail_04_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_o_rpct_heart_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_o_rpct_heart_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_o_rpct_star_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_l.par_o_rpct_star_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_mark_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_mark_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_markoff_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_markoff_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_spotlight_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_spotlight_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_spotlight_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_spotlight_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_01_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_01_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_02_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_02_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_02_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_05_x.par_x_rpct_timebomb_02_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_aura_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_aura_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_02_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_02_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_03_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_03_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_04_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_04_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_05_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_05_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_06_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_06_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_02_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_02_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_03_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_03_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_04_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_04_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_05_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_crash_05_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_02_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_02_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_03_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_03_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_04_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_04_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_05_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_05_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_05_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_dust_05_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_04_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_eye_02_04_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firebreath_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firebreath_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firecast_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firecast_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firecast_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firecast_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firedc_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firedc_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firedc_01_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_firedc_01_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hatcast_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hatcast_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hatend_01_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hatend_01_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hit_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hit_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_02_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_02_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_02_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_hnadcast_02_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_laseratk_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_laseratk_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_lightred_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_lightred_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_projbird_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_projbird_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_projexp_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_projexp_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_screenpp_01_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_screenpp_01_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_spawn_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_spawn_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_02_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_02_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_03_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_trail_03_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windbreath_01_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windcast_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windcast_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windember_01_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_windember_01_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_wp_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_wp_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_wp_01_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_wp_01_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_boim_buff01_02l.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_boim_buff01_02l.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_boim_buff01_03e_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_boim_buff01_03e_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff11.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff11.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff12.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff12.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff13.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff13.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff14.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff14.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff21.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff21.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff22.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff22.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff23.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff23.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff24.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_buff24.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_prj01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_d.par_d_rpct07_prj01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_dark_decal_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_dark_decal_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_dark_decal_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_dark_decal_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_blue_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_blue_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_green_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_green_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_red_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_red_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_yellow_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_ppct_spot_dance_yellow_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct07_ball_up_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct07_ball_up_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct07_ray_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct07_ray_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka_end.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka_end.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka_loop.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_07_bazooka_loop.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_airstrike_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_airstrike_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_airstrike_missile_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_airstrike_missile_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_all_shot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_all_shot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_09_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_09_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_09_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_09_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_face_spwn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_atk_face_spwn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_back_atk_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_back_atk_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_back_shot_dust_atk_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_back_shot_dust_atk_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ball_appear_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ball_appear_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ball_big_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ball_big_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_ball_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_exp_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_ballread_exp_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_bazooka_mesh_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_bazooka_mesh_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_boom_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_boom_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_breath_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_breath_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cannon_prj_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cannon_prj_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_coming_shot_cast_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_coming_shot_cast_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_crack_pjt_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_crack_pjt_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dance_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dance_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dance_shot_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dance_shot_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dash_atk_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dash_atk_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dashatk_dash_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dashatk_dash_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dountfracture_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dountfracture_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_elec_prj_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_elec_prj_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_elecarea_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_elecarea_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_exp_round_deacl_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_exp_round_deacl_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_atk_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_atk_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_atk_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_atk_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_shield_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eye_shield_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eyecast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_eyecast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_fire_bigearthwave_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_fire_bigearthwave_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_fire_decal_t_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_fire_decal_t_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_firebreath_breath_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_firebreath_breath_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_cast_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_cast_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_round_atk_exp_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gas_round_atk_exp_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gun_shot_decal_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gun_shot_decal_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_hand_shot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_hand_shot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_handswing_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_handswing_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_handswing_trail_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_handswing_trail_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_holo_spot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_holo_spot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_jump_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_jump_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_light_cast_01_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_light_cast_01_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_line_atk_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_line_atk_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magic_end_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magic_end_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magiccircle_prj_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magiccircle_prj_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magiccircle_prj_exp_03_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_magiccircle_prj_exp_03_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_mbition_body_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_mbition_body_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_napalm_area_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_napalm_area_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potal_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potal_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potalshot_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potalshot_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potalshot_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_potalshot_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_end_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_realtarget_end_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_rotate_pjt_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_rotate_pjt_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_round_fire_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_round_fire_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_shot_02_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_shot_02_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_signalshot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_signalshot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_signature_01_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_signature_01_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_skull_sign_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_skull_sign_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_skullshot_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_skullshot_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_smacing_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_smacing_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_shot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_shot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_sign_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_star_sign_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_thunderstorm_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_thunderstorm_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rptm_down_lighting_atk_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_rptm_down_lighting_atk_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_target_shot_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpct_07_v.par_v_target_shot_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_02_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_02_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00-1.par_l_rpcz_00-1_sk_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_fire_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_fire_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_prj01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_c_prj01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_bomb_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_bomb_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_bomb_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_bomb_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_d_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_fire_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_bazooka_fire_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_00_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_00_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_a_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_b_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_b_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_c_00_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_c_00_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_c_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_c_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_d_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_hammer_d_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_jump_dust.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_jump_dust.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_jump_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_jump_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_move_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_move_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_prj_exp_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_prj_exp_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_smoke_despawn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_smoke_despawn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_a_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_a_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_a_music_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_a_music_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_b_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_b_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_b_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_b_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_c_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_c_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_music_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_music_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_weapon_spawn_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_weapon_spawn_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_bazooka_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_bazooka_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_bazooka_despawn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_bazooka_despawn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_hammer_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_hammer_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_hammer_despawn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_hammer_despawn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_trumpet_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_trumpet_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_trumpet_despawn_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_wp_trumpet_despawn_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_cast_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_cast_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_ribbon_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_ribbon_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_rope_blue_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_rope_blue_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_cast_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_safezone_cast_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_throwhoop_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_throwhoop_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_throwhoop_aura_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_throwhoop_aura_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_01_despawn_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_01_despawn_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_hell_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_a_hell_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_b_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_b_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_b_01_despawn.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_trowhoop_b_01_despawn.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_area_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_area_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ball_appear_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ball_appear_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ball_appear_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ball_appear_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ballshoot_ball_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ballshoot_ball_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ballshoot_ball_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ballshoot_ball_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bazooka_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bazooka_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_cast_dust_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_cast_dust_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_dust_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_beam_dust_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_down_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_down_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigexp_clockwise_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigexp_clockwise_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigexp_counterclockwise_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigexp_counterclockwise_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cast_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circus_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circus_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circus_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circus_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusbomb_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusbomb_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusbomb_trail_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusbomb_trail_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusprj_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusprj_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusprj_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_circusprj_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cohesion_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cohesion_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cohesion_exp_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_cohesion_exp_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_dash_ground_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_dash_ground_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_dash_trail_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_dash_trail_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_appear_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_appear_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_cast_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_cast_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_prjexp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_doll_prjexp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_03_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_03_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_04_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_04_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_04_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energyballoon_04_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energycast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_energycast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_eyelight_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_eyelight_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_gagebuff_cast_02_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_gagebuff_cast_02_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_gagebuff_cast_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_gagebuff_cast_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_hand_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_hand_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_01_1_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_01_1_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_exp_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_exp_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_hit_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_jugle_hit_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_koukusaton_critical_curse_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_koukusaton_critical_curse_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ribbon_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_ribbon_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_rope_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_rope_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_rope_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_rope_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_fog_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_fog_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_01_e_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_01_e_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_02_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_shield_buff_02_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_spotlight_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_spotlight_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_sword_cast_01_loc_int.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_sword_cast_01_loc_int.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_tslc_01_l.par_l_tslc_shield_e_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_tslc_01_l.par_l_tslc_shield_e_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_mn_tslc_01_l.par_l_tslc_shield_s_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_mn_tslc_01_l.par_l_tslc_shield_s_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_npc_j_00.par_j_rppb_broken_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_npc_j_00.par_j_rppb_broken_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_pc_fbmm_03.par_w_lastde06.effect.json = Data\Effects\Authored\effect.kouku.source.fx_pc_fbmm_03.par_w_lastde06.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_filmnoise_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_filmnoise_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_filmnoise_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_filmnoise_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_c_zoomblur_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_post.fx_par.par_x_zoomblur_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par.par_q_attackgrass.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par.par_q_attackgrass.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_01.par_q_darkfield_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_01.par_q_darkfield_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_01.par_q_field_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_01.par_q_field_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_cardfly_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_cardfly_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_cardfly_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_cardfly_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_03.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_03.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_04.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_chip_04.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_colorpaper_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_colorpaper_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_movingtrail_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_movingtrail_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_paperdust_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_paperdust_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_exp_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_exp_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_exp_02.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_exp_02.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_ready_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpct_ready_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpctgate_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_rpctgate_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_trail_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_trail_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_warpspace_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_q_w_01.fx_par_02.par_q_warpspace_01.effect.json
		Data\Effects\Authored\effect.kouku.source.fx_u_w_01.notuse.par_u_rpcz_safezone_cast_01.effect.json = Data\Effects\Authored\effect.kouku.source.fx_u_w_01.notuse.par_u_rpcz_safezone_cast_01.effect.json
		Data\Effects\Authored\effect.kouku.source.scene_a.fx.par_g_darkaura_02_cine.effect.json = Data\Effects\Authored\effect.kouku.source.scene_a.fx.par_g_darkaura_02_cine.effect.json
		Data\Effects\Authored\effect.kouku.source.scene_a.fx.par_mp_deathmon_01_cine.effect.json = Data\Effects\Authored\effect.kouku.source.scene_a.fx.par_mp_deathmon_01_cine.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba4.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba4.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34010.ba4.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34010.ba4.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34020.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34020.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34030.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34030.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34040.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34040.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34040.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34040.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34040.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34040.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34040.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34040.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34090.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34090.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34090.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34090.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34100.clip3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34100.clip3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34110.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34110.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34110.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34110.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34120.clip3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34120.clip3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba1.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba2.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba2.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34140.ba2.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34140.ba2.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34150.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34150.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34150.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34150.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34160.ba1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34160.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34160.ba1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34160.ba1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34160.ba2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34160.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34160.ba2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34160.ba2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34510.ba3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34510.ba3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34520.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34520.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34540.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34540.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34540.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34540.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34550.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34550.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34550.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34550.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34560.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34560.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34560.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34560.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34560.clip3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34560.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34560.clip3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34560.clip3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34570.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34570.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34570.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34570.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34570.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34570.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34570.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34570.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34580.ba1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34580.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34580.ba1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34580.ba1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34580.ba2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34580.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34580.ba2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34580.ba2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34590.ba3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34590.ba3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34610.clip3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34610.clip3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip2.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip3.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip3.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip3.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip4.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip4.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.clip4.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.clip4.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34630.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34630.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34650.clip1.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34650.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34650.clip1.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34650.clip1.unified.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34650.clip2.full.restore.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34650.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.lancemaster.skill.34650.clip2.unified.effect.json = Data\Effects\Authored\effect.lancemaster.skill.34650.clip2.unified.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.e.speed.effect.json = Data\Effects\Authored\effect.maharaka.watergun.e.speed.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.q.flight.effect.json = Data\Effects\Authored\effect.maharaka.watergun.q.flight.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.q.hit.effect.json = Data\Effects\Authored\effect.maharaka.watergun.q.hit.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.q.start.effect.json = Data\Effects\Authored\effect.maharaka.watergun.q.start.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.r.flight.effect.json = Data\Effects\Authored\effect.maharaka.watergun.r.flight.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.r.hit.effect.json = Data\Effects\Authored\effect.maharaka.watergun.r.hit.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.shot.start.effect.json = Data\Effects\Authored\effect.maharaka.watergun.shot.start.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.w.flight.effect.json = Data\Effects\Authored\effect.maharaka.watergun.w.flight.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.w.hit.effect.json = Data\Effects\Authored\effect.maharaka.watergun.w.hit.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.watergun_att_1.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.watergun.watergun_att_1.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.watergun_att_2.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.watergun.watergun_att_2.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.watergun_att_3.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.watergun.watergun_att_3.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.watergun.watergun_att_4.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.watergun.watergun_att_4.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_01.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_01.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part0.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part0.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part1.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part1.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part2.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part2.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part3.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_02.part3.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_04.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.att_battle_3_04.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.jet.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.jet.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.cannon.telegraph.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.cannon.telegraph.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.ground.att_battle_1_01.full.restore.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.ground.att_battle_1_01.full.restore.effect.json
		Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.telegraph.effect.json = Data\Effects\Authored\effect.maharaka.waterpang.mokomoko.telegraph.effect.json
		Data\Effects\Authored\effect.valtan.action.15.stage002.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.15.stage002.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.400424.stage000.dash.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.400424.stage000.dash.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.400440.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.400440.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420602.stage001.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420602.stage001.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420602.stage002.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420602.stage002.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420602.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420602.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420603.stage002.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420603.stage002.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420603.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420603.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420604.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420604.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420609.stage008.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420609.stage008.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420609.stage009.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420609.stage009.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420609.stage010.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420609.stage010.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420610.stage019.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420610.stage019.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420610.stage021.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420610.stage021.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420612.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420612.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420612.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420612.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420618.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420618.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420619.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420619.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420620.stage001.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420620.stage001.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420620.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420620.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420620.stage005.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420620.stage005.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420620.stage006.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420620.stage006.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420622.stage001.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420622.stage001.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420623.stage001.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420623.stage001.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420623.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420623.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420623.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420623.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage001.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage001.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage005.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage005.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage006.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage006.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage007.body.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage007.body.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage007.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage007.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage007.sectors.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage007.sectors.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage009.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage009.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage010.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage010.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage011.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage011.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420624.stage012.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420624.stage012.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420627.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420627.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420628.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420628.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420629.stage006.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420629.stage006.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420629.stage008.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420629.stage008.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420630.stage002.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420630.stage002.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420630.stage003.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420630.stage003.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420630.stage004.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420630.stage004.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420630.stage005.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420630.stage005.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420630.stage007.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420630.stage007.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420633.stage014.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420633.stage014.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420633.stage015.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420633.stage015.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.action.420642.stage000.full.restore.effect.json = Data\Effects\Authored\effect.valtan.action.420642.stage000.full.restore.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.backstep.windup.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.backstep.windup.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.counter.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.counter.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.roar.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.charge-grab-roar.roar.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.down-smash.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.down-smash.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.earthquake-smash.delayed.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.earthquake-smash.delayed.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.earthquake-smash.impact.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.earthquake-smash.impact.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.four-slash.active.clip-02.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.four-slash.active.clip-02.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.front-back-front.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.front-back-front.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.ground-wave-smash.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.ground-wave-smash.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.ground-wave-smash.active.clip-02.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.ground-wave-smash.active.clip-02.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.high-jump.land.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.high-jump.land.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.windup.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.imprison-roar.windup.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.jump-spin.jump.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.jump-spin.jump.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.windup.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.ledge-roar.windup.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.rushes.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.portal-rush.rushes.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.red-blade-wave.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.red-blade-wave.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.super-smash.windup.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.super-smash.windup.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.swing.active.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.swing.active.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.swing.active.clip-02.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.swing.active.clip-02.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.ghost-transition-15.tracking.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.mechanic.ghost-transition-15.tracking.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.reactive.parry.slash.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.reactive.parry.slash.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.carrier-v1.reactive.parry.stance.clip-01.effect.json = Data\Effects\Authored\effect.valtan.carrier-v1.reactive.parry.stance.clip-01.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.entrance.eyes.parameters.effect.json = Data\Effects\Authored\effect.valtan.cinematic.entrance.eyes.parameters.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.finale.actor35.parameters.effect.json = Data\Effects\Authored\effect.valtan.cinematic.finale.actor35.parameters.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.phase2.actor75.at1549.effect.json = Data\Effects\Authored\effect.valtan.cinematic.phase2.actor75.at1549.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.phase2.actor76.at1539.effect.json = Data\Effects\Authored\effect.valtan.cinematic.phase2.actor76.at1539.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.roar.actor98.at4889.effect.json = Data\Effects\Authored\effect.valtan.cinematic.roar.actor98.at4889.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.roar.actor99.at3843.effect.json = Data\Effects\Authored\effect.valtan.cinematic.roar.actor99.at3843.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.trash.actor100.at667.effect.json = Data\Effects\Authored\effect.valtan.cinematic.trash.actor100.at667.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.trash.actor104.at667.effect.json = Data\Effects\Authored\effect.valtan.cinematic.trash.actor104.at667.effect.json
		Data\Effects\Authored\effect.valtan.cinematic.trash.actor106.at667.effect.json = Data\Effects\Authored\effect.valtan.cinematic.trash.actor106.at667.effect.json
		Data\Effects\Authored\effect.valtan.dash-charge.source.shield.effect.json = Data\Effects\Authored\effect.valtan.dash-charge.source.shield.effect.json
		Data\Effects\Authored\effect.valtan.ether.orb.effect.json = Data\Effects\Authored\effect.valtan.ether.orb.effect.json
		Data\Effects\Authored\effect.valtan.floor-wipe-130.effect.json = Data\Effects\Authored\effect.valtan.floor-wipe-130.effect.json
		Data\Effects\Authored\effect.valtan.ground-roar.rock.active.effect.json = Data\Effects\Authored\effect.valtan.ground-roar.rock.active.effect.json
		Data\Effects\Authored\effect.valtan.ground-roar.rock.explode.effect.json = Data\Effects\Authored\effect.valtan.ground-roar.rock.explode.effect.json
		Data\Effects\Authored\effect.valtan.pattern.420633.active.effect.json = Data\Effects\Authored\effect.valtan.pattern.420633.active.effect.json
		Data\Effects\Authored\effect.valtan.pattern.420633.active.v1.unified.effect.json = Data\Effects\Authored\effect.valtan.pattern.420633.active.v1.unified.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.dash-charge.active-shield.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.dash-charge.active-shield.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.dash-charge.windup-telegraph.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.dash-charge.windup-telegraph.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.large-donut.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.large-donut.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.attack-whirlwind.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.attack-whirlwind.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.catch-breath.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.catch-breath.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.counter.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.counter.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.four.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.four.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.six-pizza-106.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.six-pizza-106.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.three.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.three.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-fail.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-fail.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-if.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-if.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-success.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash-catch-success.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.trash.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.sequence.warp.portal.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.sequence.warp.portal.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.terrain-destruction-3.semicircle.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.terrain-destruction-3.semicircle.effect.json
		Data\Effects\Authored\effect.valtan.project-tuned.terrain-destruction-9.semicircle.effect.json = Data\Effects\Authored\effect.valtan.project-tuned.terrain-destruction-9.semicircle.effect.json
		Data\Effects\Authored\effect.valtan.red-blade-wave.active.effect.json = Data\Effects\Authored\effect.valtan.red-blade-wave.active.effect.json
		Data\Effects\Authored\effect.valtan.sequence.charge.effect.json = Data\Effects\Authored\effect.valtan.sequence.charge.effect.json
		Data\Effects\Authored\effect.valtan.sequence.charge2.effect.json = Data\Effects\Authored\effect.valtan.sequence.charge2.effect.json
		Data\Effects\Authored\effect.valtan.sequence.cross.effect.json = Data\Effects\Authored\effect.valtan.sequence.cross.effect.json
		Data\Effects\Authored\effect.valtan.sequence.four.sector.effect.json = Data\Effects\Authored\effect.valtan.sequence.four.sector.effect.json
		Data\Effects\Authored\effect.valtan.sequence.roar-charge.effect.json = Data\Effects\Authored\effect.valtan.sequence.roar-charge.effect.json
		Data\Effects\Authored\effect.valtan.sequence.six-pizza.source-warnings.effect.json = Data\Effects\Authored\effect.valtan.sequence.six-pizza.source-warnings.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.rock.active.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.rock.active.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.rock.explode.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.rock.explode.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.rock.explosion.full.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.rock.explosion.full.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.rock.telegraph.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.rock.telegraph.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.sectors.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.sectors.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.source.center-sector.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.source.center-sector.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.source.red-sector.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.source.red-sector.effect.json
		Data\Effects\Authored\effect.valtan.six-pizza.source.yellow-sector.effect.json = Data\Effects\Authored\effect.valtan.six-pizza.source.yellow-sector.effect.json
		Data\Effects\Authored\effect.valtan.sky-axe.active.effect.json = Data\Effects\Authored\effect.valtan.sky-axe.active.effect.json
		Data\Effects\Authored\effect.valtan.sky.source.chaosgate.effect.json = Data\Effects\Authored\effect.valtan.sky.source.chaosgate.effect.json
		Data\Effects\Authored\effect.valtan.sky.source.spacehole.effect.json = Data\Effects\Authored\effect.valtan.sky.source.spacehole.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_high_00.chaosgate.par_d_gstar_cgate_mad_impact_sl_11.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_high_00.chaosgate.par_d_gstar_cgate_mad_impact_sl_11.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_high_00.dust.par_c_explosion_001.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_high_00.dust.par_c_explosion_001.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_high_00.explosion.par_j_thntos_explo_003.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_high_00.explosion.par_j_thntos_explo_003.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_high_00.explosion.par_q_shipcrash_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_high_00.explosion.par_q_shipcrash_01.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_high_01.rain.par_d_rainfloor_02.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_high_01.rain.par_d_rainfloor_02.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_00.dust.par_b_wagondust_001.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_00.dust.par_b_wagondust_001.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_a_h_lightningglow_001.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_a_h_lightningglow_001.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_b_lightning_002.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_b_lightning_002.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_c_lightning_001.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_c_lightning_001.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_i_thunder_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_03.lightning.par_i_thunder_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_04.portal.par_q_simplelightwall_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_04.portal.par_q_simplelightwall_01.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_05.volcano.par_b_lava_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_05.volcano.par_b_lava_01.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_10.dust.par_h_dust_03.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_10.dust.par_h_dust_03.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_11.aura.par_f_aura_001.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_11.aura.par_f_aura_001.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_11.aura.par_g_darkaura_02_loc_int.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_11.aura.par_g_darkaura_02_loc_int.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_11.etc.par_q_rpbf_eye_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_11.etc.par_q_rpbf_eye_01.effect.json
		Data\Effects\Authored\effect.valtan.source.bfx_low_12.fire.par_q_firesh_01.effect.json = Data\Effects\Authored\effect.valtan.source.bfx_low_12.fire.par_q_firesh_01.effect.json
		Data\Effects\Authored\effect.valtan.source.default-aura.ghost.effect.json = Data\Effects\Authored\effect.valtan.source.default-aura.ghost.effect.json
		Data\Effects\Authored\effect.valtan.source.default-aura.normal.effect.json = Data\Effects\Authored\effect.valtan.source.default-aura.normal.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_d_groggyhit_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_d_groggyhit_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_e_counter_hit_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_e_counter_hit_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_k_raidgroggy_hit_01_loc_int.effect.json = Data\Effects\Authored\effect.valtan.source.fx_bs_00.hit.par_k_raidgroggy_hit_01_loc_int.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_001_pr.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_001_pr.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_002_pr.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_002_pr.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_007_pr.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_007_pr.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_009.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_d_dust_009.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_e_brandish_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_e_brandish_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_mp_dust_high_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_00.dust.par_mp_dust_high_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_01.debris.par_d_debris_003.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_01.debris.par_d_debris_003.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_01.debris.par_mp_deathmon_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_01.debris.par_mp_deathmon_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_01.distortion.par_mp_concavedis_z_08.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_01.distortion.par_mp_concavedis_z_08.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_02.gatherenergy.par_d_genergy_01_pr.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_02.gatherenergy.par_d_genergy_01_pr.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_05_l.effect.json = Data\Effects\Authored\effect.valtan.source.fx_cm_02.light.par_mp_light_05_l.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_off_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_off_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_off_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_off_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_on_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_on_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_spawn_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_itr_02326.par_l_itr_02326_spawn_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_d_rpbf_eye_01_loc_int.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_d_rpbf_eye_01_loc_int.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_d_rpbf_sk05_03_loc_int.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_d_rpbf_sk05_03_loc_int.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_rpbf_summon_001_loc_int.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.rpbf.par_rpbf_summon_001_loc_int.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_castend_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_castend_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_meshdepthline_001.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_meshdepthline_001.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_proj3_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_boss_01.tslc.par_m_tslc_proj3_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_d.par_d_rpbf_partsdestruction_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_d.par_d_rpbf_partsdestruction_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_05.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_05.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_06.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_06.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_24.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_01_24.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_06.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_06.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_11.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_11.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_12.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_12.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_13.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_04_13.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_07_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_07_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_07_2.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_07_2.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_08_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_08_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_13_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_atk_13_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_blowslash_atk_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_blowslash_atk_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_exp_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_exp_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_exp_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_cast_exp_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_castpi_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_castpi_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_crack_01_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_dash_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_dash_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_dust_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_dust_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_exp_05_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_foot_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_foot_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_hold_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_hold_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_hold_01_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_hold_01_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_light_04_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_light_04_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_light_04_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_light_04_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_magiccsat_001.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_magiccsat_001.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_potal_02_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_potal_02_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_ribbon_02_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_ribbon_02_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_06.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_sk09_exp_06.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_stonewave_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_stonewave_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_stonewave_01_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_stonewave_01_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_swing_07.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_swing_07.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_14.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_14.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_22.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_22.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_23.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_01_23.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wave_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wwind_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_wwind_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_05.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_05.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_07.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_07.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_08.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_08.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_08.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_02_08.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_05.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_05.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_08.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_06_08.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_07_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_04.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_04.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_07.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_07.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_09_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_default_05_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_default_05_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_trail_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_trail_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_atk_01_2.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_atk_01_2.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_atk_02_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_atk_02_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_dash_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_dash_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_shout_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_shout_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_stone_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_stone_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_stone_01_2.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_stone_01_2.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_trail_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_trail_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_2.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_2.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_3.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_3.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_4.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_wave_01_4.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_02_o.par_o_rpbf_atk_05_06.effect.json = Data\Effects\Authored\effect.valtan.source.fx_mn_rpbf_02_o.par_o_rpbf_atk_05_06.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_loop_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_loop_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_gravityarea_01_1.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_gravityarea_01_1.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_isez_lifeadd_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_isez_lifeadd_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_tlsc_impact_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_tlsc_impact_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_tlsccast_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_npc_m_00.par_m_tlsccast_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_filmnoise_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_filmnoise_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_02.effect.json = Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_02.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_03.effect.json = Data\Effects\Authored\effect.valtan.source.fx_post.fx_par.par_c_zoomblur_03.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_q_w_01.fx_par_10.par_q_rpbf_death_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_q_w_01.fx_par_10.par_q_rpbf_death_01.effect.json
		Data\Effects\Authored\effect.valtan.source.fx_q_w_01.fx_par_10.par_q_rpbf_shout_01.effect.json = Data\Effects\Authored\effect.valtan.source.fx_q_w_01.fx_par_10.par_q_rpbf_shout_01.effect.json
		Data\Effects\Authored\effect.valtan.source.itr-02326.off.full.effect.json = Data\Effects\Authored\effect.valtan.source.itr-02326.off.full.effect.json
		Data\Effects\Authored\effect.valtan.source.itr-02326.spawn-and-idle.effect.json = Data\Effects\Authored\effect.valtan.source.itr-02326.spawn-and-idle.effect.json
		Data\Effects\Authored\effect.valtan.source.scene_a.fx.par_j_arktrail_02_cine.effect.json = Data\Effects\Authored\effect.valtan.source.scene_a.fx.par_j_arktrail_02_cine.effect.json
		Data\Effects\Authored\effect.valtan.struggling.rock.active.effect.json = Data\Effects\Authored\effect.valtan.struggling.rock.active.effect.json
		Data\Effects\Authored\effect.valtan.struggling.rock.explode.effect.json = Data\Effects\Authored\effect.valtan.struggling.rock.explode.effect.json
		Data\Effects\Authored\effect.valtan.struggling.source-in-out.active.effect.json = Data\Effects\Authored\effect.valtan.struggling.source-in-out.active.effect.json
		Data\Effects\Authored\effect.valtan.struggling.source-portal-dash.effect.json = Data\Effects\Authored\effect.valtan.struggling.source-portal-dash.effect.json
		Data\Effects\Authored\effect.valtan.struggling.source-underfoot.active.effect.json = Data\Effects\Authored\effect.valtan.struggling.source-underfoot.active.effect.json
		Data\Effects\Authored\effect.valtan.struggling.source-yellow-roar.warning.effect.json = Data\Effects\Authored\effect.valtan.struggling.source-yellow-roar.warning.effect.json
		Data\Effects\Authored\effect.valtan.tracking-axe.large-circle.warning.effect.json = Data\Effects\Authored\effect.valtan.tracking-axe.large-circle.warning.effect.json
		Data\Effects\Authored\effect.vehicle.ancient-sea.lookinfo.ambient.effect.json = Data\Effects\Authored\effect.vehicle.ancient-sea.lookinfo.ambient.effect.json
		Data\Effects\Authored\effect.vehicle.ancient-sea.lookinfo.spawn.effect.json = Data\Effects\Authored\effect.vehicle.ancient-sea.lookinfo.spawn.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8200.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8200.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8201.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8201.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8202.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8202.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8203.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8203.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8204.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8204.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8205.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8205.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8206.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8206.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8207.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8207.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.ship.wake.8208.run_battle_1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.ship.wake.8208.run_battle_1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98520.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98520.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98521.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98521.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98522.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98522.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98522.1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98522.1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98523.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_myth.98523.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98520.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98520.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98521.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98521.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98522.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98522.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98522.1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98522.1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98523.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_ancient_sea.98523.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97300.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97300.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97310.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97310.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97320.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97320.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97330.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_aufstehen_r.97330.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_mokoboard_rainbow.95722.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_mokoboard_rainbow.95722.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98380.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98380.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98381.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98381.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98382.1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_starlight_blessing_serene.98382.1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96000.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96000.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96010.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96010.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96020.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96020.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96030.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_terpeion_gold.96030.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97730.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97730.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97731.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97731.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.1.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.2.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97732.2.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97733.0.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97733.0.full.restore.effect.json
		Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97733.1.full.restore.effect.json = Data\Effects\Authored\effect.vehicle.vehicle_unicorn_tube_sea.97733.1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17000.ba0.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17000.ba0.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17000.ba1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17000.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17000.ba2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17000.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17000.ba3.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17000.ba3.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17030.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17030.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17030.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17030.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17040.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17040.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17040.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17040.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17060.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17060.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17060.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17060.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17080.clip1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17080.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17080.clip2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17080.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17080.clip2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17080.clip2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17090.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17090.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17090.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17090.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17100.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17100.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17100.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17100.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17110.clip2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17110.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17110.clip2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17110.clip2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17110.clip3.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17110.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17110.clip3.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17110.clip3.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17140.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17140.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17140.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17140.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip1.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip1.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip3.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.clip3.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.clip3.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17170.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17170.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba0.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba0.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba1.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba1.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.ba3.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.ba3.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.clip3.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.clip3.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17240.clip4.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17240.clip4.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17250.clip1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17250.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17250.clip1.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17250.clip1.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17250.clip2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17250.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17250.clip2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17250.clip2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip1.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip1.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip1.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip1.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip2.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip2.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip2.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip2.unified.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip5.full.restore.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip5.full.restore.effect.json
		Data\Effects\Authored\effect.warlord.skill.17820.clip5.unified.effect.json = Data\Effects\Authored\effect.warlord.skill.17820.clip5.unified.effect.json
		Data\Effects\Authored\effect.world.entry_aura.active.effect.json = Data\Effects\Authored\effect.world.entry_aura.active.effect.json
		Data\Effects\Authored\effect.world.entry_aura.effect.json = Data\Effects\Authored\effect.world.entry_aura.effect.json
		Data\Effects\Authored\effect.world.item.destruction_bomb.effect.json = Data\Effects\Authored\effect.world.item.destruction_bomb.effect.json
		Data\Effects\Authored\effect.world.item.destruction_bomb.flight.effect.json = Data\Effects\Authored\effect.world.item.destruction_bomb.flight.effect.json
		Data\Effects\Authored\effect.world.item.holy_charm.effect.json = Data\Effects\Authored\effect.world.item.holy_charm.effect.json
		Data\Effects\Authored\effect.world.item.time_stop.effect.json = Data\Effects\Authored\effect.world.item.time_stop.effect.json
		Data\Effects\Authored\effect.world.item.whirlwind_grenade.effect.json = Data\Effects\Authored\effect.world.item.whirlwind_grenade.effect.json
		Data\Effects\Authored\effect.world.item.whirlwind_grenade.flight.effect.json = Data\Effects\Authored\effect.world.item.whirlwind_grenade.flight.effect.json
		Data\Effects\Authored\effect.world.mouse_click.effect.json = Data\Effects\Authored\effect.world.mouse_click.effect.json
		Data\Effects\Authored\effect.world.move_destination.effect.json = Data\Effects\Authored\effect.world.move_destination.effect.json
		Data\Effects\Authored\effect.world.ping.effect.json = Data\Effects\Authored\effect.world.ping.effect.json
		Data\Effects\Authored\effect.world.respawn_aura.effect.json = Data\Effects\Authored\effect.world.respawn_aura.effect.json
		Data\Effects\Authored\effect.world.target_reticle.effect.json = Data\Effects\Authored\effect.world.target_reticle.effect.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "NpcActionCues", "NpcActionCues", "{2F4E9DDB-AE86-54FF-8A3C-F11E9864746D}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\NpcActionCues\NPC_58700.npcactioncues.json = Data\Effects\NpcActionCues\NPC_58700.npcactioncues.json
		Data\Effects\NpcActionCues\NPC_59030.npcactioncues.json = Data\Effects\NpcActionCues\NPC_59030.npcactioncues.json
		Data\Effects\NpcActionCues\NPC_59060.npcactioncues.json = Data\Effects\NpcActionCues\NPC_59060.npcactioncues.json
		Data\Effects\NpcActionCues\NPC_59504.npcactioncues.json = Data\Effects\NpcActionCues\NPC_59504.npcactioncues.json
		Data\Effects\NpcActionCues\NPC_59620.npcactioncues.json = Data\Effects\NpcActionCues\NPC_59620.npcactioncues.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Sequences", "Sequences", "{E1822C40-7D67-5B03-BF33-1ACFCDBD77A0}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\Sequences\effect.artist.skill.31930.full.restore.effectsequence.json = Data\Effects\Sequences\effect.artist.skill.31930.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.dimensionmaster.skill.2050520.full.restore.effectsequence.json = Data\Effects\Sequences\effect.dimensionmaster.skill.2050520.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.dimensionmaster.skill.2050540.full.restore.effectsequence.json = Data\Effects\Sequences\effect.dimensionmaster.skill.2050540.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.dimensionmaster.skill.2050540.tuning.restore.effectsequence.json = Data\Effects\Sequences\effect.dimensionmaster.skill.2050540.tuning.restore.effectsequence.json
		Data\Effects\Sequences\effect.guardianknight.skill.49420.clip.0.full.restore.effectsequence.json = Data\Effects\Sequences\effect.guardianknight.skill.49420.clip.0.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.lancemaster.skill.34630.clip1.full.restore.effectsequence.json = Data\Effects\Sequences\effect.lancemaster.skill.34630.clip1.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.lancemaster.skill.34630.clip2.full.restore.effectsequence.json = Data\Effects\Sequences\effect.lancemaster.skill.34630.clip2.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.lancemaster.skill.34630.full.restore.effectsequence.json = Data\Effects\Sequences\effect.lancemaster.skill.34630.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.warlord.skill.17250.clip1.full.restore.effectsequence.json = Data\Effects\Sequences\effect.warlord.skill.17250.clip1.full.restore.effectsequence.json
		Data\Effects\Sequences\effect.warlord.skill.17250.clip2.full.restore.effectsequence.json = Data\Effects\Sequences\effect.warlord.skill.17250.clip2.full.restore.effectsequence.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "V2", "V2", "{A05499D1-EA4F-54BB-B875-2E0F71BF24FF}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\V2\EffectRoles.json = Data\Effects\V2\EffectRoles.json
		Data\Effects\V2\Independent.json = Data\Effects\V2\Independent.json
		Data\Effects\V2\TextureSlotUsage.v1.json = Data\Effects\V2\TextureSlotUsage.v1.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Authored", "Authored", "{E62C7DD5-06BD-5F03-9D85-4EA87526161F}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\V2\Authored\bingo.bomb.mark_1.effectv2.json = Data\Effects\V2\Authored\bingo.bomb.mark_1.effectv2.json
		Data\Effects\V2\Authored\bingo.bomb_1.effectv2.json = Data\Effects\V2\Authored\bingo.bomb_1.effectv2.json
		Data\Effects\V2\Authored\bingo.hammer.warning.arrow.effectv2.json = Data\Effects\V2\Authored\bingo.hammer.warning.arrow.effectv2.json
		Data\Effects\V2\Authored\bingo.hammer_1.effectv2.json = Data\Effects\V2\Authored\bingo.hammer_1.effectv2.json
		Data\Effects\V2\Authored\bingo.skull.red_1.effectv2.json = Data\Effects\V2\Authored\bingo.skull.red_1.effectv2.json
		Data\Effects\V2\Authored\bingo.skull.white_1.effectv2.json = Data\Effects\V2\Authored\bingo.skull.white_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.appear.card_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.appear.card_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.appear.smoke_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.appear.smoke_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.blue_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.blue_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.green_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.green_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.orange_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.orange_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.purple_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.purple_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.spread_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.spread_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.ball.smoke.yellow_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.ball.smoke.yellow_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.blur_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.blur_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.blur_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.blur_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.blur_3.effectv2.json = Data\Effects\V2\Authored\boss.kouku.blur_3.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.clober.black_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.clober.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.clober.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.clober.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.dia.black_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.dia.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.dia.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.dia.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.heart.black_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.heart.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.heart.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.heart.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.spade.black_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.spade.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.spade.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.spade.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.card.white_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.card.white_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.curtain_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.curtain_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.clap.smoke.l_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.clap.smoke.l_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.clap.star_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.clap.star_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.clover_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.clover_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.dia_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.dia_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.heart_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.heart_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.light.glow_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.light.glow_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.light_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.light_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.radial.blue_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.radial.blue_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.radial.green_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.radial.green_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.radial.orange_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.radial.orange_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.radial.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.radial.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dance.spade_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dance.spade_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.converge_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.converge_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.converge_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.converge_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.converge_3.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.converge_3.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.shield_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.shield_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.star.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.star.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.star.smoke_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.star.smoke_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.disarm.star_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.disarm.star_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dj.cardrain.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dj.cardrain.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.dj.delivery.effectv2.json = Data\Effects\V2\Authored\boss.kouku.dj.delivery.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.fear.face_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.fear.face_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.core.big_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.core.big_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.core.big_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.core.big_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.core.small_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.core.small_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.core.small_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.core.small_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.heart.big_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.heart.big_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.heart.small_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.heart.small_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.star.bang_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.star.bang_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.star.black_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.star.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.star.radial_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.star.radial_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.find.star.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.find.star.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.joker.hammer.burst_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.joker.hammer.burst_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_3.effectv2.json = Data\Effects\V2\Authored\boss.kouku.joker.hammer.decal_3.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.beam_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.beam_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.beam_3.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.beam_3.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.converge_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.converge_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.corn_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.corn_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.corn_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.corn_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.distortion_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.distortion_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.expansion_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.expansion_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.expansion_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.expansion_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.glow_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.glow_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.glow_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.glow_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.glow_3.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.glow_3.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.glow_4.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.glow_4.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.laser.glow_5.effectv2.json = Data\Effects\V2\Authored\boss.kouku.laser.glow_5.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.medusa.ball.blue_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.medusa.ball.blue_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.medusa.ball.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.medusa.ball.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.medusa.blue_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.medusa.blue_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.medusa.laser_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.medusa.laser_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.medusa.red_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.medusa.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.aura_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.aura_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.e_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.e_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.e_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.e_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.n_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.n_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.n_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.n_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.s_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.s_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.s_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.s_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.w_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.w_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.decal.w_2.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.decal.w_2.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.star.back.ccl_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.star.back.ccl_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.star.back.cl_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.star.back.cl_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.star.front.ccl_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.star.front.ccl_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.pizza.star.front.cl_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.pizza.star.front.cl_1.effectv2.json
		Data\Effects\V2\Authored\boss.kouku.radial_1.effectv2.json = Data\Effects\V2\Authored\boss.kouku.radial_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.axe.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.axe.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.axe.decal_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.axe.decal_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.axe_elec_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.axe_elec_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.axe_fall_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.axe_fall_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.axe_stuck_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.axe_stuck_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blackhole_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blackhole_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blur_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blur_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blur_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blur_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blur_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blur_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blur_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blur_4.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.blur_5.effectv2.json = Data\Effects\V2\Authored\boss.valtan.blur_5.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe.red_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe.red_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe.red_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe.red_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe.red_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe.red_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe.red_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe.red_4.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.breathe_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.breathe_4.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.ca_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.ca_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.chase.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.chase.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.chase.decal_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.chase.decal_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.chase.spread_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.chase.spread_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.decal_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.decal_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.aura_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.aura_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.aura_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.aura_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.black_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.black_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.black_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.black_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.black_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.egg.cyan_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.egg.cyan_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.fluid_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.fluid_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_4.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_5.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_5.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hand_6.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hand_6.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hit_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hit_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hit_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hit_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.hit_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.hit_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.portal.black_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.portal.black_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.portal.cyan_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.portal.cyan_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.project-tuned.sequence.trash.pulse.effectv2.json = Data\Effects\V2\Authored\boss.valtan.project-tuned.sequence.trash.pulse.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.rock-pillar.active.effectv2.json = Data\Effects\V2\Authored\boss.valtan.rock-pillar.active.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.rock-pillar.explosion.effectv2.json = Data\Effects\V2\Authored\boss.valtan.rock-pillar.explosion.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.comet_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.comet_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.comet_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.comet_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.converge_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.converge_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.emit_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.emit_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.fog_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.fog_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.fog_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.fog_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.fog_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.fog_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.shout.fog_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.shout.fog_4.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.flash_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.flash_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.smoke_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.smoke_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.sonic_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.sonic_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.sonic_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.sonic_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.sonic_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.sonic_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.wave_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.wave_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.six.wave_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.six.wave_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.spread_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.spread_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.spread_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.spread_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.sw_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.sw_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.twinkle_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.twinkle_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.twohand.decal_1.effectv2.json = Data\Effects\V2\Authored\boss.valtan.twohand.decal_1.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.twohand.decal_2.effectv2.json = Data\Effects\V2\Authored\boss.valtan.twohand.decal_2.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.twohand.decal_3.effectv2.json = Data\Effects\V2\Authored\boss.valtan.twohand.decal_3.effectv2.json
		Data\Effects\V2\Authored\boss.valtan.twohand.decal_4.effectv2.json = Data\Effects\V2\Authored\boss.valtan.twohand.decal_4.effectv2.json
		Data\Effects\V2\Authored\cardmaze.symbol.club.effectv2.json = Data\Effects\V2\Authored\cardmaze.symbol.club.effectv2.json
		Data\Effects\V2\Authored\cardmaze.symbol.diamond.effectv2.json = Data\Effects\V2\Authored\cardmaze.symbol.diamond.effectv2.json
		Data\Effects\V2\Authored\cardmaze.symbol.heart.effectv2.json = Data\Effects\V2\Authored\cardmaze.symbol.heart.effectv2.json
		Data\Effects\V2\Authored\cardmaze.symbol.spade.effectv2.json = Data\Effects\V2\Authored\cardmaze.symbol.spade.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.appear_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.appear_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.blur_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.blur_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.core_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.core_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.core_2.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.core_2.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.hammer_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.hammer_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.hit_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.hit_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.magic_circle_1.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.magic_circle_1.effectv2.json
		Data\Effects\V2\Authored\esther.balthorr.magic_circle_2.effectv2.json = Data\Effects\V2\Authored\esther.balthorr.magic_circle_2.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.appear_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.appear_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.appear_2.effectv2.json = Data\Effects\V2\Authored\esther.ninave.appear_2.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.arrow_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.arrow_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.blur_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.blur_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.impact_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.impact_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.impact_2.effectv2.json = Data\Effects\V2\Authored\esther.ninave.impact_2.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.impact_3.effectv2.json = Data\Effects\V2\Authored\esther.ninave.impact_3.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.radial_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.radial_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.thunder_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.thunder_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.water_1.effectv2.json = Data\Effects\V2\Authored\esther.ninave.water_1.effectv2.json
		Data\Effects\V2\Authored\esther.ninave.water_2.effectv2.json = Data\Effects\V2\Authored\esther.ninave.water_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.appear_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.appear_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.aura_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.aura_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.aura_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.aura_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.aura_3.effectv2.json = Data\Effects\V2\Authored\esther.thirain.aura_3.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.aura_4.effectv2.json = Data\Effects\V2\Authored\esther.thirain.aura_4.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_10.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_10.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_11.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_11.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_12.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_12.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_13.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_13.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_14.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_14.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_15.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_15.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_16.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_16.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_17.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_17.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_3.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_3.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_4.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_4.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_5.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_5.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_6.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_6.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_7.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_7.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_8.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_8.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blade_9.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blade_9.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blur_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blur_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.blur_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.blur_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_3.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_3.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_4.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_4.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_5.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_5.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.decal_6.effectv2.json = Data\Effects\V2\Authored\esther.thirain.decal_6.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.distortion_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.distortion_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.lumen_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.lumen_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.lumen_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.lumen_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.lumen_3.effectv2.json = Data\Effects\V2\Authored\esther.thirain.lumen_3.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.lumen_4.effectv2.json = Data\Effects\V2\Authored\esther.thirain.lumen_4.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.lumen_5.effectv2.json = Data\Effects\V2\Authored\esther.thirain.lumen_5.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.magic_circle_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.magic_circle_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.magic_circle_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.magic_circle_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.point_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.point_1.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.point_2.effectv2.json = Data\Effects\V2\Authored\esther.thirain.point_2.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.point_3.effectv2.json = Data\Effects\V2\Authored\esther.thirain.point_3.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.point_4.effectv2.json = Data\Effects\V2\Authored\esther.thirain.point_4.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.point_5.effectv2.json = Data\Effects\V2\Authored\esther.thirain.point_5.effectv2.json
		Data\Effects\V2\Authored\esther.thirain.trail_1.effectv2.json = Data\Effects\V2\Authored\esther.thirain.trail_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.appear_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.appear_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.blur_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.blur_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.blur_2.effectv2.json = Data\Effects\V2\Authored\esther.wei.blur_2.effectv2.json
		Data\Effects\V2\Authored\esther.wei.blur_3.effectv2.json = Data\Effects\V2\Authored\esther.wei.blur_3.effectv2.json
		Data\Effects\V2\Authored\esther.wei.blur_4.effectv2.json = Data\Effects\V2\Authored\esther.wei.blur_4.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_2.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_2.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_3.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_3.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_sw_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_sw_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_sw_2.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_sw_2.effectv2.json
		Data\Effects\V2\Authored\esther.wei.dochul_sw_3.effectv2.json = Data\Effects\V2\Authored\esther.wei.dochul_sw_3.effectv2.json
		Data\Effects\V2\Authored\esther.wei.magic_circle_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.magic_circle_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.magic_circle_2.effectv2.json = Data\Effects\V2\Authored\esther.wei.magic_circle_2.effectv2.json
		Data\Effects\V2\Authored\esther.wei.magic_circle_3.effectv2.json = Data\Effects\V2\Authored\esther.wei.magic_circle_3.effectv2.json
		Data\Effects\V2\Authored\esther.wei.magic_circle_4.effectv2.json = Data\Effects\V2\Authored\esther.wei.magic_circle_4.effectv2.json
		Data\Effects\V2\Authored\esther.wei.trail_1.effectv2.json = Data\Effects\V2\Authored\esther.wei.trail_1.effectv2.json
		Data\Effects\V2\Authored\esther.wei.trail_2.effectv2.json = Data\Effects\V2\Authored\esther.wei.trail_2.effectv2.json
		Data\Effects\V2\Authored\kouku.bingo.encore.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.bingo.encore.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.bingo.ending.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.bingo.ending.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate1.authored.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate1.authored.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate1.authored.p4.popup.transition.fade.effectv2.json = Data\Effects\V2\Authored\kouku.gate1.authored.p4.popup.transition.fade.effectv2.json
		Data\Effects\V2\Authored\kouku.gate1.full.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate1.full.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate2.clear.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate2.clear.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate2.intro.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate2.intro.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate2.maze.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate2.maze.fade.black.effectv2.json
		Data\Effects\V2\Authored\kouku.gate3.intro.fade.black.effectv2.json = Data\Effects\V2\Authored\kouku.gate3.intro.fade.black.effectv2.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Bindings", "Bindings", "{677B4893-0CF4-58C2-8EC4-D3DEB6FAD4AC}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\V2\Bindings\BOSS_VALTAN.effectv2bindings.json = Data\Effects\V2\Bindings\BOSS_VALTAN.effectv2bindings.json
		Data\Effects\V2\Bindings\MN_RPCT_05.effectv2bindings.json = Data\Effects\V2\Bindings\MN_RPCT_05.effectv2bindings.json
		Data\Effects\V2\Bindings\MN_RPCT_06.effectv2bindings.json = Data\Effects\V2\Bindings\MN_RPCT_06.effectv2bindings.json
		Data\Effects\V2\Bindings\NPC_58700.effectv2bindings.json = Data\Effects\V2\Bindings\NPC_58700.effectv2bindings.json
		Data\Effects\V2\Bindings\NPC_59030.effectv2bindings.json = Data\Effects\V2\Bindings\NPC_59030.effectv2bindings.json
		Data\Effects\V2\Bindings\NPC_59060.effectv2bindings.json = Data\Effects\V2\Bindings\NPC_59060.effectv2bindings.json
		Data\Effects\V2\Bindings\NPC_59504.effectv2bindings.json = Data\Effects\V2\Bindings\NPC_59504.effectv2bindings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Groups", "Groups", "{0752A33C-3676-5057-8C6B-5F5CA03EBECA}"
	ProjectSection(SolutionItems) = preProject
		Data\Effects\V2\Groups\bingo.bomb.effectv2group.json = Data\Effects\V2\Groups\bingo.bomb.effectv2group.json
		Data\Effects\V2\Groups\bingo.bomb.mark.effectv2group.json = Data\Effects\V2\Groups\bingo.bomb.mark.effectv2group.json
		Data\Effects\V2\Groups\bingo.hammer.effectv2group.json = Data\Effects\V2\Groups\bingo.hammer.effectv2group.json
		Data\Effects\V2\Groups\bingo.skull.red.effectv2group.json = Data\Effects\V2\Groups\bingo.skull.red.effectv2group.json
		Data\Effects\V2\Groups\bingo.skull.white.effectv2group.json = Data\Effects\V2\Groups\bingo.skull.white.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.appear.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.appear.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.ball.smoke.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.ball.smoke.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.clober.black.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.clober.black.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.clober.red.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.clober.red.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.dia.black.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.dia.black.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.dia.red.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.dia.red.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.heart.black.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.heart.black.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.heart.red.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.heart.red.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.spade.black.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.spade.black.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.card.spade.red.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.card.spade.red.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.dance.clap.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.dance.clap.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.dance.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.dance.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.disarm.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.disarm.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.fear.face.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.fear.face.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.find.core.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.find.core.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.find.heart.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.find.heart.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.find.star.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.find.star.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.joker.hammer.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.joker.hammer.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.laser.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.laser.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.medusa.blue.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.medusa.blue.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.medusa.laser.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.medusa.laser.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.medusa.red.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.medusa.red.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.aura.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.aura.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.e.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.e.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.n.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.n.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.s.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.s.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.star.ccl.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.star.ccl.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.star.cl.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.star.cl.effectv2group.json
		Data\Effects\V2\Groups\boss.kouku.pizza.w.effectv2group.json = Data\Effects\V2\Groups\boss.kouku.pizza.w.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.axe.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.axe.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.blackhole.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.blackhole.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.breathe.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.breathe.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.breathe.red.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.breathe.red.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.impact.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.impact.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.magicball.aura.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.magicball.aura.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.magicball.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.magicball.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.portal.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.portal.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.pounding.chase.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.pounding.chase.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.pounding.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.pounding.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.project-tuned.sequence.trash.pulse-group.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.project-tuned.sequence.trash.pulse-group.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.rock-pillar.sequence.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.rock-pillar.sequence.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.shout.burst.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.shout.burst.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.shout.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.shout.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.six.sonic.after.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.six.sonic.after.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.six.sonic.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.six.sonic.effectv2group.json
		Data\Effects\V2\Groups\boss.valtan.twohand.effectv2group.json = Data\Effects\V2\Groups\boss.valtan.twohand.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.exit.club.effectv2group.json = Data\Effects\V2\Groups\cardmaze.exit.club.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.exit.diamond.effectv2group.json = Data\Effects\V2\Groups\cardmaze.exit.diamond.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.exit.heart.effectv2group.json = Data\Effects\V2\Groups\cardmaze.exit.heart.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.exit.spade.effectv2group.json = Data\Effects\V2\Groups\cardmaze.exit.spade.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.mark.club.effectv2group.json = Data\Effects\V2\Groups\cardmaze.mark.club.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.mark.diamond.effectv2group.json = Data\Effects\V2\Groups\cardmaze.mark.diamond.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.mark.heart.effectv2group.json = Data\Effects\V2\Groups\cardmaze.mark.heart.effectv2group.json
		Data\Effects\V2\Groups\cardmaze.mark.spade.effectv2group.json = Data\Effects\V2\Groups\cardmaze.mark.spade.effectv2group.json
		Data\Effects\V2\Groups\kouku.gate2.intro.fade.effectv2group.json = Data\Effects\V2\Groups\kouku.gate2.intro.fade.effectv2group.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Encounters", "Encounters", "{CE7700BF-E4A3-585A-9FF9-E2AAAE2BD70F}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Bern", "Bern", "{67846088-7213-5D35-BCCA-72D88B4553F3}"
	ProjectSection(SolutionItems) = preProject
		Data\Encounters\Bern\BernEntranceCamera.json = Data\Encounters\Bern\BernEntranceCamera.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KoukuSaydon", "KoukuSaydon", "{73677C61-11D9-505E-85BF-1C9C443B7670}"
	ProjectSection(SolutionItems) = preProject
		Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json = Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Valtan", "Valtan", "{E7D5C6D7-06F2-551E-BAD2-B76C71CF0641}"
	ProjectSection(SolutionItems) = preProject
		Data\Encounters\Valtan\ValtanChargeImpactActions.json = Data\Encounters\Valtan\ValtanChargeImpactActions.json
		Data\Encounters\Valtan\ValtanCinematicCamera.json = Data\Encounters\Valtan\ValtanCinematicCamera.json
		Data\Encounters\Valtan\ValtanCombatObjects.json = Data\Encounters\Valtan\ValtanCombatObjects.json
		Data\Encounters\Valtan\ValtanCoverPiercingActions.json = Data\Encounters\Valtan\ValtanCoverPiercingActions.json
		Data\Encounters\Valtan\ValtanDebugAudition.json = Data\Encounters\Valtan\ValtanDebugAudition.json
		Data\Encounters\Valtan\ValtanEncounter.json = Data\Encounters\Valtan\ValtanEncounter.json
		Data\Encounters\Valtan\ValtanPatternRotations.json = Data\Encounters\Valtan\ValtanPatternRotations.json
		Data\Encounters\Valtan\ValtanPropBreakActions.json = Data\Encounters\Valtan\ValtanPropBreakActions.json
		Data\Encounters\Valtan\ValtanWallContactActions.json = Data\Encounters\Valtan\ValtanWallContactActions.json
		Data\Encounters\Valtan\ValtanWorldEvents.json = Data\Encounters\Valtan\ValtanWorldEvents.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Guide", "Guide", "{8616D941-E1BE-5A3A-8392-5C7CA7632B1B}"
	ProjectSection(SolutionItems) = preProject
		Data\Guide\GuideCatalog.json = Data\Guide\GuideCatalog.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "DimensionMaster", "DimensionMaster", "{F53DE104-44C5-533C-AE7B-2E6A938BBB82}"
	ProjectSection(SolutionItems) = preProject
		Data\Guide\DimensionMaster\Combat.json = Data\Guide\DimensionMaster\Combat.json
		Data\Guide\DimensionMaster\Placement.json = Data\Guide\DimensionMaster\Placement.json
		Data\Guide\DimensionMaster\Prompts.json = Data\Guide\DimensionMaster\Prompts.json
		Data\Guide\DimensionMaster\Triggers.json = Data\Guide\DimensionMaster\Triggers.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Items", "Items", "{66A58081-1AF6-57DA-9EF9-DF55853A4E32}"
	ProjectSection(SolutionItems) = preProject
		Data\Items\ItemCatalog.json = Data\Items\ItemCatalog.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KoukuSaydon", "KoukuSaydon", "{32A17A30-215B-56ED-A0FF-07DFFA1ACAA0}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Gate1", "Gate1", "{D48AC68E-D619-523E-BF1F-727C1C613041}"
	ProjectSection(SolutionItems) = preProject
		Data\KoukuSaydon\Gate1\KoukuSaydonComposition.json = Data\KoukuSaydon\Gate1\KoukuSaydonComposition.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Maps", "Maps", "{F8B345CD-1FBC-5517-9E73-4CC518772D2B}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\MapCatalog.json = Data\Maps\MapCatalog.json
		Data\Maps\SequenceViewer.labels.json = Data\Maps\SequenceViewer.labels.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Authoring", "Authoring", "{FAAF6643-F55B-511F-98FA-7B296A82AA0D}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_BER_BERNCASTLE", "LV_BER_BERNCASTLE", "{2D8B9D4C-9DC3-5716-814F-9AA192A5B77F}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapeffects.json = Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapeffects.json
		Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.maplights.json = Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.maplights.json
		Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapmaterials.json = Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapmaterials.json
		Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapplacements = Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapplacements
		Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapwater.json = Data\Maps\Authoring\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapwater.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_DEV_TRAINING_GROUND", "LV_DEV_TRAINING_GROUND", "{06B041F1-05E2-576A-898A-5C8F087002B3}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapplacements = Data\Maps\Authoring\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL00", "LV_LOBBY_CLASSSELECT_SL00", "{C92DFC48-427A-5DB0-87EA-3341CD30DD62}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.maplights.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.maplights.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.worldsequences.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.worldsequences.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL01", "LV_LOBBY_CLASSSELECT_SL01", "{75864815-DDCF-55D2-BBBA-E847445AF2BD}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL03", "LV_LOBBY_CLASSSELECT_SL03", "{97C1370E-3C66-5D77-9F7D-A2A729BD8CC5}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL08", "LV_LOBBY_CLASSSELECT_SL08", "{DFE007B8-2407-56BA-AF4C-19CE0D50448A}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL10", "LV_LOBBY_CLASSSELECT_SL10", "{91207229-C6D6-50E2-8C7D-67CBFAB046FD}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL12", "LV_LOBBY_CLASSSELECT_SL12", "{9C2CEA37-7855-53B4-8452-109FA427E302}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapmaterials.json = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapmaterials.json
		Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapplacements = Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_HEARTRB_ED", "LV_LUT_HEARTRB_ED", "{30572973-77B5-5233-866B-7638F9FFFCF4}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02306.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02306.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02307.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02307.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02308.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02308.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02309.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02309.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02310.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02310.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02311.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02311.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02315.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02315.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02316.debrisrecipe.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02316.debrisrecipe.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.camerashots.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.camerashots.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployplacements = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployplacements
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.destructionsimulation.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.destructionsimulation.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapeffects.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapeffects.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.maplights.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.maplights.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapmaterials.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapmaterials.json
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapplacements = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapplacements
		Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.worldsequences.json = Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.worldsequences.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_MIDNIGHTC_ED", "LV_LUT_MIDNIGHTC_ED", "{B580BE8A-5437-51AB-857B-9E62D2E5B103}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.camerashots.json = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.camerashots.json
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.deployplacements = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.deployplacements
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.maplights.json = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.maplights.json
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapmaterials.json = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapmaterials.json
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapmotions.json = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapmotions.json
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapplacements = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapplacements
		Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.worldsequences.json = Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.worldsequences.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_OCN_EVENTIS_MHP", "LV_OCN_EVENTIS_MHP", "{C2B8D718-DFEA-5AB0-A1B7-13B3210761F8}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.camerashots.json = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.camerashots.json
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.maplights.json = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.maplights.json
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapmaterials.json = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapmaterials.json
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapplacements = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapplacements
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapwater.json = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapwater.json
		Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.worldsequences.json = Data\Maps\Authoring\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.worldsequences.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_PVP_COLOSSEUM", "LV_PVP_COLOSSEUM", "{14E91526-66BC-5C12-8BAC-9EE70F0D5CC3}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.maplights.json = Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.maplights.json
		Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapmaterials.json = Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapmaterials.json
		Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapplacements = Data\Maps\Authoring\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_SHS_RCARENA_D", "LV_SHS_RCARENA_D", "{6DE08018-AB4A-59CA-A15D-8E597050F1F3}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Authoring\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapplacements = Data\Maps\Authoring\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Editor", "Editor", "{411FA728-6882-57C9-AF94-0394AF7C7FED}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Editor\ACTIVE.maparea = Data\Maps\Editor\ACTIVE.maparea
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Imported", "Imported", "{328877D6-992D-5FDA-8F16-FB2967107F54}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_BER_BERNCASTLE", "LV_BER_BERNCASTLE", "{F82E31A2-CC09-564E-BBF8-DA4DD2AF1471}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapset = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapset
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.renderprofiles.json = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.renderprofiles.json
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.shards.receipt.json = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.shards.receipt.json
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_BASE.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_BASE.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_BASE.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_BASE.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_ISLAND00.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_ISLAND00.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_ISLAND00.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_ISLAND00.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_LANDSCAPE.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_LANDSCAPE.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_LANDSCAPE.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_LANDSCAPE.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_MaterialExtra00.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_MaterialExtra00.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_MaterialExtra00.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_MaterialExtra00.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM00.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM00.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM00.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM00.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM01.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM01.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM01.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM01.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM02.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM02.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM02.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM02.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM03.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM03.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM03.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM03.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM04.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM04.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM04.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM04.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM05.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM05.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM05.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM05.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM06.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM06.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM06.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM06.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM07.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM07.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM07.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM07.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM08.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM08.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM08.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_RNM08.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL00.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL00.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL00.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL00.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL01.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL01.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL01.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL01.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL02.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL02.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL02.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL02.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL03.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL03.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL03.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL03.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL04.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL04.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL04.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL04.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL05.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL05.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL05.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL05.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL06.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL06.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL06.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL06.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL07.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL07.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL07.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL07.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL08.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL08.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL08.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL08.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL09.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL09.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL09.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL09.mapplacements
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL10.mapassets = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL10.mapassets
		Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL10.mapplacements = Data\Maps\Imported\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE_SL10.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_DEV_TRAINING_GROUND", "LV_DEV_TRAINING_GROUND", "{5750AF10-1DED-57C9-A253-C6B4C0500F6D}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapassets = Data\Maps\Imported\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapassets
		Data\Maps\Imported\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapplacements = Data\Maps\Imported\LV_DEV_TRAINING_GROUND\LV_DEV_TRAINING_GROUND.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL00", "LV_LOBBY_CLASSSELECT_SL00", "{A3F0547E-06F3-55CE-B9C5-B4CC74D17837}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapassets
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL01", "LV_LOBBY_CLASSSELECT_SL01", "{1B19FB48-6946-5532-944B-2D5F54C759DE}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL01\LV_LOBBY_CLASSSELECT_SL01.mapassets
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL03", "LV_LOBBY_CLASSSELECT_SL03", "{FE67B544-6FDC-5184-A46F-853A94BCCB11}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL03\LV_LOBBY_CLASSSELECT_SL03.mapassets
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL08", "LV_LOBBY_CLASSSELECT_SL08", "{7CED3764-384A-50FD-9FBA-71AAB73A2C6A}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL08\LV_LOBBY_CLASSSELECT_SL08.mapassets
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL10", "LV_LOBBY_CLASSSELECT_SL10", "{9793008E-9895-57B7-B97B-E314D28B4E39}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL10\LV_LOBBY_CLASSSELECT_SL10.mapassets
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL12", "LV_LOBBY_CLASSSELECT_SL12", "{EEC32F58-11FF-55AE-B36E-7C1C68FF4574}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapassets = Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL12\LV_LOBBY_CLASSSELECT_SL12.mapassets
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_HEARTRB_ED", "LV_LUT_HEARTRB_ED", "{A4E3D7DA-6EEC-5F80-9A2D-B63155EEBD33}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapset = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapset
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_BASE.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_BASE.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_BASE.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_BASE.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_PS.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_PS.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_PS.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_PS.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL00.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL00.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL00.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL00.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL01.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL01.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL01.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL01.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL02.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL02.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL02.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL02.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL03.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL03.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL03.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL03.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL04.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL04.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL04.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL04.mapplacements
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL05.mapassets = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL05.mapassets
		Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL05.mapplacements = Data\Maps\Imported\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED_SL05.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_MIDNIGHTC_ED", "LV_LUT_MIDNIGHTC_ED", "{15A1E9EA-9BC4-513A-89A6-894545E731D5}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.build.receipt.json = Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.build.receipt.json
		Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.deployassets = Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.deployassets
		Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapassets = Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapassets
		Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapplacements = Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.mapplacements
		Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.nonstatic.inventory.json = Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.nonstatic.inventory.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_OCN_EVENTIS_MHP", "LV_OCN_EVENTIS_MHP", "{4E0CE829-4EAC-5261-B429-8A752A45548B}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.build.receipt.json = Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.build.receipt.json
		Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.landscape-merge.receipt.json = Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.landscape-merge.receipt.json
		Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapassets = Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapassets
		Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapplacements = Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.mapplacements
		Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.renderprofiles.json = Data\Maps\Imported\LV_OCN_EVENTIS_MHP\LV_OCN_EVENTIS_MHP.renderprofiles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_PVP_COLOSSEUM", "LV_PVP_COLOSSEUM", "{EB97F70B-7F87-5488-8125-A2D304B78545}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.build.receipt.json = Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.build.receipt.json
		Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapassets = Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapassets
		Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapplacements = Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.mapplacements
		Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.renderprofiles.json = Data\Maps\Imported\LV_PVP_COLOSSEUM\LV_PVP_COLOSSEUM.renderprofiles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_SHS_RCARENA_D", "LV_SHS_RCARENA_D", "{2545BAEB-F7FE-5C33-9F8D-67A1DA14CD98}"
	ProjectSection(SolutionItems) = preProject
		Data\Maps\Imported\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapassets = Data\Maps\Imported\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapassets
		Data\Maps\Imported\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapplacements = Data\Maps\Imported\LV_SHS_RCARENA_D\LV_SHS_RCARENA_D.mapplacements
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Navigation", "Navigation", "{6D8282A6-EBEB-5275-905F-7C3D212FD1D7}"
	ProjectSection(SolutionItems) = preProject
		Data\Navigation\LV_BER_BERNCASTLE.Bern.navpaint = Data\Navigation\LV_BER_BERNCASTLE.Bern.navpaint
		Data\Navigation\LV_BER_BERNCASTLE.Bern.navsource = Data\Navigation\LV_BER_BERNCASTLE.Bern.navsource
		Data\Navigation\LV_BER_BERNCASTLE.Bern2.navsource = Data\Navigation\LV_BER_BERNCASTLE.Bern2.navsource
		Data\Navigation\LV_BER_BERNCASTLE.Bern3.navpaint = Data\Navigation\LV_BER_BERNCASTLE.Bern3.navpaint
		Data\Navigation\LV_BER_BERNCASTLE.Bern3.navsource = Data\Navigation\LV_BER_BERNCASTLE.Bern3.navsource
		Data\Navigation\LV_BER_BERNCASTLE.BernSea.navsource = Data\Navigation\LV_BER_BERNCASTLE.BernSea.navsource
		Data\Navigation\LV_BER_BERNCASTLE.navpaint = Data\Navigation\LV_BER_BERNCASTLE.navpaint
		Data\Navigation\LV_BER_BERNCASTLE.navregions = Data\Navigation\LV_BER_BERNCASTLE.navregions
		Data\Navigation\LV_BER_BERNCASTLE.navsource = Data\Navigation\LV_BER_BERNCASTLE.navsource
		Data\Navigation\LV_DEV_TRAINING_GROUND.navgrid.json = Data\Navigation\LV_DEV_TRAINING_GROUND.navgrid.json
		Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.Character_Select.navpaint = Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.Character_Select.navpaint
		Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.Character_Select.navsource = Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.Character_Select.navsource
		Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navpaint = Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navpaint
		Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navregions = Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navregions
		Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navsource = Data\Navigation\LV_LOBBY_CLASSSELECT_SL00.navsource
		Data\Navigation\LV_LUT_HEARTRB_ED.navblockers = Data\Navigation\LV_LUT_HEARTRB_ED.navblockers
		Data\Navigation\LV_LUT_HEARTRB_ED.navpaint = Data\Navigation\LV_LUT_HEARTRB_ED.navpaint
		Data\Navigation\LV_LUT_HEARTRB_ED.navsource = Data\Navigation\LV_LUT_HEARTRB_ED.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.CardMiro.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.CardMiro.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.CardMiro.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.CardMiro.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate1Fine.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate1Fine.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate1Fine.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate1Fine.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate2Fine.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate2Fine.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate2Fine.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate2Fine.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate3Fine.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate3Fine.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate3Fine.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Gate3Fine.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario2.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario2.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario3.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario3.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario3.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario3.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario4.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario4.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario4.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.Mario4.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.navregions = Data\Navigation\LV_LUT_MIDNIGHTC_ED.navregions
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.navsource
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.StartFine.navpaint = Data\Navigation\LV_LUT_MIDNIGHTC_ED.StartFine.navpaint
		Data\Navigation\LV_LUT_MIDNIGHTC_ED.StartFine.navsource = Data\Navigation\LV_LUT_MIDNIGHTC_ED.StartFine.navsource
		Data\Navigation\LV_OCN_EVENTIS_MHP.navgrid.json = Data\Navigation\LV_OCN_EVENTIS_MHP.navgrid.json
		Data\Navigation\LV_OCN_EVENTIS_MHP.navregions = Data\Navigation\LV_OCN_EVENTIS_MHP.navregions
		Data\Navigation\LV_OCN_EVENTIS_MHP.WaterpangEntry.navsource = Data\Navigation\LV_OCN_EVENTIS_MHP.WaterpangEntry.navsource
		Data\Navigation\LV_PVP_COLOSSEUM.navsource = Data\Navigation\LV_PVP_COLOSSEUM.navsource
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Rendering", "Rendering", "{0374593F-01E3-5402-B4C6-1D0D300E967F}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Authored", "Authored", "{46E46DDC-B3F6-5758-8AB8-6EEDFDE17AE1}"
	ProjectSection(SolutionItems) = preProject
		Data\Rendering\Authored\CharacterSelectFloorSwap.json = Data\Rendering\Authored\CharacterSelectFloorSwap.json
		Data\Rendering\Authored\CharacterSelectShowcase.json = Data\Rendering\Authored\CharacterSelectShowcase.json
		Data\Rendering\Authored\LightResources.json = Data\Rendering\Authored\LightResources.json
		Data\Rendering\Authored\RenderingProfiles.json = Data\Rendering\Authored\RenderingProfiles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Reference", "Reference", "{CCE75567-A887-5918-80BF-E81B21290A61}"
	ProjectSection(SolutionItems) = preProject
		Data\Rendering\Reference\KoukuImportedSourceLights.maplights.json = Data\Rendering\Reference\KoukuImportedSourceLights.maplights.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "ResourceIntake", "ResourceIntake", "{707D5EEC-0341-512D-91D4-5C2FEA3939DC}"
	ProjectSection(SolutionItems) = preProject
		Data\ResourceIntake\LV_LUT_MIDNIGHTC_ED.resource-intake.json = Data\ResourceIntake\LV_LUT_MIDNIGHTC_ED.resource-intake.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Sound", "Sound", "{3FDD1641-829C-5CD4-957F-0772F3192BEE}"
	ProjectSection(SolutionItems) = preProject
		Data\Sound\CharacterSoundCatalog.json = Data\Sound\CharacterSoundCatalog.json
		Data\Sound\CharacterVoiceTypes.json = Data\Sound\CharacterVoiceTypes.json
		Data\Sound\EstherActionSoundCues.json = Data\Sound\EstherActionSoundCues.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Titles", "Titles", "{68793192-E907-5E44-8464-79D2774F7F2F}"
	ProjectSection(SolutionItems) = preProject
		Data\Titles\HonorTitles.json = Data\Titles\HonorTitles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "UI", "UI", "{41A68280-3C9F-519D-8B48-6953EEFD39DB}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "AvatarBook", "AvatarBook", "{A248DF72-580D-5937-99C1-89054650636E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\AvatarBook\AvatarBook_Layout.json = Data\UI\AvatarBook\AvatarBook_Layout.json
		Data\UI\AvatarBook\AvatarBookDisplay.json = Data\UI\AvatarBook\AvatarBookDisplay.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "BossUI", "BossUI", "{9B1B446F-C072-59E2-B81C-F3CEAD396C78}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\BossUI\BossUI.json = Data\UI\BossUI\BossUI.json
		Data\UI\BossUI\ImmuneGauge_Layout.json = Data\UI\BossUI\ImmuneGauge_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "CharacterInfo", "CharacterInfo", "{00363427-5A8B-54F9-9EDF-A1903ABFB60D}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\CharacterInfo\CharacterInfo_Layout.json = Data\UI\CharacterInfo\CharacterInfo_Layout.json
		Data\UI\CharacterInfo\CharacterInfoDisplay.json = Data\UI\CharacterInfo\CharacterInfoDisplay.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "CharacterSelect", "CharacterSelect", "{0DA0FDE1-72B3-59B7-B661-4DD728949EA5}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\CharacterSelect\CharacterSelectWindow_Layout.json = Data\UI\CharacterSelect\CharacterSelectWindow_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Chat", "Chat", "{267F8EEF-8BAC-52D9-9481-9CED79C4EEEA}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Chat\ChatWindow_Layout.json = Data\UI\Chat\ChatWindow_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "ClassSelect", "ClassSelect", "{EDDC3A4B-2707-5168-AEE5-5D25F1708C99}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\ClassSelect\ClassSelect_Layout.json = Data\UI\ClassSelect\ClassSelect_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Colosseum", "Colosseum", "{131EAEDD-B0BF-5077-B79D-45B086885F9B}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Colosseum\CombatHUD_Layout.json = Data\UI\Colosseum\CombatHUD_Layout.json
		Data\UI\Colosseum\IntroCutscene_Layout.json = Data\UI\Colosseum\IntroCutscene_Layout.json
		Data\UI\Colosseum\MatchCountdown_Layout.json = Data\UI\Colosseum\MatchCountdown_Layout.json
		Data\UI\Colosseum\MatchHUD_Layout.json = Data\UI\Colosseum\MatchHUD_Layout.json
		Data\UI\Colosseum\MatchLoading_Layout.json = Data\UI\Colosseum\MatchLoading_Layout.json
		Data\UI\Colosseum\QueueDialog_Layout.json = Data\UI\Colosseum\QueueDialog_Layout.json
		Data\UI\Colosseum\Result_Defeat.keyframes.json = Data\UI\Colosseum\Result_Defeat.keyframes.json
		Data\UI\Colosseum\Result_Draw.keyframes.json = Data\UI\Colosseum\Result_Draw.keyframes.json
		Data\UI\Colosseum\Result_Layout.json = Data\UI\Colosseum\Result_Layout.json
		Data\UI\Colosseum\Result_TitleTracks.json = Data\UI\Colosseum\Result_TitleTracks.json
		Data\UI\Colosseum\Result_Victory.keyframes.json = Data\UI\Colosseum\Result_Victory.keyframes.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "CombatAnalysis", "CombatAnalysis", "{1DC0E7F5-8ABC-51AB-B0BE-6AF1134C3DC7}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\CombatAnalysis\CombatAnalysis_Layout.json = Data\UI\CombatAnalysis\CombatAnalysis_Layout.json
		Data\UI\CombatAnalysis\CombatAnalysisUi.json = Data\UI\CombatAnalysis\CombatAnalysisUi.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Common", "Common", "{79623BD9-9C0F-58E1-836F-917421318E9D}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Common\QuickSlotDrag_Layout.json = Data\UI\Common\QuickSlotDrag_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Customizing", "Customizing", "{4017F1BC-FB1C-56C6-9355-87F5F38319A9}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Customizing\CustomizingColorModels.json = Data\UI\Customizing\CustomizingColorModels.json
		Data\UI\Customizing\CustomizingCostumes.json = Data\UI\Customizing\CustomizingCostumes.json
		Data\UI\Customizing\CustomizingFacePresets.json = Data\UI\Customizing\CustomizingFacePresets.json
		Data\UI\Customizing\CustomizingFaceTextures.json = Data\UI\Customizing\CustomizingFaceTextures.json
		Data\UI\Customizing\CustomizingHairstyles.json = Data\UI\Customizing\CustomizingHairstyles.json
		Data\UI\Customizing\CustomizingIcons.json = Data\UI\Customizing\CustomizingIcons.json
		Data\UI\Customizing\CustomizingMaterialVariables.json = Data\UI\Customizing\CustomizingMaterialVariables.json
		Data\UI\Customizing\CustomizingMeshTypes.json = Data\UI\Customizing\CustomizingMeshTypes.json
		Data\UI\Customizing\CustomizingUI.json = Data\UI\Customizing\CustomizingUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "DeadScene", "DeadScene", "{15D4FCA3-F25A-516A-8B36-B9B5976AC53F}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\DeadScene\DeadSceneUI.json = Data\UI\DeadScene\DeadSceneUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Durability", "Durability", "{CD8EDF90-DA6A-578B-8C17-8AE76DE0B275}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Durability\DurabilityUI.json = Data\UI\Durability\DurabilityUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Esther", "Esther", "{27D8D9C2-A065-5EC5-99FF-226BD15573F2}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Esther\EstherCutin.json = Data\UI\Esther\EstherCutin.json
		Data\UI\Esther\EstherUI.json = Data\UI\Esther\EstherUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "HeadStatus", "HeadStatus", "{70029C33-7C82-5AD2-A20D-3EE172062D00}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HeadStatus\ChatBubble_Layout.json = Data\UI\HeadStatus\ChatBubble_Layout.json
		Data\UI\HeadStatus\HealthBar_Layout.json = Data\UI\HeadStatus\HealthBar_Layout.json
		Data\UI\HeadStatus\ValtanArmorBreak_Layout.json = Data\UI\HeadStatus\ValtanArmorBreak_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "HonorTitle", "HonorTitle", "{D62628C6-5502-5E7A-A56B-7EACF5B69E7F}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HonorTitle\HonorTitle_Layout.json = Data\UI\HonorTitle\HonorTitle_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "HUD", "HUD", "{286C3B1A-B741-5046-8B4A-CA13A668EC40}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\GuardianKnightIdentity.json = Data\UI\HUD\GuardianKnightIdentity.json
		Data\UI\HUD\HUD_Layout.json = Data\UI\HUD\HUD_Layout.json
		Data\UI\HUD\HudBuffIcons.json = Data\UI\HUD\HudBuffIcons.json
		Data\UI\HUD\HudBuffSources.json = Data\UI\HUD\HudBuffSources.json
		Data\UI\HUD\SkillSlotMarks.json = Data\UI\HUD\SkillSlotMarks.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "IdentityAnimation", "IdentityAnimation", "{CB856184-4EB3-5711-9015-CABFAB4FB2CA}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Artist", "Artist", "{95AE86DB-1705-5BCE-A744-CFA6A2E655C7}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\IdentityAnimation\Artist\BrushIdle.json = Data\UI\HUD\IdentityAnimation\Artist\BrushIdle.json
		Data\UI\HUD\IdentityAnimation\Artist\BubbleUp.json = Data\UI\HUD\IdentityAnimation\Artist\BubbleUp.json
		Data\UI\HUD\IdentityAnimation\Artist\Gauge.json = Data\UI\HUD\IdentityAnimation\Artist\Gauge.json
		Data\UI\HUD\IdentityAnimation\Artist\GaugeSlot.json = Data\UI\HUD\IdentityAnimation\Artist\GaugeSlot.json
		Data\UI\HUD\IdentityAnimation\Artist\SkillXState.json = Data\UI\HUD\IdentityAnimation\Artist\SkillXState.json
		Data\UI\HUD\IdentityAnimation\Artist\SkillZState.json = Data\UI\HUD\IdentityAnimation\Artist\SkillZState.json
		Data\UI\HUD\IdentityAnimation\Artist\ZXWipe.json = Data\UI\HUD\IdentityAnimation\Artist\ZXWipe.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "GuardianKnight", "GuardianKnight", "{74AD9016-BB61-5B71-AFEF-814F6261737E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\IdentityAnimation\GuardianKnight\EmberSocket.json = Data\UI\HUD\IdentityAnimation\GuardianKnight\EmberSocket.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LanceMaster", "LanceMaster", "{E6545828-3CAC-5230-9C9F-99FC4918A57B}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0Burn.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0Burn.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0BurnLoop.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0BurnLoop.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0Fill.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge0Fill.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1Burn.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1Burn.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1BurnLoop.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1BurnLoop.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1Fill.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge1Fill.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2Burn.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2Burn.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2BurnLoop.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2BurnLoop.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2Fill.json = Data\UI\HUD\IdentityAnimation\LanceMaster\Gauge2Fill.json
		Data\UI\HUD\IdentityAnimation\LanceMaster\StanceMaster.json = Data\UI\HUD\IdentityAnimation\LanceMaster\StanceMaster.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "QuickSlot", "QuickSlot", "{DDED5BBA-E8D0-5B54-AEFC-B1869E3ECE7E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\IdentityAnimation\QuickSlot\UseSkillFlash.json = Data\UI\HUD\IdentityAnimation\QuickSlot\UseSkillFlash.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Warlord", "Warlord", "{E03C927C-F2D5-5069-B464-B7E911AC9A31}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\HUD\IdentityAnimation\Warlord\CooldownWipe.json = Data\UI\HUD\IdentityAnimation\Warlord\CooldownWipe.json
		Data\UI\HUD\IdentityAnimation\Warlord\DefenseBody.json = Data\UI\HUD\IdentityAnimation\Warlord\DefenseBody.json
		Data\UI\HUD\IdentityAnimation\Warlord\DefenseEffect.json = Data\UI\HUD\IdentityAnimation\Warlord\DefenseEffect.json
		Data\UI\HUD\IdentityAnimation\Warlord\DefenseModeEffect.json = Data\UI\HUD\IdentityAnimation\Warlord\DefenseModeEffect.json
		Data\UI\HUD\IdentityAnimation\Warlord\EnableRingX.json = Data\UI\HUD\IdentityAnimation\Warlord\EnableRingX.json
		Data\UI\HUD\IdentityAnimation\Warlord\EnableRingZ.json = Data\UI\HUD\IdentityAnimation\Warlord\EnableRingZ.json
		Data\UI\HUD\IdentityAnimation\Warlord\GaugeL.json = Data\UI\HUD\IdentityAnimation\Warlord\GaugeL.json
		Data\UI\HUD\IdentityAnimation\Warlord\GaugeR.json = Data\UI\HUD\IdentityAnimation\Warlord\GaugeR.json
		Data\UI\HUD\IdentityAnimation\Warlord\ProtectBody.json = Data\UI\HUD\IdentityAnimation\Warlord\ProtectBody.json
		Data\UI\HUD\IdentityAnimation\Warlord\ProtectEffect.json = Data\UI\HUD\IdentityAnimation\Warlord\ProtectEffect.json
		Data\UI\HUD\IdentityAnimation\Warlord\SkillXState.json = Data\UI\HUD\IdentityAnimation\Warlord\SkillXState.json
		Data\UI\HUD\IdentityAnimation\Warlord\SkillZState.json = Data\UI\HUD\IdentityAnimation\Warlord\SkillZState.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Interact", "Interact", "{FB17D171-AC09-567C-9D5D-925BDB7512F5}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Interact\InteractKey_Layout.json = Data\UI\Interact\InteractKey_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Inventory", "Inventory", "{A14F55F2-CA7F-5CEF-95DA-828FE4B9358E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Inventory\InventoryUI.json = Data\UI\Inventory\InventoryUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "ItemAnnounce", "ItemAnnounce", "{F690A480-8C92-5831-A099-52BB04BA0975}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\ItemAnnounce\ItemAnnounce_Layout.json = Data\UI\ItemAnnounce\ItemAnnounce_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "ItemUpgrade", "ItemUpgrade", "{FD5BA58A-0E1C-5284-8CDC-DF62284A1E6E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\ItemUpgrade\ItemUpgradeUI.json = Data\UI\ItemUpgrade\ItemUpgradeUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KakulFade", "KakulFade", "{E71F13F2-9412-5D1E-B7AE-6066EA21A84D}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\KakulFade\KakulFadeUI.json = Data\UI\KakulFade\KakulFadeUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "KoukuSaydon", "KoukuSaydon", "{0252730C-CB86-562A-92E8-4161A266AD14}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\KoukuSaydon\DungeonTimer_Layout.json = Data\UI\KoukuSaydon\DungeonTimer_Layout.json
		Data\UI\KoukuSaydon\KoukuHudModes.json = Data\UI\KoukuSaydon\KoukuHudModes.json
		Data\UI\KoukuSaydon\MadnessGauge_Layout.json = Data\UI\KoukuSaydon\MadnessGauge_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Loading", "Loading", "{C0F02528-1ACF-539F-AC04-57C42FF59E41}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Loading\LoadingLayout.json = Data\UI\Loading\LoadingLayout.json
		Data\UI\Loading\LoadingRecovery.json = Data\UI\Loading\LoadingRecovery.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Lobby", "Lobby", "{7A65FAB6-8D6E-573D-AA54-BAB2A803A9BC}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Lobby\Lobby_Layout.json = Data\UI\Lobby\Lobby_Layout.json
		Data\UI\Lobby\LobbyServers.json = Data\UI\Lobby\LobbyServers.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Minimap", "Minimap", "{2FC666B6-8E9D-5DC4-B88B-271278F89F64}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Minimap\Minimap_Layout.json = Data\UI\Minimap\Minimap_Layout.json
		Data\UI\Minimap\MinimapAreas.json = Data\UI\Minimap\MinimapAreas.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "MVP", "MVP", "{07039AE2-1A17-5AFD-8C6E-ED0FA860166F}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\MVP\MvpAwards.json = Data\UI\MVP\MvpAwards.json
		Data\UI\MVP\MvpClassSymbols.json = Data\UI\MVP\MvpClassSymbols.json
		Data\UI\MVP\MvpContentNames.json = Data\UI\MVP\MvpContentNames.json
		Data\UI\MVP\MvpResult_BadgeEffect.keyframes.json = Data\UI\MVP\MvpResult_BadgeEffect.keyframes.json
		Data\UI\MVP\MvpResult_Intro.keyframes.json = Data\UI\MVP\MvpResult_Intro.keyframes.json
		Data\UI\MVP\MvpResult_Intro_Above.keyframes.json = Data\UI\MVP\MvpResult_Intro_Above.keyframes.json
		Data\UI\MVP\MvpResult_Intro_Below.keyframes.json = Data\UI\MVP\MvpResult_Intro_Below.keyframes.json
		Data\UI\MVP\MvpResult_Intro_Mid.keyframes.json = Data\UI\MVP\MvpResult_Intro_Mid.keyframes.json
		Data\UI\MVP\MvpResult_Layout.json = Data\UI\MVP\MvpResult_Layout.json
		Data\UI\MVP\MvpResult_StageReveal.json = Data\UI\MVP\MvpResult_StageReveal.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Party", "Party", "{C2490479-F478-5578-A73B-8398E3EE4E2C}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Party\PartyContextMenu_Layout.json = Data\UI\Party\PartyContextMenu_Layout.json
		Data\UI\Party\PartyInviteConfirm_Layout.json = Data\UI\Party\PartyInviteConfirm_Layout.json
		Data\UI\Party\PartyWindow_Layout.json = Data\UI\Party\PartyWindow_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "RaidClear", "RaidClear", "{A2D57939-EF28-5862-A8DB-3185673314F3}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\RaidClear\RaidClear_Kouku.keyframes.json = Data\UI\RaidClear\RaidClear_Kouku.keyframes.json
		Data\UI\RaidClear\RaidClear_Kouku_Layout.json = Data\UI\RaidClear\RaidClear_Kouku_Layout.json
		Data\UI\RaidClear\RaidClear_Layout.json = Data\UI\RaidClear\RaidClear_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "RaidEntry", "RaidEntry", "{F3570F91-9E28-5770-819E-21C2E0B1563D}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\RaidEntry\BernValtanEntry_Layout.json = Data\UI\RaidEntry\BernValtanEntry_Layout.json
		Data\UI\RaidEntry\ValtanRaidEntry_Layout.json = Data\UI\RaidEntry\ValtanRaidEntry_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "RaidGateProgress", "RaidGateProgress", "{0F2AAE7F-737C-5D3A-BD4E-994CFAB57938}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\RaidGateProgress\RaidGateProgress_Layout.json = Data\UI\RaidGateProgress\RaidGateProgress_Layout.json
		Data\UI\RaidGateProgress\RaidGateVote_Layout.json = Data\UI\RaidGateProgress\RaidGateVote_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Repair", "Repair", "{7D038F0E-1A2C-5936-B9EB-333A5D12C55E}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Repair\RepairUI.json = Data\UI\Repair\RepairUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "ScreenUI", "ScreenUI", "{7A85E813-D45D-50A0-9ED5-ADB1F3DC4786}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\ScreenUI\ScreenUI.json = Data\UI\ScreenUI\ScreenUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Shop", "Shop", "{47E98013-028F-5640-8992-A64AC004FC3D}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Shop\ShopUI.json = Data\UI\Shop\ShopUI.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "SkillWindow", "SkillWindow", "{914EB6E8-AF7F-57DC-B3E0-347067FF53C7}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\SkillWindow\LanceMaster.skillroster.json = Data\UI\SkillWindow\LanceMaster.skillroster.json
		Data\UI\SkillWindow\SkillWindow_Layout.json = Data\UI\SkillWindow\SkillWindow_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "SystemMenu", "SystemMenu", "{2FB60A2D-7C6B-55A0-B724-7455BEE79E45}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\SystemMenu\SystemMenuButtons_Layout.json = Data\UI\SystemMenu\SystemMenuButtons_Layout.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "SystemOption", "SystemOption", "{41CDA9A1-3BB8-52F0-886F-AC08095295CC}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\SystemOption\SystemOption_Layout.json = Data\UI\SystemOption\SystemOption_Layout.json
		Data\UI\SystemOption\SystemOptionRows.json = Data\UI\SystemOption\SystemOptionRows.json
		Data\UI\SystemOption\SystemOptionStrings.json = Data\UI\SystemOption\SystemOptionStrings.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Vehicle", "Vehicle", "{9B9E6FBC-E9C2-5E1E-8D70-826BD6B4AF28}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\Vehicle\Vehicle_Layout.json = Data\UI\Vehicle\Vehicle_Layout.json
		Data\UI\Vehicle\VehicleUiCatalog.json = Data\UI\Vehicle\VehicleUiCatalog.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "WorldMap", "WorldMap", "{9CC8187F-6D24-5BBD-9526-99DEE5C6B6BE}"
	ProjectSection(SolutionItems) = preProject
		Data\UI\WorldMap\SongCastGauge_Layout.json = Data\UI\WorldMap\SongCastGauge_Layout.json
		Data\UI\WorldMap\WorldMap_Layout.json = Data\UI\WorldMap\WorldMap_Layout.json
		Data\UI\WorldMap\WorldMapLabels.json = Data\UI\WorldMap\WorldMapLabels.json
		Data\UI\WorldMap\WorldMapNpcSymbols.json = Data\UI\WorldMap\WorldMapNpcSymbols.json
		Data\UI\WorldMap\WorldMapPanels.json = Data\UI\WorldMap\WorldMapPanels.json
		Data\UI\WorldMap\WorldMapSquareHoles.json = Data\UI\WorldMap\WorldMapSquareHoles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Valtan", "Valtan", "{C8CCD376-841E-5747-AA6D-D549BC9E1721}"
	ProjectSection(SolutionItems) = preProject
		Data\Valtan\Valtan.animation-chain-promotion.receipt.json = Data\Valtan\Valtan.animation-chain-promotion.receipt.json
		Data\Valtan\Valtan.animation-chain-promotions.json = Data\Valtan\Valtan.animation-chain-promotions.json
		Data\Valtan\Valtan.clearrewards.json = Data\Valtan\Valtan.clearrewards.json
		Data\Valtan\Valtan.cliptemplates.json = Data\Valtan\Valtan.cliptemplates.json
		Data\Valtan\Valtan.combatobjects.json = Data\Valtan\Valtan.combatobjects.json
		Data\Valtan\Valtan.gameplay.json = Data\Valtan\Valtan.gameplay.json
		Data\Valtan\Valtan.hitalignment-allowlist.json = Data\Valtan\Valtan.hitalignment-allowlist.json
		Data\Valtan\Valtan.legacy-compatibility.json = Data\Valtan\Valtan.legacy-compatibility.json
		Data\Valtan\Valtan.pattern.json = Data\Valtan\Valtan.pattern.json
		Data\Valtan\Valtan.presentation.debug.json = Data\Valtan\Valtan.presentation.debug.json
		Data\Valtan\Valtan.presentation.json = Data\Valtan\Valtan.presentation.json
		Data\Valtan\Valtan.worldeventsets.json = Data\Valtan\Valtan.worldeventsets.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Published", "Published", "{C1440991-5114-5CE4-9E32-E70C6F0F4635}"
	ProjectSection(SolutionItems) = preProject
		Data\Valtan\Published\BOSS_VALTAN.effectv2bindings.json = Data\Valtan\Published\BOSS_VALTAN.effectv2bindings.json
		Data\Valtan\Published\Valtan.patternsoundcues.json = Data\Valtan\Published\Valtan.patternsoundcues.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Vehicles", "Vehicles", "{95FD4F37-6850-57A3-9072-7447FF86A402}"
	ProjectSection(SolutionItems) = preProject
		Data\Vehicles\VehicleProfiles.json = Data\Vehicles\VehicleProfiles.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Worlds", "Worlds", "{A0D7C11C-8722-551B-AFE5-E67127789383}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_BER_BERNCASTLE", "LV_BER_BERNCASTLE", "{D54F0AA2-5306-5961-B730-D4E7439B0E71}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_BER_BERNCASTLE\bern19.source-coordinates.json = Data\Worlds\LV_BER_BERNCASTLE\bern19.source-coordinates.json
		Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json = Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_DEV_TRAINING_GROUND", "LV_DEV_TRAINING_GROUND", "{65A9A92A-76ED-5E3D-BBB6-67F187BBADD6}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json = Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LOBBY_CLASSSELECT_SL00", "LV_LOBBY_CLASSSELECT_SL00", "{425AABC5-8723-5A1C-B5D1-3DBBC6370D6D}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\Gameplay.world.json = Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\Gameplay.world.json
		Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\SpawnGroups.world.json = Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\SpawnGroups.world.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_HEARTRB_ED", "LV_LUT_HEARTRB_ED", "{F7852B54-0B4C-5BA8-B44D-692409063175}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_LUT_HEARTRB_ED\EncounterProps.world.json = Data\Worlds\LV_LUT_HEARTRB_ED\EncounterProps.world.json
		Data\Worlds\LV_LUT_HEARTRB_ED\Gameplay.world.json = Data\Worlds\LV_LUT_HEARTRB_ED\Gameplay.world.json
		Data\Worlds\LV_LUT_HEARTRB_ED\SpawnGroups.world.json = Data\Worlds\LV_LUT_HEARTRB_ED\SpawnGroups.world.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_LUT_MIDNIGHTC_ED", "LV_LUT_MIDNIGHTC_ED", "{AA67F307-3827-5A3E-96EB-7AFCCA0C6122}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_LUT_MIDNIGHTC_ED\Gameplay.world.json = Data\Worlds\LV_LUT_MIDNIGHTC_ED\Gameplay.world.json
		Data\Worlds\LV_LUT_MIDNIGHTC_ED\SpawnGroups.world.json = Data\Worlds\LV_LUT_MIDNIGHTC_ED\SpawnGroups.world.json
		Data\Worlds\LV_LUT_MIDNIGHTC_ED\StageMarkers.json = Data\Worlds\LV_LUT_MIDNIGHTC_ED\StageMarkers.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_OCN_EVENTIS_MHP", "LV_OCN_EVENTIS_MHP", "{5F67A4E2-F57B-50DD-BDD9-D4D4658B7D86}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_OCN_EVENTIS_MHP\Gameplay.world.json = Data\Worlds\LV_OCN_EVENTIS_MHP\Gameplay.world.json
	EndProjectSection
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "LV_PVP_COLOSSEUM", "LV_PVP_COLOSSEUM", "{6549391F-1013-5AB8-9260-7C9E42353B44}"
	ProjectSection(SolutionItems) = preProject
		Data\Worlds\LV_PVP_COLOSSEUM\Gameplay.world.json = Data\Worlds\LV_PVP_COLOSSEUM\Gameplay.world.json
	EndProjectSection
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Debug|x86 = Debug|x86
		Release|x64 = Release|x64
		Release|x86 = Release|x86
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x64.ActiveCfg = Debug|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x64.Build.0 = Debug|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x86.ActiveCfg = Debug|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x86.Build.0 = Debug|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x64.ActiveCfg = Release|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x64.Build.0 = Release|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x86.ActiveCfg = Release|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x86.Build.0 = Release|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x64.ActiveCfg = Debug|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x64.Build.0 = Debug|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x86.ActiveCfg = Debug|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x86.Build.0 = Debug|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x64.ActiveCfg = Release|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x64.Build.0 = Release|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x86.ActiveCfg = Release|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x86.Build.0 = Release|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x64.ActiveCfg = Debug|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x64.Build.0 = Debug|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x86.ActiveCfg = Debug|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x86.Build.0 = Debug|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x64.ActiveCfg = Release|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x64.Build.0 = Release|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x86.ActiveCfg = Release|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x86.Build.0 = Release|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x64.ActiveCfg = Debug|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x86.ActiveCfg = Debug|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x64.ActiveCfg = Release|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x86.ActiveCfg = Release|Win32
		{D9E5A74B-70B1-41DF-8871-118120CBC7CD}.Debug|x64.ActiveCfg = Debug|x64
		{D9E5A74B-70B1-41DF-8871-118120CBC7CD}.Debug|x86.ActiveCfg = Debug|Win32
		{D9E5A74B-70B1-41DF-8871-118120CBC7CD}.Release|x64.ActiveCfg = Release|x64
		{D9E5A74B-70B1-41DF-8871-118120CBC7CD}.Release|x86.ActiveCfg = Release|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x64.ActiveCfg = Debug|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x64.Build.0 = Debug|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x86.ActiveCfg = Debug|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x86.Build.0 = Debug|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x64.ActiveCfg = Release|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x64.Build.0 = Release|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x86.ActiveCfg = Release|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x86.Build.0 = Release|Win32
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Debug|x64.ActiveCfg = Debug|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Debug|x86.ActiveCfg = Debug|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Release|x64.ActiveCfg = Release|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Release|x86.ActiveCfg = Release|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Debug|x64.ActiveCfg = Debug|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Debug|x86.ActiveCfg = Debug|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Release|x64.ActiveCfg = Release|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Release|x86.ActiveCfg = Release|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Debug|x64.ActiveCfg = Debug|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Debug|x86.ActiveCfg = Debug|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Release|x64.ActiveCfg = Release|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Release|x86.ActiveCfg = Release|x64
		{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}.Debug|x64.ActiveCfg = Debug|x64
		{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}.Debug|x86.ActiveCfg = Debug|x64
		{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}.Release|x64.ActiveCfg = Release|x64
		{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}.Release|x86.ActiveCfg = Release|x64
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {D406B777-1065-4D58-B716-7FC29A94B7C2}
	EndGlobalSection
	GlobalSection(NestedProjects) = preSolution
		{485FA94D-8E70-5F3F-8221-DDCD06A724DC} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{1BB87F58-65F6-568F-9191-AF20466FFE4A} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{029209C1-5EC5-55C5-B168-71F24E43D964} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{7B5F7812-C61A-5748-886D-DE465DDD2C87} = {029209C1-5EC5-55C5-B168-71F24E43D964}
		{3510878C-9EE6-5921-8C3E-412CC687F2FC} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{F2E070DE-E1E8-5763-97F2-324A6887A25C} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{E1444538-B294-5CA1-BD6C-0876C599EC31} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{672FAC17-1563-5D99-9113-D875472EF00E} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{E43A8D01-3CF6-5D80-917A-5E57F7620B2C} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{C6586D75-3D30-5265-9E12-EBDDE052F569} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{484244C8-9459-58F3-83CC-016BA027239F} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{26886C0C-025B-54D9-85B6-591B251C85F6} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{2DAB4577-F73D-5A87-AE1D-B5F96BCF9CCF} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{27581604-0006-54AD-8451-23924C4866B8} = {7B5F7812-C61A-5748-886D-DE465DDD2C87}
		{3B2E1995-9C50-55D2-B28D-078CE382FA77} = {029209C1-5EC5-55C5-B168-71F24E43D964}
		{9F1E4AEF-9194-506F-B1B9-67733D2628B9} = {029209C1-5EC5-55C5-B168-71F24E43D964}
		{E3D17799-341C-58B1-AD9A-12131A5C70A1} = {029209C1-5EC5-55C5-B168-71F24E43D964}
		{4B8552BE-4F68-504B-9B48-05DFEF3BDA37} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{CF8105F5-B159-5425-A3E5-DC6C5CE420F9} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{F86E4F27-13C8-57FE-A818-A44FC30E00B7} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{B9AA23DB-3A62-5018-914E-1553E3682B54} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{5032393B-C7F9-5F0A-A986-126A5A6B52AD} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{736C79C4-3DE8-5052-8293-8DCEDA91D403} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{53E67D93-5977-53A3-B92C-D1220E6D86CB} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{29C7BA96-5088-53C3-AEA0-18DF71436FE6} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{4D55ABB7-6016-50AB-BDC3-83751F679519} = {E3D17799-341C-58B1-AD9A-12131A5C70A1}
		{532DCCBC-1D56-54C9-9BA5-84980169EC9A} = {029209C1-5EC5-55C5-B168-71F24E43D964}
		{04A3B383-33FC-5B88-B264-2205B896651B} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{4CC1AC32-F0E5-56B5-A3DE-3306AABE3301} = {04A3B383-33FC-5B88-B264-2205B896651B}
		{2CB9D6B9-80F6-5F54-9170-EFECAC1EEAE2} = {04A3B383-33FC-5B88-B264-2205B896651B}
		{C1EB8271-A950-5494-9139-8B5122774DB7} = {2CB9D6B9-80F6-5F54-9170-EFECAC1EEAE2}
		{F7E333D9-9033-5E32-B8C3-332A19F21ADE} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{43DF313D-CACC-5B6E-936D-09C870B6DE8C} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{E9B9715E-E544-5424-AB66-A68B8503C3C5} = {43DF313D-CACC-5B6E-936D-09C870B6DE8C}
		{FD63E535-93AF-5856-B06C-AD1E7C602A91} = {43DF313D-CACC-5B6E-936D-09C870B6DE8C}
		{B7442E27-10F7-5B9E-9E4B-22EB6E2B06E9} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{83B3A406-9FEE-5264-8F2B-D2F84FCC9527} = {B7442E27-10F7-5B9E-9E4B-22EB6E2B06E9}
		{59E6F36E-AA3E-5AB6-91AA-389AA6955725} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{802F9714-C0A7-5172-AC1E-F20BCC23B277} = {59E6F36E-AA3E-5AB6-91AA-389AA6955725}
		{2F4E9DDB-AE86-54FF-8A3C-F11E9864746D} = {59E6F36E-AA3E-5AB6-91AA-389AA6955725}
		{E1822C40-7D67-5B03-BF33-1ACFCDBD77A0} = {59E6F36E-AA3E-5AB6-91AA-389AA6955725}
		{A05499D1-EA4F-54BB-B875-2E0F71BF24FF} = {59E6F36E-AA3E-5AB6-91AA-389AA6955725}
		{E62C7DD5-06BD-5F03-9D85-4EA87526161F} = {A05499D1-EA4F-54BB-B875-2E0F71BF24FF}
		{677B4893-0CF4-58C2-8EC4-D3DEB6FAD4AC} = {A05499D1-EA4F-54BB-B875-2E0F71BF24FF}
		{0752A33C-3676-5057-8C6B-5F5CA03EBECA} = {A05499D1-EA4F-54BB-B875-2E0F71BF24FF}
		{CE7700BF-E4A3-585A-9FF9-E2AAAE2BD70F} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{67846088-7213-5D35-BCCA-72D88B4553F3} = {CE7700BF-E4A3-585A-9FF9-E2AAAE2BD70F}
		{73677C61-11D9-505E-85BF-1C9C443B7670} = {CE7700BF-E4A3-585A-9FF9-E2AAAE2BD70F}
		{E7D5C6D7-06F2-551E-BAD2-B76C71CF0641} = {CE7700BF-E4A3-585A-9FF9-E2AAAE2BD70F}
		{8616D941-E1BE-5A3A-8392-5C7CA7632B1B} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{F53DE104-44C5-533C-AE7B-2E6A938BBB82} = {8616D941-E1BE-5A3A-8392-5C7CA7632B1B}
		{66A58081-1AF6-57DA-9EF9-DF55853A4E32} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{32A17A30-215B-56ED-A0FF-07DFFA1ACAA0} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{D48AC68E-D619-523E-BF1F-727C1C613041} = {32A17A30-215B-56ED-A0FF-07DFFA1ACAA0}
		{F8B345CD-1FBC-5517-9E73-4CC518772D2B} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{FAAF6643-F55B-511F-98FA-7B296A82AA0D} = {F8B345CD-1FBC-5517-9E73-4CC518772D2B}
		{2D8B9D4C-9DC3-5716-814F-9AA192A5B77F} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{06B041F1-05E2-576A-898A-5C8F087002B3} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{C92DFC48-427A-5DB0-87EA-3341CD30DD62} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{75864815-DDCF-55D2-BBBA-E847445AF2BD} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{97C1370E-3C66-5D77-9F7D-A2A729BD8CC5} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{DFE007B8-2407-56BA-AF4C-19CE0D50448A} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{91207229-C6D6-50E2-8C7D-67CBFAB046FD} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{9C2CEA37-7855-53B4-8452-109FA427E302} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{30572973-77B5-5233-866B-7638F9FFFCF4} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{B580BE8A-5437-51AB-857B-9E62D2E5B103} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{C2B8D718-DFEA-5AB0-A1B7-13B3210761F8} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{14E91526-66BC-5C12-8BAC-9EE70F0D5CC3} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{6DE08018-AB4A-59CA-A15D-8E597050F1F3} = {FAAF6643-F55B-511F-98FA-7B296A82AA0D}
		{411FA728-6882-57C9-AF94-0394AF7C7FED} = {F8B345CD-1FBC-5517-9E73-4CC518772D2B}
		{328877D6-992D-5FDA-8F16-FB2967107F54} = {F8B345CD-1FBC-5517-9E73-4CC518772D2B}
		{F82E31A2-CC09-564E-BBF8-DA4DD2AF1471} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{5750AF10-1DED-57C9-A253-C6B4C0500F6D} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{A3F0547E-06F3-55CE-B9C5-B4CC74D17837} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{1B19FB48-6946-5532-944B-2D5F54C759DE} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{FE67B544-6FDC-5184-A46F-853A94BCCB11} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{7CED3764-384A-50FD-9FBA-71AAB73A2C6A} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{9793008E-9895-57B7-B97B-E314D28B4E39} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{EEC32F58-11FF-55AE-B36E-7C1C68FF4574} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{A4E3D7DA-6EEC-5F80-9A2D-B63155EEBD33} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{15A1E9EA-9BC4-513A-89A6-894545E731D5} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{4E0CE829-4EAC-5261-B429-8A752A45548B} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{EB97F70B-7F87-5488-8125-A2D304B78545} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{2545BAEB-F7FE-5C33-9F8D-67A1DA14CD98} = {328877D6-992D-5FDA-8F16-FB2967107F54}
		{6D8282A6-EBEB-5275-905F-7C3D212FD1D7} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{0374593F-01E3-5402-B4C6-1D0D300E967F} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{46E46DDC-B3F6-5758-8AB8-6EEDFDE17AE1} = {0374593F-01E3-5402-B4C6-1D0D300E967F}
		{CCE75567-A887-5918-80BF-E81B21290A61} = {0374593F-01E3-5402-B4C6-1D0D300E967F}
		{707D5EEC-0341-512D-91D4-5C2FEA3939DC} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{3FDD1641-829C-5CD4-957F-0772F3192BEE} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{68793192-E907-5E44-8464-79D2774F7F2F} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{41A68280-3C9F-519D-8B48-6953EEFD39DB} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{A248DF72-580D-5937-99C1-89054650636E} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{9B1B446F-C072-59E2-B81C-F3CEAD396C78} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{00363427-5A8B-54F9-9EDF-A1903ABFB60D} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{0DA0FDE1-72B3-59B7-B661-4DD728949EA5} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{267F8EEF-8BAC-52D9-9481-9CED79C4EEEA} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{EDDC3A4B-2707-5168-AEE5-5D25F1708C99} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{131EAEDD-B0BF-5077-B79D-45B086885F9B} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{1DC0E7F5-8ABC-51AB-B0BE-6AF1134C3DC7} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{79623BD9-9C0F-58E1-836F-917421318E9D} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{4017F1BC-FB1C-56C6-9355-87F5F38319A9} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{15D4FCA3-F25A-516A-8B36-B9B5976AC53F} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{CD8EDF90-DA6A-578B-8C17-8AE76DE0B275} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{27D8D9C2-A065-5EC5-99FF-226BD15573F2} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{70029C33-7C82-5AD2-A20D-3EE172062D00} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{D62628C6-5502-5E7A-A56B-7EACF5B69E7F} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{286C3B1A-B741-5046-8B4A-CA13A668EC40} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{CB856184-4EB3-5711-9015-CABFAB4FB2CA} = {286C3B1A-B741-5046-8B4A-CA13A668EC40}
		{95AE86DB-1705-5BCE-A744-CFA6A2E655C7} = {CB856184-4EB3-5711-9015-CABFAB4FB2CA}
		{74AD9016-BB61-5B71-AFEF-814F6261737E} = {CB856184-4EB3-5711-9015-CABFAB4FB2CA}
		{E6545828-3CAC-5230-9C9F-99FC4918A57B} = {CB856184-4EB3-5711-9015-CABFAB4FB2CA}
		{DDED5BBA-E8D0-5B54-AEFC-B1869E3ECE7E} = {CB856184-4EB3-5711-9015-CABFAB4FB2CA}
		{E03C927C-F2D5-5069-B464-B7E911AC9A31} = {CB856184-4EB3-5711-9015-CABFAB4FB2CA}
		{FB17D171-AC09-567C-9D5D-925BDB7512F5} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{A14F55F2-CA7F-5CEF-95DA-828FE4B9358E} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{F690A480-8C92-5831-A099-52BB04BA0975} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{FD5BA58A-0E1C-5284-8CDC-DF62284A1E6E} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{E71F13F2-9412-5D1E-B7AE-6066EA21A84D} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{0252730C-CB86-562A-92E8-4161A266AD14} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{C0F02528-1ACF-539F-AC04-57C42FF59E41} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{7A65FAB6-8D6E-573D-AA54-BAB2A803A9BC} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{2FC666B6-8E9D-5DC4-B88B-271278F89F64} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{07039AE2-1A17-5AFD-8C6E-ED0FA860166F} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{C2490479-F478-5578-A73B-8398E3EE4E2C} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{A2D57939-EF28-5862-A8DB-3185673314F3} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{F3570F91-9E28-5770-819E-21C2E0B1563D} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{0F2AAE7F-737C-5D3A-BD4E-994CFAB57938} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{7D038F0E-1A2C-5936-B9EB-333A5D12C55E} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{7A85E813-D45D-50A0-9ED5-ADB1F3DC4786} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{47E98013-028F-5640-8992-A64AC004FC3D} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{914EB6E8-AF7F-57DC-B3E0-347067FF53C7} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{2FB60A2D-7C6B-55A0-B724-7455BEE79E45} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{41CDA9A1-3BB8-52F0-886F-AC08095295CC} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{9B9E6FBC-E9C2-5E1E-8D70-826BD6B4AF28} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{9CC8187F-6D24-5BBD-9526-99DEE5C6B6BE} = {41A68280-3C9F-519D-8B48-6953EEFD39DB}
		{C8CCD376-841E-5747-AA6D-D549BC9E1721} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{C1440991-5114-5CE4-9E32-E70C6F0F4635} = {C8CCD376-841E-5747-AA6D-D549BC9E1721}
		{95FD4F37-6850-57A3-9072-7447FF86A402} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{A0D7C11C-8722-551B-AFE5-E67127789383} = {5FD83BF2-4FC1-50CF-98CA-6006F7B68A77}
		{D54F0AA2-5306-5961-B730-D4E7439B0E71} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{65A9A92A-76ED-5E3D-BBB6-67F187BBADD6} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{425AABC5-8723-5A1C-B5D1-3DBBC6370D6D} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{F7852B54-0B4C-5BA8-B44D-692409063175} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{AA67F307-3827-5A3E-96EB-7AFCCA0C6122} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{5F67A4E2-F57B-50DD-BDD9-D4D4658B7D86} = {A0D7C11C-8722-551B-AFE5-E67127789383}
		{6549391F-1013-5AB8-9260-7C9E42353B44} = {A0D7C11C-8722-551B-AFE5-E67127789383}
	EndGlobalSection
EndGlobal

```

## Client/Default/Client.vcxproj 완전 교체본

변경 종류: 기존 파일 전체 교체. 위치: 파일의 처음부터 마지막 줄까지. 무관한 기존 내용은 보존한다.

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <ItemGroup>
	<ClInclude Include="..\Public\ActionPresentationTimeline.h" />
	<ClInclude Include="..\Public\CameraShakeService.h" />
	<ClInclude Include="..\Public\ValtanCinematicCameraController.h" />
	<ClInclude Include="..\Public\ValtanCinematicCameraDocument.h" />
	<ClInclude Include="..\Public\WorldDestructionProjectionDocument.h" />
	<ClInclude Include="..\Public\WorldDestructionDebrisPresentationDocument.h" />
	<ClInclude Include="..\Public\WorldDestructionDebrisPresentationRuntime.h" />
	<ClInclude Include="..\Public\WorldDestructionProjectionRuntime.h" />
    <ClInclude Include="..\Public\AnimationPreviewAssets.h" />
    <ClInclude Include="..\Public\AnimationAuthoringBridge.h" />
    <ClInclude Include="..\Public\AnimationSkillBindingDocument.h" />
    <ClInclude Include="..\Public\CharacterActionWorkbench.h" />
    <ClInclude Include="..\Public\BoneAnimationDocument.h" />
    <ClInclude Include="..\Public\BoneAnimationWorkbench.h" />
    <ClInclude Include="..\Public\CharacterModelWorkbench.h" />
    <ClInclude Include="..\Public\CharacterActionCombatDocument.h" />
	<ClInclude Include="..\Public\KoukuSaydonAnimationActionDocument.h" />
	<ClInclude Include="..\Public\KoukuSaydonAnimationPatternDocument.h" />
	<ClInclude Include="..\Public\KoukuSaydonCompositionDocument.h" />
	<ClInclude Include="..\Public\KoukuSaydonActionWorkbench.h" />
	<ClInclude Include="..\Public\KoukuSaydonBossTool.h" />
	<ClInclude Include="..\Public\KoukuSaydonPatternAuditionService.h" />
	<ClInclude Include="..\Public\ValtanPatternFlowDocument.h" />
	<ClInclude Include="..\Public\ValtanPatternFlowService.h" />
	<ClInclude Include="..\Public\ValtanTuningCommandService.h" />
	<ClInclude Include="..\Public\ValtanPatternPreviewDocument.h" />
	<ClInclude Include="..\Public\ValtanPatternAuthoringEffectDocument.h" />
	<ClInclude Include="..\Public\ValtanCombatObjectSoundCueDocument.h" />
	<ClInclude Include="..\Public\ValtanPatternEffectCueDocument.h" />
	<ClInclude Include="..\Public\ValtanPatternEffectCueAuthoring.h" />
    <ClInclude Include="..\Public\ValtanPatternShakeCueDocument.h" />
    <ClInclude Include="..\Public\ValtanPatternSoundCueDocument.h" />
    <ClInclude Include="..\Public\BoneChainSimulation.h" />
    <ClInclude Include="..\Public\FaceCustomizeApplier.h" />
    <ClInclude Include="..\Public\FaceMorphApplier.h" />
    <ClInclude Include="..\Public\FaceSliderDocument.h" />
    <ClInclude Include="..\Public\BalanceTool.h" />
    <ClInclude Include="..\Public\BalanceTestPanel.h" />
      <ClInclude Include="..\Public\ValtanBossTool.h" />
	  <ClInclude Include="..\Public\CameraTool.h" />
	<ClInclude Include="..\Public\ActionCompositionGraphModel.h" />
	<ClInclude Include="..\Public\BossLogicFlowView.h" />
	<ClInclude Include="..\Public\ValtanActionWorkbench.h" />
	<ClInclude Include="..\Public\CompositionResourceTree.h" />
	<ClInclude Include="..\Public\CompositionAnimationResource.h" />
	<ClInclude Include="..\Public\CompositionWorkbenchSession.h" />
	<ClInclude Include="..\Public\CompositionEditing.h" />
	<ClInclude Include="..\Public\CompositionTimeline.h" />
    <ClInclude Include="..\Public\Animation_Tool.h" />
    <ClInclude Include="..\Public\Character.h" />
    <ClInclude Include="..\Public\LocalMovePrediction.h" />
    <ClInclude Include="..\Public\HitAreaWire.h" />
    <ClInclude Include="..\Public\SkeletalAfterimage.h" />
    <ClInclude Include="..\Public\CharacterCatalog.h" />
    <ClInclude Include="..\Public\ActorCatalog.h" />
    <ClInclude Include="..\Public\CharacterSelectionState.h" />
    <ClInclude Include="..\Public\CharacterRoster.h" />
    <ClInclude Include="..\Public\CharacterSelectWindowView.h" />
    <ClInclude Include="..\Public\MinimapView.h" />
    <ClInclude Include="..\Public\KoukuMadnessGaugeView.h" />
    <ClInclude Include="..\Public\StatusEffectTextView.h" />
    <ClInclude Include="..\Public\DungeonTimerView.h" />
    <ClInclude Include="..\Public\MvpAwardCatalog.h" />
    <ClInclude Include="..\Public\MvpResultView.h" />
    <ClInclude Include="..\Public\RaidGateProgressView.h" />
    <ClInclude Include="..\Public\SystemMenuButtonsView.h" />
    <ClInclude Include="..\Public\InteractKeyPromptView.h" />
    <ClInclude Include="..\Public\UITextOcclusion.h" />
    <ClInclude Include="..\Public\BossImmuneGaugeView.h" />
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h" />
    <ClInclude Include="..\Public\Effect_CascadeCompiler.h" />
	<ClInclude Include="..\Public\Effect_RuntimeAuthority.h" />
	<ClInclude Include="..\Public\Effect_Artist31470ShaderRegistry.h" />
	<ClInclude Include="..\Public\Effect_ReconstructedExecution.h" />
    <ClInclude Include="..\Public\Generated\Effect_SourceContractRegistry.generated.h" />
    <ClInclude Include="..\Public\Effect_ComponentDocument.h" />
    <ClInclude Include="..\Public\Effect_Distribution.h" />
    <ClInclude Include="..\Public\Effect_MaterialTemplate.h" />
    <ClInclude Include="..\Public\Effect_DimensionMasterQMaterial.h" />
    <ClInclude Include="..\Public\Effect_DimensionMasterALTVMaterial.h" />
    <ClInclude Include="..\Public\Effect_DimensionMasterVMaterial.h" />
    <ClInclude Include="..\Public\EffectAuthoringTransfer.h" />
    <ClInclude Include="..\Public\Effect_DocumentCodec.h" />
    <ClInclude Include="..\Public\Effect_Playback.h" />
    <ClInclude Include="..\Public\Effect_ParticleUpdatePool.h" />
    <ClInclude Include="..\Public\Effect_LightPresentation.h" />
    <ClInclude Include="..\Public\Effect_DocumentRenderer.h" />
    <ClInclude Include="..\Public\Effect_MaterialProgramRegistry.h" />
    <ClInclude Include="..\Public\Effect_DirectAuthoredSourceIndex.h" />
    <ClInclude Include="..\Public\Effect_Object.h" />
    <ClInclude Include="..\Public\Effect_ScreenOverlayPresentation.h" />
    <ClInclude Include="..\Public\Effect_ThumbnailCache.h" />
    <ClInclude Include="..\Public\Effect_Catalog.h" />
    <ClInclude Include="..\Public\EffectResourceCatalog.h" />
    <ClInclude Include="..\Public\Effect_LoadPreparationJob.h" />
    <ClInclude Include="..\Public\Effect_OccurrenceTuning.h" />
    <ClInclude Include="..\Public\Effect_VisualProgramCorpus.h" />
    <ClInclude Include="..\Public\AnimationEffectCueDocument.h" />
    <ClInclude Include="..\Public\SoundCueCatalog.h" />
    <ClInclude Include="..\Public\EstherActionSoundCueDocument.h" />
    <ClInclude Include="..\Public\NpcActionEffectCueDocument.h" />
    <ClInclude Include="..\Public\Effect_PresentationService.h" />
    <ClInclude Include="..\Public\EffectFailureDiagnostic.h" />
    <ClInclude Include="..\Public\Effect_ProductPrewarmQueue.h" />
	<ClInclude Include="..\Private\CharacterSelectArenaSpawnGate.h" />
    <ClInclude Include="..\Public\Level_CharacterSelect.h" />
    <ClInclude Include="..\Public\CharacterSelectShowcase.h" />
    <ClInclude Include="..\Public\PlayableCharacterAssetService.h" />
    <ClInclude Include="..\Public\SquareHoleInstrumentCatalog.h" />
    <ClInclude Include="..\Public\DeployPropRuntime.h" />
    <ClInclude Include="..\Public\WorldSequencePlayer.h" />
    <ClInclude Include="..\Public\DestructionSimulationDocument.h" />
    <ClInclude Include="..\Public\DestructionSimulationRuntime.h" />
    <ClInclude Include="..\Public\DestructionSimulationController.h" />
    <ClInclude Include="..\Public\Trigger_Box.h" />
    <ClInclude Include="..\Public\ValtanPresentationAssetService.h" />
	<ClInclude Include="..\Public\NpcPlacementPresentationService.h" />
	<ClInclude Include="..\Public\NpcPresentationAssetService.h" />
	<ClInclude Include="..\Public\VehiclePresentationAssetService.h" />
	<ClInclude Include="..\Public\MonsterPresentationAssetService.h" />
	<ClInclude Include="..\Public\KoukuSaydonPresentationAssetService.h" />
	<ClInclude Include="..\Public\KoukuSaydonPresentationPlayer.h" />
	<ClInclude Include="..\Public\KoukuCinematicAnimationCatalog.h" />
	<ClInclude Include="..\Public\KoukuSaydonPreviewRootMotion.h" />
	<ClInclude Include="..\Public\KoukuSaydonAnimationBlend.h" />
	<ClInclude Include="..\Public\MonsterPresentationContract.h" />
    <ClInclude Include="..\Public\WorldEntityCommandSink.h" />
    <ClInclude Include="..\Public\NetworkWorldEntityCommandSink.h" />
    <ClInclude Include="..\Public\CharacterSpec.h" />
    <ClInclude Include="..\Public\ClientReplication.h" />
    <ClInclude Include="..\Public\CombatDebugVisibility.h" />
    <ClInclude Include="..\Public\PlayerHandGripTransform.h" />
    <ClInclude Include="..\Public\ClientReplicationEvent.h" />
	<ClInclude Include="..\Public\CombatObjectProjectionRuntime.h" />
    <ClInclude Include="..\Public\CombatHUDViewModel.h" />
    <ClInclude Include="..\Public\EstherCutinPresentationService.h" />
    <ClInclude Include="..\Public\RaidBossShowcaseService.h" />

    <ClInclude Include="..\Public\HUDRuntimeView.h" />
    <ClInclude Include="..\Public\CustomizingCostumeDocument.h" />
    <ClInclude Include="..\Public\CustomizingFacePresetDocument.h" />
    <ClInclude Include="..\Public\CustomizingFaceTextureDocument.h" />
    <ClInclude Include="..\Public\CustomizingIconDocument.h" />
    <ClInclude Include="..\Public\CustomizingView.h" />
    <ClInclude Include="..\Public\CharacterOutfitApplier.h" />
    <ClInclude Include="..\Public\DurabilityHudView.h" />
    <ClInclude Include="..\Public\InventoryView.h" />
    <ClInclude Include="..\Public\RepairWindowView.h" />
    <ClInclude Include="..\Public\ShopWindowView.h" />
    <ClInclude Include="..\Public\QuickSlotDragView.h" />
    <ClInclude Include="..\Public\CharacterInfoWindowView.h" />
    <ClInclude Include="..\Public\CharacterPortraitRenderer.h" />
    <ClInclude Include="..\Public\AvatarBookWindowView.h" />
    <ClInclude Include="..\Public\UILabelFont.h" />
    <ClInclude Include="..\Public\SkillWindowView.h" />
    <ClInclude Include="..\Public\ChatWindowView.h" />
    <ClInclude Include="..\Public\PartyWindowView.h" />
    <ClInclude Include="..\Public\ReplicatedPlayerHealth.h" />
    <ClInclude Include="..\Public\PartyTransferNotice.h" />
    <ClInclude Include="..\Public\WorldPlayerNameplateView.h" />
    <ClInclude Include="..\Public\WorldHealthBarView.h" />
    <ClInclude Include="..\Public\WorldPlayerChatBubbleView.h" />
    <ClInclude Include="..\Public\PartyInteractionView.h" />
    <ClInclude Include="..\Public\CombatAnalysisFrameView.h" />
    <ClInclude Include="..\Public\VehicleWindowView.h" />
    <ClInclude Include="..\Public\SystemOptionWindowView.h" />
    <ClInclude Include="..\Public\SystemOptionWindowView_Internal.h" />
    <ClInclude Include="..\Public\SystemOptionRowsDocument.h" />
    <ClInclude Include="..\Public\WorldMapWindowView.h" />
    <ClInclude Include="..\Public\SongCastGaugeView.h" />
    <ClInclude Include="..\Public\HonorTitleWindowView.h" />
    <ClInclude Include="..\Public\RaidEntryPreviewView.h" />
    <ClInclude Include="..\Public\UITextureCache.h" />
    <ClInclude Include="..\Public\UILayoutRuntime.h" />
    <ClInclude Include="..\Public\UIInputRouter.h" />
    <ClInclude Include="..\Public\UIWindowDrag.h" />
    <ClInclude Include="..\Public\UI_Sprite.h" />
    <ClInclude Include="..\Public\Level_Bern.h" />
    <ClInclude Include="..\Public\Level_KakulSaydonArena.h" />
    <ClInclude Include="..\Public\Level_Development.h" />
    <ClInclude Include="..\Public\ColosseumLoadingView.h" />
    <ClInclude Include="..\Public\ColosseumMatchView.h" />
    <ClInclude Include="..\Public\ColosseumIntroCutscene.h" />
    <ClInclude Include="..\Public\ColosseumMatchStart.h" />
    <ClInclude Include="..\Public\Level_Lobby.h" />
    <ClInclude Include="..\Public\LevelRegistry.h" />
	<ClInclude Include="..\Public\RenderingProfileService.h" />
    <ClInclude Include="..\Public\AnimationTargetService.h" />
    <ClInclude Include="..\Public\CharacterPreviewPanel.h" />
    <ClInclude Include="..\Public\EquipmentAuthoringTool.h" />
    <ClInclude Include="..\Public\EquipmentPresentationCatalog.h" />
    <ClInclude Include="..\Public\EquipmentPresentationService.h" />
    <ClInclude Include="..\Public\PlayableCharacterPreviewContract.h" />
    <ClInclude Include="..\Public\LobbyCommandService.h" />
    <ClInclude Include="..\Public\LevelTransitionService.h" />
    <ClInclude Include="..\Public\Level_ValtanArena.h" />
    <ClInclude Include="..\Public\Logic_LanceMaster.h" />
    <ClInclude Include="..\Public\Logic_GunSlinger.h" />
    <ClInclude Include="..\Public\Logic_Artist.h" />
    <ClInclude Include="..\Public\Logic_DimensionMaster.h" />
    <ClInclude Include="..\Public\Logic_Slayer.h" />
    <ClInclude Include="..\Public\Logic_Warlord.h" />
    <ClInclude Include="..\Public\Logic_GuardianKnight.h" />
    <ClInclude Include="..\Public\NetObjectRegistry.h" />
    <ClInclude Include="..\Public\Npc.h" />
    <ClInclude Include="..\Public\NetworkManager.h" />
    <ClInclude Include="..\Public\ClientSessionDiagnostic.h" />
    <ClInclude Include="..\Public\Part_Body.h" />
    <ClInclude Include="..\Public\Part_Vehicle.h" />
    <ClInclude Include="..\Public\Part_Equipment.h" />
	<ClInclude Include="..\Public\DeferredMaterialRenderUtils.h" />
    <ClInclude Include="..\Public\SourceCharacterMaterialParameters.h" />
    <ClInclude Include="..\Public\SourceEquipmentMaterialPrograms.h" />
    <ClInclude Include="..\Public\SourceMovieMaterialPrograms.h" />
    <ClInclude Include="..\Public\SourceMapForwardMaterialParameters.h" />
    <ClInclude Include="..\Public\SourceMapWaterMaterialParameters.h" />
    <ClInclude Include="..\Public\SourceMapTranslucentMaterialParameters.h" />
    <ClInclude Include="..\Public\MapAssetRenderUtils.h" />
    <ClInclude Include="..\Public\MapStaticBatchObject.h" />
    <ClInclude Include="..\Public\MapStaticChunkObject.h" />
    <ClInclude Include="..\Public\MapStaticProxyBaker.h" />
    <ClInclude Include="..\Public\Body_Valtan.h" />
    <ClInclude Include="..\Public\ValtanPatternTree.h" />
    <ClInclude Include="..\Public\ValtanCinematicEffectLibrary.h" />
    <ClInclude Include="..\Public\ValtanViewAdmission.h" />
    <ClInclude Include="..\Public\ValtanPresentationGenerationAdmission.h" />
    <ClInclude Include="..\Public\ValtanPatternAuditionService.h" />
    <ClInclude Include="..\public\Camera_Free.h" />
    <ClInclude Include="..\Public\Client_Defines.h" />
    <ClInclude Include="..\public\Effect_Tool.h" />
    <ClInclude Include="..\Public\Effect_Tool_V2.h" />
    <ClInclude Include="..\Public\EffectCompositionWorldResource.h" />
    <ClInclude Include="..\Public\EffectCompositionModelPreview.h" />
    <ClInclude Include="..\Public\EffectCompositionWorkbench.h" />
    <ClInclude Include="..\Public\EffectEditingSession.h" />
    <ClInclude Include="..\Public\EffectAuthoringResourceTree.h" />
    <ClInclude Include="..\Public\EffectAuthoringSequencer.h" />
    <ClInclude Include="..\Public\EffectRecoveryCamera.h" />
    <ClInclude Include="..\Public\RecoveryCameraAuthoringSession.h" />
    <ClInclude Include="..\Public\SequenceCameraEditor.h" />
    <ClInclude Include="..\Public\EffectAuthoringV2Pane.h" />
    <ClInclude Include="..\Public\EffectV2_Object.h" />
    <ClInclude Include="..\Public\EffectV2_Catalog.h" />
    <ClInclude Include="..\Public\EffectV2_Document.h" />
    <ClInclude Include="..\Public\EffectV2_Runtime.h" />
    <ClInclude Include="..\Public\EffectV2_Target.h" />
    <ClInclude Include="..\Public\HUDLayoutTool.h" />
    <ClInclude Include="..\Public\Level_Loading.h" />
    <ClInclude Include="..\public\Loader.h" />
    <ClInclude Include="..\Public\MainApp.h" />
    <ClInclude Include="..\Public\PlayerController.h" />
    <ClInclude Include="..\Public\PlayerCommandSink.h" />
    <ClInclude Include="..\Public\PlayerSkillCatalog.h" />
	<ClInclude Include="..\Public\SkillGroundTargetPreview.h" />
	<ClInclude Include="..\Public\ClickMoveEffect.h" />
    <ClInclude Include="..\Public\ItemCatalog.h" />
    <ClInclude Include="..\Public\HonorTitleCatalog.h" />
    <ClInclude Include="..\Public\NetworkPlayerCommandSink.h" />
    <ClInclude Include="..\Public\ProfilerCaptureIO.h" />
    <ClInclude Include="..\Public\ProfilerTool.h" />
    <ClInclude Include="..\Public\RenderingBenchmark.h" />
    <ClInclude Include="..\Public\RenderingTechniqueGuide.h" />
    <ClInclude Include="..\Public\RenderingReferenceGuide.h" />
    <ClInclude Include="..\Public\BossCompositionDocument.h" />
    <ClInclude Include="..\Public\SequencerTool.h" />
    <ClInclude Include="..\Public\AssetPreparationBatch.h" />
    <ClInclude Include="..\Public\MapAssetCatalog.h" />
    <ClInclude Include="..\Public\MapEffectDocument.h" />
    <ClInclude Include="..\Public\MapEffectPresentationRuntime.h" />
    <ClInclude Include="..\Public\MapLightDocument.h" />
    <ClInclude Include="..\Public\MapLightPresentationRuntime.h" />
    <ClInclude Include="..\Public\MaharakaWaterpangPresentation.h" />
    <ClInclude Include="..\Public\MapEditorWorkspaceService.h" />
    <ClInclude Include="..\Public\MapNavigationContract.h" />
    <ClInclude Include="..\Public\LevelNavigationDebug.h" />
    <ClInclude Include="..\Public\MapAssetObject.h" />
    <ClInclude Include="..\Public\DeployPropCatalog.h" />
    <ClInclude Include="..\Public\DeployPropObject.h" />
    <ClInclude Include="..\Public\MapAssetPreview.h" />
    <ClInclude Include="..\Public\MapPlacementDocument.h" />
    <ClInclude Include="..\Public\MapPlacementRuntime.h" />
    <ClInclude Include="..\Public\MapAuthoringHost.h" />
    <ClInclude Include="..\Public\MapPublishRunner.h" />
    <ClInclude Include="..\Public\MapPlacementEditSession.h" />
    <ClInclude Include="..\Public\MapLoadScope.h" />
    <ClInclude Include="..\Public\MapTool.h" />
	<ClInclude Include="..\Public\WorldSequenceDocument.h" />
	<ClInclude Include="..\Public\WorldSequenceToolPanel.h" />
    <ClInclude Include="..\Public\WorldGameplayDocument.h" />
    <ClInclude Include="..\Public\EncounterPatternReference.h" />
    <ClInclude Include="..\Public\WorldDestructionDocument.h" />
    <ClInclude Include="..\Public\SpawnGroupDocument.h" />
    <ClInclude Include="..\Public\NavGridBaker.h" />
    <ClInclude Include="..\Public\NavGridPaintDocument.h" />
    <ClInclude Include="..\Public\NavRuntimeBlockerDocument.h" />
    <ClInclude Include="..\Public\RuntimeAssetRoot.h" />
    <ClInclude Include="..\Public\ProjectDataRoot.h" />
    <ClInclude Include="..\Public\UserSettingsDocument.h" />
    <ClInclude Include="..\Public\ClientWindowDisplay.h" />
    <ClInclude Include="..\Public\DataJson.h" />
    <ClInclude Include="..\Public\Valtan.h" />
    <ClInclude Include="Client.h" />
    <ClInclude Include="framework.h" />
    <ClInclude Include="Resource.h" />
    <ClInclude Include="targetver.h" />
  </ItemGroup>
  <ItemGroup>
	<ClCompile Include="..\Private\ValtanActionWorkbench.cpp" />
    <ClCompile Include="..\Private\ValtanActionWorkbench_Details.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench_Preview.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClInclude Include="..\Private\ValtanActionWorkbench_EditSupport.h" />
	<ClCompile Include="..\Private\ValtanActionWorkbench_Blueprint.cpp" />
	<ClCompile Include="..\Private\ValtanActionWorkbench_Timeline.cpp" />
	<ClCompile Include="..\Private\ActionCompositionGraphModel.cpp" />
	<ClCompile Include="..\Private\CompositionResourceTree.cpp" />
	<ClCompile Include="..\Private\BossLogicFlowViewModel.cpp" />
	<ClCompile Include="..\Private\BossLogicFlowRenderer.cpp" />
	<ClCompile Include="..\Private\ActionPresentationTimeline.cpp" />
	<ClCompile Include="..\Private\CameraShakeService.cpp" />
	<ClCompile Include="..\Private\ValtanCinematicCameraController.cpp" />
	<ClCompile Include="..\Private\ValtanCinematicCameraDocument.cpp" />
	<ClCompile Include="..\Private\WorldDestructionProjectionDocument.cpp" />
	<ClCompile Include="..\Private\WorldDestructionDebrisPresentationDocument.cpp" />
	<ClCompile Include="..\Private\WorldDestructionDebrisPresentationRuntime.cpp" />
	<ClCompile Include="..\Private\WorldDestructionProjectionRuntime.cpp" />
    <ClCompile Include="..\Private\Animation_Tool.cpp" />
    <ClCompile Include="..\Private\AnimationAuthoringBridge.cpp" />
    <ClCompile Include="..\Private\AnimationSkillBindingDocument.cpp" />
    <ClCompile Include="..\Private\CharacterActionWorkbench.cpp" />
    <ClCompile Include="..\Private\BoneAnimationDocument.cpp" />
    <ClCompile Include="..\Private\BoneAnimationWorkbench.cpp" />
    <ClCompile Include="..\Private\BoneAnimationWorkbench_Viewport.cpp" />
    <ClCompile Include="..\Private\CharacterModelWorkbench.cpp" />
    <ClCompile Include="..\Private\CharacterActionCombatDocument.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonAnimationActionDocument.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonAnimationPatternDocument.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonCompositionDocument.cpp" />
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Animation.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Camera.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Clipboard.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Collider.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Document.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Helpers.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_History.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Logic.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Patterns.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Presentation.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Preview.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Publish.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Summon.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Timeline.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_WorldScene.cpp">
      <AdditionalOptions>/bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_CameraLookup.h" />
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_Internal.h" />
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_TimelineTypes.h" />
	<ClCompile Include="..\Private\KoukuSaydonBossTool.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonPatternAuditionService.cpp" />
	<ClCompile Include="..\Private\ValtanPatternFlowDocument.cpp" />
	<ClCompile Include="..\Private\ValtanPatternFlowService.cpp" />
	<ClCompile Include="..\Private\ValtanTuningCommandService.cpp" />
	<ClCompile Include="..\Private\ValtanPatternPreviewDocument.cpp" />
	<ClCompile Include="..\Private\ValtanPatternAuthoringEffectDocument.cpp" />
	<ClCompile Include="..\Private\ValtanCombatObjectSoundCueDocument.cpp" />
	<ClCompile Include="..\Private\ValtanPatternEffectCueDocument.cpp" />
	<ClCompile Include="..\Private\ValtanPatternEffectCueAuthoring.cpp" />
    <ClCompile Include="..\Private\ValtanPatternShakeCueDocument.cpp" />
    <ClCompile Include="..\Private\ValtanPatternSoundCueDocument.cpp" />
    <ClCompile Include="..\Private\BoneChainSimulation.cpp" />
    <ClCompile Include="..\Private\FaceCustomizeApplier.cpp" />
    <ClCompile Include="..\Private\FaceMorphApplier.cpp" />
    <ClCompile Include="..\Private\FaceSliderDocument.cpp" />
    <ClCompile Include="..\Private\BalanceTool.cpp" />
    <ClCompile Include="..\Private\BalanceTestPanel.cpp" />
      <ClCompile Include="..\Private\ValtanBossTool.cpp" />
      <ClCompile Include="..\Private\CameraTool.cpp" />
    <ClCompile Include="..\Private\Character.cpp" />
    <ClCompile Include="..\Private\HitAreaWire.cpp" />
    <ClCompile Include="..\Private\SkeletalAfterimage.cpp" />
    <ClCompile Include="..\Private\CharacterCatalog.cpp" />
    <ClCompile Include="..\Private\ActorCatalog.cpp" />
    <ClCompile Include="..\Private\CharacterSelectionState.cpp" />
    <ClCompile Include="..\Private\CharacterRoster.cpp" />
    <ClCompile Include="..\Private\CharacterSelectWindowView.cpp" />
    <ClCompile Include="..\Private\MinimapView.cpp" />
    <ClCompile Include="..\Private\KoukuMadnessGaugeView.cpp" />
    <ClCompile Include="..\Private\StatusEffectTextView.cpp" />
    <ClCompile Include="..\Private\DungeonTimerView.cpp" />
    <ClCompile Include="..\Private\MvpAwardCatalog.cpp" />
    <ClCompile Include="..\Private\MvpResultView.cpp" />
    <ClCompile Include="..\Private\RaidGateProgressView.cpp" />
    <ClCompile Include="..\Private\SystemMenuButtonsView.cpp" />
    <ClCompile Include="..\Private\InteractKeyPromptView.cpp" />
    <ClCompile Include="..\Private\UITextOcclusion.cpp" />
    <ClCompile Include="..\Private\BossImmuneGaugeView.cpp" />
    <ClCompile Include="..\Private\Effect_CascadeCompiler.cpp" />
	<ClCompile Include="..\Private\Effect_RuntimeAuthority.cpp" />
	<ClCompile Include="..\Private\Effect_Artist31470ShaderRegistry.cpp" />
	<ClCompile Include="..\Private\Effect_ReconstructedExecution.cpp" />
    <ClCompile Include="..\Private\Effect_DocumentCodec.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Distribution.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
    <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ParticleUpdatePool.cpp" />
    <ClCompile Include="..\Private\Effect_Playback.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
    <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_LightPresentation.cpp" />
    <ClCompile Include="..\Private\Effect_DocumentRenderer.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_MaterialProgramRegistry.cpp" />
    <ClCompile Include="..\Private\Effect_DirectAuthoredSourceIndex.cpp" />
    <ClCompile Include="..\Private\Effect_Object.cpp" />
    <ClCompile Include="..\Private\Effect_ScreenOverlayPresentation.cpp" />
    <ClCompile Include="..\Private\Effect_ThumbnailCache.cpp" />
    <ClCompile Include="..\Private\Effect_Catalog.cpp" />
    <ClCompile Include="..\Private\EffectResourceCatalog.cpp" />
    <ClCompile Include="..\Private\Effect_LoadPreparationJob.cpp" />
    <ClCompile Include="..\Private\Effect_OccurrenceTuning.cpp" />
    <ClCompile Include="..\Private\Effect_VisualProgramCorpus.cpp" />
    <ClCompile Include="..\Private\AnimationEffectCueDocument.cpp" />
    <ClCompile Include="..\Private\SoundCueCatalog.cpp" />
    <ClCompile Include="..\Private\EstherActionSoundCueDocument.cpp" />
    <ClCompile Include="..\Private\NpcActionEffectCueDocument.cpp" />
    <ClCompile Include="..\Private\Effect_PresentationService.cpp" />
    <ClCompile Include="..\Private\Effect_ProductPrewarmQueue.cpp" />
    <ClCompile Include="..\Private\Level_CharacterSelect.cpp" />
    <ClCompile Include="..\Private\CharacterSelectShowcase.cpp" />
    <ClCompile Include="..\Private\PlayableCharacterAssetService.cpp" />
    <ClCompile Include="..\Private\SquareHoleInstrumentCatalog.cpp" />
    <ClCompile Include="..\Private\Trigger_Box.cpp" />
    <ClCompile Include="..\Private\ValtanPresentationAssetService.cpp" />
	<ClCompile Include="..\Private\NpcPlacementPresentationService.cpp" />
	<ClCompile Include="..\Private\NpcPresentationAssetService.cpp" />
	<ClCompile Include="..\Private\VehiclePresentationAssetService.cpp" />
	<ClCompile Include="..\Private\MonsterPresentationAssetService.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonPresentationAssetService.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonPresentationPlayer.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
	<ClCompile Include="..\Private\KoukuSaydonPresentationPlayer_LogicPreview.cpp" />
	<ClCompile Include="..\Private\KoukuCinematicAnimationCatalog.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonPreviewRootMotion.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonAnimationBlend.cpp" />
    <ClCompile Include="..\Private\NetworkWorldEntityCommandSink.cpp" />
    <ClCompile Include="..\Private\ClientReplication.cpp" />
    <ClCompile Include="..\Private\ClientReplication_BattleItems.cpp" />
    <ClCompile Include="..\Private\ClientReplication_WaterGun.cpp" />
	<ClCompile Include="..\Private\CombatObjectProjectionRuntime.cpp" />
    <ClCompile Include="..\Private\CombatHUDViewModel.cpp" />
    <ClCompile Include="..\Private\EstherCutinPresentationService.cpp" />
    <ClCompile Include="..\Private\RaidBossShowcaseService.cpp" />

    <ClCompile Include="..\Private\HUDRuntimeView.cpp" />
    <ClCompile Include="..\Private\CustomizingCostumeDocument.cpp" />
    <ClCompile Include="..\Private\CustomizingFacePresetDocument.cpp" />
    <ClCompile Include="..\Private\CustomizingFaceTextureDocument.cpp" />
    <ClCompile Include="..\Private\CustomizingIconDocument.cpp" />
    <ClCompile Include="..\Private\CustomizingView.cpp" />
    <ClCompile Include="..\Private\CharacterOutfitApplier.cpp" />
    <ClCompile Include="..\Private\DurabilityHudView.cpp" />
    <ClCompile Include="..\Private\InventoryView.cpp" />
    <ClCompile Include="..\Private\RepairWindowView.cpp" />
    <ClCompile Include="..\Private\ShopWindowView.cpp" />
    <ClCompile Include="..\Private\QuickSlotDragView.cpp" />
    <ClCompile Include="..\Private\CharacterInfoWindowView.cpp" />
    <ClCompile Include="..\Private\CharacterPortraitRenderer.cpp" />
    <ClCompile Include="..\Private\AvatarBookWindowView.cpp" />
    <ClCompile Include="..\Private\SkillWindowView.cpp" />
    <ClCompile Include="..\Private\ChatWindowView.cpp" />
    <ClCompile Include="..\Private\PartyWindowView.cpp" />
    <ClCompile Include="..\Private\WorldPlayerNameplateView.cpp" />
    <ClCompile Include="..\Private\WorldHealthBarView.cpp" />
    <ClCompile Include="..\Private\WorldPlayerChatBubbleView.cpp" />
    <ClCompile Include="..\Private\PartyInteractionView.cpp" />
    <ClCompile Include="..\Private\CombatAnalysisFrameView.cpp" />
    <ClCompile Include="..\Private\VehicleWindowView.cpp" />
    <ClCompile Include="..\Private\SystemOptionWindowView.cpp" />
    <ClCompile Include="..\Private\SystemOptionWindowView_Rows.cpp" />
    <ClCompile Include="..\Private\SystemOptionRowsDocument.cpp" />
    <ClCompile Include="..\Private\WorldMapWindowView.cpp" />
    <ClCompile Include="..\Private\SongCastGaugeView.cpp" />
    <ClCompile Include="..\Private\HonorTitleWindowView.cpp" />
    <ClCompile Include="..\Private\RaidEntryPreviewView.cpp" />
    <ClCompile Include="..\Private\UITextureCache.cpp" />
    <ClCompile Include="..\Private\UILayoutRuntime.cpp" />
    <ClCompile Include="..\Private\UIInputRouter.cpp" />
    <ClCompile Include="..\Private\UIWindowDrag.cpp" />
    <ClCompile Include="..\Private\UI_Sprite.cpp" />
    <ClCompile Include="..\Private\Level_Bern.cpp" />
    <ClCompile Include="..\Private\Level_KakulSaydonArena.cpp" />
    <ClCompile Include="..\Private\Level_Development.cpp" />
    <ClCompile Include="..\Private\ColosseumMatchView.cpp" />
    <ClCompile Include="..\Private\Level_Lobby.cpp" />
    <ClCompile Include="..\Private\LevelRegistry.cpp" />
	<ClCompile Include="..\Private\RenderingProfileService.cpp" />
    <ClCompile Include="..\Private\AnimationTargetService.cpp" />
    <ClCompile Include="..\Private\CharacterPreviewPanel.cpp" />
    <ClCompile Include="..\Private\EquipmentAuthoringTool.cpp" />
    <ClCompile Include="..\Private\EquipmentPresentationCatalog.cpp" />
    <ClCompile Include="..\Private\EquipmentPresentationService.cpp" />
    <ClCompile Include="..\Private\PlayableCharacterPreviewContract.cpp" />
    <ClCompile Include="..\Private\LobbyCommandService.cpp" />
    <ClCompile Include="..\Private\LevelTransitionService.cpp" />
    <ClCompile Include="..\Private\Level_ValtanArena.cpp" />
    <ClCompile Include="..\Private\Logic_LanceMaster.cpp" />
    <ClCompile Include="..\Private\Logic_GunSlinger.cpp" />
    <ClCompile Include="..\Private\Logic_Artist.cpp" />
    <ClCompile Include="..\Private\Logic_DimensionMaster.cpp" />
    <ClCompile Include="..\Private\Logic_Slayer.cpp" />
    <ClCompile Include="..\Private\Logic_Warlord.cpp" />
    <ClCompile Include="..\Private\Logic_GuardianKnight.cpp" />
    <ClCompile Include="..\Private\NetObjectRegistry.cpp" />
    <ClCompile Include="..\Private\Npc.cpp" />
    <ClCompile Include="..\Private\NetworkManager.cpp" />
    <ClCompile Include="..\Private\ClientSessionDiagnostic.cpp" />
    <ClCompile Include="..\Private\Part_Body.cpp" />
    <ClCompile Include="..\Private\Part_Vehicle.cpp" />
    <ClCompile Include="..\Private\Part_Equipment.cpp" />
	<ClCompile Include="..\Private\DeferredMaterialRenderUtils.cpp" />
    <ClCompile Include="..\Private\MapAssetRenderUtils.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
    <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticBatchObject.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticChunkObject.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticProxyBaker.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Body_Valtan.cpp" />
    <ClCompile Include="..\Private\ValtanPatternTree.cpp" />
    <ClCompile Include="..\Private\ValtanCinematicEffectLibrary.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPresentationGenerationAdmission.cpp" />
    <ClCompile Include="..\Private\ValtanPatternAuditionService.cpp" />
    <ClCompile Include="..\private\Camera_Free.cpp" />
    <ClCompile Include="..\private\Effect_Tool.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_V2.cpp" />
    <ClCompile Include="..\Private\EffectCompositionWorldResource.cpp" />
    <ClCompile Include="..\Private\EffectCompositionModelPreview.cpp" />
    <ClCompile Include="..\Private\EffectCompositionWorkbench.cpp" />
    <ClCompile Include="..\Private\EffectEditingSession.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringResourceTree.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Camera.cpp" />
    <ClCompile Include="..\Private\EffectRecoveryCamera.cpp" />
    <ClCompile Include="..\Private\RecoveryCameraAuthoringSession.cpp" />
    <ClCompile Include="..\Private\SequenceCameraEditor.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Timeline.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Resources.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Tracks.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Presentation.cpp" />
    <ClCompile Include="..\Private\EffectAuthoringV2Pane.cpp" />
    <ClCompile Include="..\Private\Effect_Tool_Workspace.cpp" />
    <ClCompile Include="..\Private\EffectV2_Object.cpp" />
    <ClCompile Include="..\Private\EffectV2_Catalog.cpp" />
    <ClCompile Include="..\Private\EffectV2_Document.cpp" />
    <ClCompile Include="..\Private\EffectV2_Runtime.cpp" />
    <ClCompile Include="..\Private\HUDLayoutTool.cpp" />
    <ClCompile Include="..\Private\Level_Loading.cpp" />
    <ClCompile Include="..\private\Loader.cpp" />
    <ClCompile Include="..\Private\MainApp.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_Profiler.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles>WinSock2.h;Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_Dragon.cpp" />
    <ClCompile Include="..\Private\MainApp_ValtanAxe.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_SequenceViewer.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_WorldLevel.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\PlayerController.cpp" />
    <ClCompile Include="..\Private\PlayerSkillCatalog.cpp" />
	<ClCompile Include="..\Private\SkillGroundTargetPreview.cpp" />
	<ClCompile Include="..\Private\ClickMoveEffect.cpp" />
    <ClCompile Include="..\Private\ItemCatalog.cpp" />
    <ClCompile Include="..\Private\HonorTitleCatalog.cpp" />
    <ClCompile Include="..\Private\NetworkPlayerCommandSink.cpp" />
    <ClCompile Include="..\Private\ProfilerCaptureIO.cpp" />
    <ClCompile Include="..\Private\ProfilerTool.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\RenderingBenchmark.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\BossCompositionDocument.cpp" />
    <ClCompile Include="..\Private\SequencerTool.cpp" />
    <ClCompile Include="..\Private\AssetPreparationBatch.cpp" />
    <ClCompile Include="..\Private\MapAssetCatalog.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\MapEffectDocument.cpp" />
    <ClCompile Include="..\Private\MapEffectPresentationRuntime.cpp" />
    <ClCompile Include="..\Private\MapLightDocument.cpp" />
    <ClCompile Include="..\Private\MapLightPresentationRuntime.cpp" />
    <ClCompile Include="..\Private\MaharakaWaterpangPresentation.cpp" />
    <ClCompile Include="..\Private\MapEditorWorkspaceService.cpp" />
    <ClCompile Include="..\Private\MapNavigationContract.cpp" />
    <ClCompile Include="..\Private\LevelNavigationDebug.cpp" />
    <ClCompile Include="..\Private\MapAssetObject.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\DeployPropCatalog.cpp" />
    <ClCompile Include="..\Private\DeployPropObject.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\DeployPropRuntime.cpp" />
    <ClCompile Include="..\Private\WorldSequencePlayer.cpp" />
    <ClCompile Include="..\Private\DestructionSimulationDocument.cpp" />
    <ClCompile Include="..\Private\DestructionSimulationRuntime.cpp" />
    <ClCompile Include="..\Private\DestructionSimulationController.cpp" />
    <ClCompile Include="..\Private\MapAssetPreview.cpp" />
    <ClCompile Include="..\Private\MapPlacementDocument.cpp" />
    <ClCompile Include="..\Private\MapPlacementRuntime.cpp" />
    <ClCompile Include="..\Private\MapAuthoringHost.cpp" />
    <ClCompile Include="..\Private\MapPublishRunner.cpp" />
    <ClCompile Include="..\Private\MapPlacementEditSession.cpp" />
    <ClCompile Include="..\Private\MapTool.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
	<ClCompile Include="..\Private\WorldSequenceDocument.cpp" />
	<ClCompile Include="..\Private\WorldSequenceToolPanel.cpp">
	  <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\WorldGameplayDocument.cpp" />
    <ClCompile Include="..\Private\EncounterPatternReference.cpp" />
    <ClCompile Include="..\Private\WorldDestructionDocument.cpp" />
    <ClCompile Include="..\Private\SpawnGroupDocument.cpp" />
    <ClCompile Include="..\Private\NavGridBaker.cpp" />
    <ClCompile Include="..\Private\NavGridPaintDocument.cpp" />
    <ClCompile Include="..\Private\NavRuntimeBlockerDocument.cpp" />
    <ClCompile Include="..\Private\RuntimeAssetRoot.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\ProjectDataRoot.cpp" />
    <ClCompile Include="..\Private\UserSettingsDocument.cpp" />
    <ClCompile Include="..\Private\ClientWindowDisplay.cpp" />
    <ClCompile Include="..\Private\DataJson.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Valtan.cpp" />
    <ClCompile Include="Client.cpp" />
  </ItemGroup>
  <ItemGroup>
    <Image Include="Client.ico" />
    <Image Include="small.ico" />
    <Image Include="cursor.cur" />
  </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="Client.rc" />
  </ItemGroup>
  <ItemGroup>
    <None Include="Client.vcxproj" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMesh.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEstherNpc.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl">
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl" />
	<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxSkillGroundTargetPreview.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticle.hlsl">
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectTrail.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectDecal.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectAnimMeshV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectMeshV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectRectV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectParticleV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectMeshParticleV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectDecalV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectTrailV2.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshPreview_V2.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Diagnostic.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectCommon.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_MapMaterialSurface.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapForwardPrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapWaterPrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapTranslucentPrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapDirectPrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_StaticShadowMap.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SceneHeightFog.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceStoneSurface.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceSpecialSurface.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceLandscapeSurface.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceFoliageSurface.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceFoliageWind.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMaharakaStandWind.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectUe3MaterialFamilies.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterQNative.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNative.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterVNative.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeSampleScene.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeColorResponse.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectSliceSceneDepth.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectRuntimeMaterialPacket.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectStandardColorV1.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLocalDecalAdapter.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDecalReceiver.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Ue3Glasshole02.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Ue3Glasshole02Translation.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapInstance.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapChunk.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapProxy.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshPreview.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxTex.hlsl" />
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{1a5672c6-54b4-4779-99e7-84ef85171388}</ProjectGuid>
    <RootNamespace>Client</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="Shared">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros">
    <!-- The project owns runtime deployment. Do not copy DLLs from a developer's vcpkg installation. -->
    <VcpkgApplocalDeps>false</VcpkgApplocalDeps>
  </PropertyGroup>
  <PropertyGroup>
    <!-- Keep command-line project builds and Framework.sln builds on one
         canonical intermediate tree. The VC++ default otherwise inserts the
         project name only outside a solution, allowing stale LTCG objects to
         overwrite a freshly linked Client.exe when Visual Studio launches. -->
    <IntDir>$(ProjectDir)$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <OutDir>..\Bin\Debug\</OutDir>
    <LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.200.139</LocalDebuggerEnvironment>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <OutDir>..\Bin\Release\</OutDir>
    <LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.200.139</LocalDebuggerEnvironment>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>../Public/;../../EngineSDK/Inc/;../../Shared/Public/;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>../../EngineSDK/lib/Debug/;../../EngineSDK/lib/</AdditionalLibraryDirectories>
      <AdditionalDependencies>Engine.lib;DirectXTKd.lib;Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
    <FxCompile>
      <ShaderType>Effect</ShaderType>
    </FxCompile>
    <FxCompile>
      <ShaderModel>5.0</ShaderModel>
    </FxCompile>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>../Public/;../../EngineSDK/Inc/;../../Shared/Public/;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>../../EngineSDK/lib/Release/;../../EngineSDK/lib/</AdditionalLibraryDirectories>
      <AdditionalDependencies>Engine.lib;DirectXTK.lib;Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
    <FxCompile>
      <ShaderType>Effect</ShaderType>
    </FxCompile>
    <FxCompile>
      <ShaderModel>5.0</ShaderModel>
    </FxCompile>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ProjectReference Include="..\..\Engine\Default\Engine.vcxproj">
      <Project>{922baefb-8a3c-4f15-9f51-bf7c7ecc1cd7}</Project>
      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
      <LinkLibraryDependencies>false</LinkLibraryDependencies>
    </ProjectReference>
    <ProjectReference Include="..\..\Shared\Default\Shared.vcxproj">
      <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
      <LinkLibraryDependencies>true</LinkLibraryDependencies>
    </ProjectReference>
  </ItemGroup>
  <Import Project="..\..\Tools\Build\CppCompilation.props" />
  <ItemGroup>
    <ClCompile Include="..\Private\Client_Pch.cpp">
      <PrecompiledHeader Condition="'$(LostArkUsePch)'=='true' and '$(Platform)'=='x64'">Create</PrecompiledHeader>
      <ExcludedFromBuild Condition="'$(LostArkUsePch)'!='true' or '$(Platform)'!='x64'">true</ExcludedFromBuild>
    </ClCompile>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h" />
  </ItemGroup>
  <ItemGroup>
    <Manifest Include="Client.manifest" />
  </ItemGroup>
  <ItemDefinitionGroup>
    <Manifest>
      <AdditionalManifestFiles>$(ProjectDir)Client.manifest;%(AdditionalManifestFiles)</AdditionalManifestFiles>
    </Manifest>
  </ItemDefinitionGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <PropertyGroup>
    <EngineRoot>$(ProjectDir)..\..\Engine\</EngineRoot>
    <EngineSdkRoot>$(ProjectDir)..\..\EngineSDK\</EngineSdkRoot>
    <EngineBinaryRoot>$(EngineRoot)Bin\$(Configuration)\</EngineBinaryRoot>
    <EngineSdkConfigurationLibRoot>$(EngineSdkRoot)lib\$(Configuration)\</EngineSdkConfigurationLibRoot>
    <AssimpRuntimeName Condition="'$(Configuration)'=='Debug'">assimp-vc143-mtd.dll</AssimpRuntimeName>
    <AssimpRuntimeName Condition="'$(Configuration)'=='Release'">assimp-vc143-mt.dll</AssimpRuntimeName>
    <AssimpRuntimeSource>$(EngineRoot)ThirdPartyLib\Assimp\Bin\$(Configuration)\$(AssimpRuntimeName)</AssimpRuntimeSource>
    <PhysXRuntimeRoot>$(EngineRoot)ThirdPartyLib\PhysX\Bin\$(Configuration)\</PhysXRuntimeRoot>
    <EngineCellShaderSource>$(EngineBinaryRoot)Shader_Cell.cso</EngineCellShaderSource>
    <EngineDeferredShaderSource>$(EngineBinaryRoot)Shader_Deferred.cso</EngineDeferredShaderSource>
  </PropertyGroup>
  <Target Name="ValidateClientBuildDomains" BeforeTargets="ClCompile" Condition="'$(Platform)'=='x64' and '$(LostArkPublishRuntimeData)'=='true'">
    <Exec Command="powershell.exe -NoProfile -ExecutionPolicy Bypass -File &quot;$(ProjectDir)..\..\Tools\Build\Invoke-BuildDomainOwner.ps1&quot; -Owner Client -ResourceRoot &quot;$(ProjectDir)..\Bin\Resources&quot;" />
  </Target>
  <Target Name="PrepareEngineSdk" BeforeTargets="FxExport;FxCompile;ClCompile" DependsOnTargets="ResolveProjectReferences" Condition="'$(Platform)'=='x64'">
    <ItemGroup>
      <EnginePublicHeaders Include="$(EngineRoot)Public\**\*.*" />
      <ImGuiPublicHeaders Include="$(EngineRoot)External\imgui\imgui.h;$(EngineRoot)External\imgui\imconfig.h" />
      <EngineSdkConfigurationLibraries Include="$(EngineBinaryRoot)*.lib" />
      <EngineSdkThirdPartyLibraries Include="$(EngineRoot)ThirdPartyLib\*.lib" />
      <EngineShaderSources Include="$(EngineRoot)Bin\ShaderFiles\*.*" />
    </ItemGroup>
    <MakeDir Directories="$(EngineSdkRoot)inc;$(EngineSdkRoot)lib;$(EngineSdkConfigurationLibRoot);$(EngineSdkRoot)hlsl;$(ProjectDir)..\Bin\ShaderFiles" />
    <Copy SourceFiles="@(EnginePublicHeaders)" DestinationFiles="@(EnginePublicHeaders->'$(EngineSdkRoot)inc\%(RecursiveDir)%(Filename)%(Extension)')" SkipUnchangedFiles="true" />
    <Copy SourceFiles="@(ImGuiPublicHeaders)" DestinationFolder="$(EngineSdkRoot)inc" SkipUnchangedFiles="true" />
    <Copy SourceFiles="@(EngineSdkConfigurationLibraries)" DestinationFolder="$(EngineSdkConfigurationLibRoot)" SkipUnchangedFiles="true" />
    <Copy SourceFiles="@(EngineSdkThirdPartyLibraries)" DestinationFolder="$(EngineSdkRoot)lib" SkipUnchangedFiles="true" />
    <Copy SourceFiles="@(EngineShaderSources)" DestinationFolder="$(EngineSdkRoot)hlsl" SkipUnchangedFiles="true" />
    <Copy SourceFiles="@(EngineShaderSources)" DestinationFolder="$(ProjectDir)..\Bin\ShaderFiles" SkipUnchangedFiles="true" />
  </Target>
  <Target Name="DeployClientCompiledShaders" AfterTargets="Build" Condition="'$(Platform)'=='x64'">
    <Error Condition="!Exists('$(EngineCellShaderSource)')" Text="Engine compiled shader is missing for $(Configuration): $(EngineCellShaderSource)" />
    <Error Condition="!Exists('$(EngineDeferredShaderSource)')" Text="Engine compiled shader is missing for $(Configuration): $(EngineDeferredShaderSource)" />
    <ItemGroup>
      <ClientEngineCompiledShaders Include="$(EngineCellShaderSource)" />
      <ClientEngineCompiledShaders Include="$(EngineDeferredShaderSource)" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_ScreenSpaceLighting.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_GpuSceneVisibility.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_GpuHiZCopy.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_GpuHiZReduce.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup001.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup009.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup017.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup025.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup080.cso" />
      <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup084.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup160.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup176.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup192.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup208.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup235.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup214.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup224.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup237.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1136.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1120.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1104.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup108.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup096.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1472.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1408.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1344.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1152.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup1088.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup896.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup768.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup640.cso" />
    <ClientEngineCompiledShaders Include="$(EngineBinaryRoot)Shader_Deferred_SourceGroup576.cso" />
    </ItemGroup>
    <Error Condition="!Exists('%(ClientEngineCompiledShaders.Identity)')" Text="Required Engine compiled shader is missing: %(ClientEngineCompiledShaders.Identity)" />
    <MakeDir Directories="$(TargetDir)" />
    <Copy SourceFiles="@(ClientEngineCompiledShaders)" DestinationFolder="$(TargetDir)" SkipUnchangedFiles="true" />
    <Message Text="Deployed Engine compiled shaders to $(TargetDir)" Importance="high" />
  </Target>
  <Target Name="DeployClientRuntimeDependencies" AfterTargets="Build" Condition="'$(Platform)'=='x64'">
    <Error Condition="'$(AssimpRuntimeName)'==''" Text="Unsupported configuration '$(Configuration)' for Assimp runtime deployment." />
    <Error Condition="!Exists('$(AssimpRuntimeSource)')" Text="Required Assimp runtime is missing: $(AssimpRuntimeSource)" />
    <Error Condition="!Exists('$(PhysXRuntimeRoot)PhysX_64.dll')" Text="Required PhysX runtime is missing: $(PhysXRuntimeRoot)PhysX_64.dll" />
    <Error Condition="!Exists('$(PhysXRuntimeRoot)PhysXCommon_64.dll')" Text="Required PhysX runtime is missing: $(PhysXRuntimeRoot)PhysXCommon_64.dll" />
    <Error Condition="!Exists('$(PhysXRuntimeRoot)PhysXFoundation_64.dll')" Text="Required PhysX runtime is missing: $(PhysXRuntimeRoot)PhysXFoundation_64.dll" />
    <Error Condition="!Exists('$(EngineBinaryRoot)Engine.dll')" Text="Engine runtime was not built for $(Configuration): $(EngineBinaryRoot)Engine.dll" />
    <Error Condition="'$(Configuration)'=='Debug' and !Exists('$(EngineBinaryRoot)Engine.pdb')" Text="Engine debug symbols were not built: $(EngineBinaryRoot)Engine.pdb" />
    <ItemGroup>
      <ClientRuntimeDependencies Include="$(EngineBinaryRoot)Engine.dll" />
      <ClientRuntimeDependencies Include="$(EngineBinaryRoot)Engine.pdb" Condition="'$(Configuration)'=='Debug'" />
      <ClientRuntimeDependencies Include="$(EngineRoot)ThirdPartyLib\FMOD\Bin\fmod.dll" />
      <ClientRuntimeDependencies Include="$(AssimpRuntimeSource)" />
      <ClientRuntimeDependencies Include="$(PhysXRuntimeRoot)PhysX_64.dll" />
      <ClientRuntimeDependencies Include="$(PhysXRuntimeRoot)PhysXCommon_64.dll" />
      <ClientRuntimeDependencies Include="$(PhysXRuntimeRoot)PhysXFoundation_64.dll" />
    </ItemGroup>
    <Copy SourceFiles="@(ClientRuntimeDependencies)" DestinationFolder="$(TargetDir)" SkipUnchangedFiles="true" />
    <Message Condition="'$(Configuration)'=='Debug'" Text="Deployed $(AssimpRuntimeName), Engine.dll, Engine.pdb, fmod.dll, and PhysX runtimes to $(TargetDir)" Importance="high" />
    <Message Condition="'$(Configuration)'!='Debug'" Text="Deployed $(AssimpRuntimeName), Engine.dll, fmod.dll, and PhysX runtimes to $(TargetDir)" Importance="high" />
  </Target>
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\ClassSelectionTimeline.h" />
    <ClInclude Include="..\Public\ClassMovieInspection.h" />
    <ClInclude Include="..\Public\ClassMovieInspector.h" />
    <ClCompile Include="..\Private\ClassMovieInspector.cpp" />
    <ClCompile Include="..\Private\ClassSelectionPresentation_Inspection.cpp" />
    <ClInclude Include="..\Public\ClassSelectionPresentation.h" />
    <ClCompile Include="..\Private\ClassSelectionPresentation.cpp" />
    <ClCompile Include="..\Private\ClassSelectionPresentation_Authoring.cpp" />
    <ClInclude Include="..\Public\ArenaCameraProfile.h" />
    <ClCompile Include="..\Private\ArenaCameraProfile.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\LightResourceCatalog.h" />
    <ClCompile Include="..\Private\LightResourceCatalog.cpp" />
    <ClInclude Include="..\Public\AreaLightAuthoringSession.h" />
    <ClCompile Include="..\Private\AreaLightAuthoringSession.cpp" />
    <ClCompile Include="..\Private\MainApp_RenderingLighting.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\WorldSequenceObject.cpp" />
    <ClCompile Include="..\Private\WorldSequencePlayer_Objects.cpp" />
    <ClCompile Include="..\Private\Level_KakulSaydonArena_WorldObjects.cpp" />
    <ClCompile Include="..\Private\WorldObjectTool.cpp" />
    <ClCompile Include="..\Private\WorldLevelTool.cpp" />
    <ClCompile Include="..\Private\WorldLevelHierarchy.cpp" />
    <ClCompile Include="..\Private\WorldSceneTool.cpp" />
    <ClCompile Include="..\Private\WorldSceneTool_Animation.cpp" />
    <ClInclude Include="..\Public\WorldSceneTool.h" />
    <ClCompile Include="..\Private\PhysicalResourceCatalog.cpp" />
    <ClInclude Include="..\Public\WorldSequenceObject.h" />
    <ClInclude Include="..\Public\WorldObjectTool.h" />
    <ClInclude Include="..\Public\WorldLevelTool.h" />
    <ClInclude Include="..\Public\WorldLevelHierarchy.h" />
    <ClInclude Include="..\Public\EditorUndoHistory.h" />
    <ClInclude Include="..\Public\PhysicalResourceCatalog.h" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_DimensionMasterWRMaterial.h" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterWRNative.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_DimensionMasterSDMaterial.h" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterSDNative.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_ArtistMaterial.h" />
    <ClInclude Include="..\Public\Effect_WarlordNativeMaterial.h" />
    <ClInclude Include="..\Public\Effect_NativeScreenPostMaterial.h" />
    <ClCompile Include="..\Private\Effect_NativeScreenPostMaterial.cpp" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNative.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeInputs.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeCommon.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativePrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup1600.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup1664.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2304.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2368.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2432.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2496.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2560.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2624.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2688.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2752.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2816.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2880.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2944.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3008.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3072.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3136.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3200.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3264.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3328.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3584.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3648.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3712.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3776.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup448.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup512.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup832.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNative.hlsli" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectNativeScreenPost.hlsl" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_LanceMasterVAMaterial.h" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANative.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_ShaderFamily.h" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterQ.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterV.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterWR.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterSD.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV064.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV128.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV192.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist448.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist512.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist768.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist832.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster512.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster576.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster640.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster704.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster768.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterQ.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterV.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterWR.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterSD.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV064.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV128.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV192.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist448.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist512.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist768.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist832.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster512.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster576.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster640.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster704.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster768.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Engine_Shader_Defines.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active003RibbonMaterial.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active011OuterMaterial.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active022DecalMaterial.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470RuntimeMaterial.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup448.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup512.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup832.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup064.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup128.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup192.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterVNativeShared.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup512.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup576.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup640.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup704.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectMeshFamilyCarrier.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectParticleFamilyCarrier.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectV2_Common.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterMaterial.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapCharacterSelectDirect.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapSL10Forward.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapSL10Direct.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapCharacterSelectPrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterForward.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterPrograms.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectNativeScreenUV.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist1600.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist1664.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1152.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1216.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1280.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1344.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1024.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1088.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1152.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1984.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord960.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist1600.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist1664.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1152.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1216.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1280.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1344.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1024.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1088.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1152.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1984.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord960.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup1600.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup1664.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1216.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1280.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1344.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1024.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1088.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1984.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup960.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2304.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2304.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2304.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWorldNative.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2368.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2432.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2560.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2688.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2752.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2816.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2880.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2944.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3008.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3072.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3136.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3200.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3264.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3328.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3584.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2368.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2432.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2560.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2688.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2752.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2816.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2880.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2944.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3008.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3072.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3136.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3200.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3264.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3328.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3584.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2368.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2432.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2560.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2688.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2752.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2816.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2880.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2944.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3008.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3072.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3136.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3200.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3264.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3328.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3584.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Effect_Tool_Internal.h" />
    <ClCompile Include="..\Private\Effect_Tool_ResourceBrowser.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Detail.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_MaterialDetail.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_CatalogPreview.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Valtan.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_ValtanWorld.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Editing.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_DocumentIo.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Playback.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_OccurrenceAuthoring.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_TransformHistory.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Helpers.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Effect_DocumentRenderer_Internal.h" />
    <ClCompile Include="..\Private\Effect_DocumentRenderer_ResourceStaging.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_PreparedDocument.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Catalog.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Staging.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_MaterialBinding.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Geometry.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Particles.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Rendering.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_MaterialHelpers.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_GeometryHelpers.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_CacheHelpers.cpp">
      <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase448.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase512.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase832.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchAdditionalArtistCases1600.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchAdditionalArtistCases1664.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchWorldNativeCases2304.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2304.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2368.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2432.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2560.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3584.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2688.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2816.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2880.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2944.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3008.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3072.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3136.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3200.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3264.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3328.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase64.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase128.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase192.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase512.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase576.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase640.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase704.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1216.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1280.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1344.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase0.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase960.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1024.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1088.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1984.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBasePrograms.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightPrograms.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\MapTool_Internal.h" />
    <ClInclude Include="..\Private\MapTool_NavigationResources.h" />
    <ClCompile Include="..\Private\MapTool_DestructionAuthoring.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_WorldGameplay.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Cutscenes.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_WorldGameplayPanels.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Navigation.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Area.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_NavigationPanels.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Placements.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_DestructionPanels.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_CameraShots.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_NavigationBake.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Helpers.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>

      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Animation_Tool_Internal.h" />
    <ClCompile Include="..\Private\Animation_Tool_ValtanPlayback.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_KoukuPlayback.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_ValtanComposition.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_PatternAuthoring.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_CompositionSounds.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_DocumentIo.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_ValtanInspector.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_ValtanPanels.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_KoukuAuthoring.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_SkillBindings.cpp" />
    <ClCompile Include="..\Private\Animation_Tool_Helpers.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\SourceCharacterMaterialParameters.cpp" />
    <None Include="..\Private\SourceCharacterMaterialParameters_Generated.inl" />
    <ClCompile Include="..\Private\Effect_ArtistMaterial.cpp" />
    <None Include="..\Private\Effect_ArtistMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_LanceMasterVAMaterial.cpp" />
    <None Include="..\Private\Effect_LanceMasterVAMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_WarlordNativeMaterial.cpp" />
    <None Include="..\Private\Effect_WarlordNativeMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_DimensionMasterALTVMaterial.cpp" />
    <None Include="..\Private\Effect_DimensionMasterALTVMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_DimensionMasterQMaterial.cpp" />
    <None Include="..\Private\Effect_DimensionMasterQMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_DimensionMasterVMaterial.cpp" />
    <None Include="..\Private\Effect_DimensionMasterVMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_DimensionMasterWRMaterial.cpp" />
    <None Include="..\Private\Effect_DimensionMasterWRMaterial_Tables.inl" />
    <ClCompile Include="..\Private\Effect_DimensionMasterSDMaterial.cpp" />
    <None Include="..\Private\Effect_DimensionMasterSDMaterial_Tables.inl" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup000.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Effect_DocumentCodec_Artist31470.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_AuthoredElements.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_AuthoringOverrides.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_DetailIo.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_JsonPrimitives.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_MaterialIo.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_MaterialValidation.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_PortableRuntime.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_RuntimeValidation.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_SourceRecipeIo.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_SourceValidation.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_Validation.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
    <ClInclude Include="..\Private\Effect_DocumentCodec_Internal.h" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup001.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup009.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup017.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup025.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup080.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup084.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup160.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup176.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup192.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup208.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup235.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup214.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup224.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup237.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1136.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1120.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1104.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup108.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup096.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1472.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1408.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1344.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1088.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup896.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup640.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup576.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup001.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup009.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup017.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup025.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup080.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup084.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup160.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup176.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup192.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup208.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup235.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup214.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup224.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup237.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1136.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1120.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1104.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup108.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup096.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1472.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1408.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1344.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1152.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1088.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup896.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup768.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup640.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup576.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup001.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup009.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup017.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup025.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup080.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup084.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup160.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup176.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup192.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup208.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup235.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup214.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup224.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup237.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1136.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1120.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1104.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup108.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup096.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1472.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1408.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1344.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1152.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1088.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup896.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup768.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup640.hlsl" />
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup576.hlsl" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup001.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup009.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup017.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup025.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup080.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup084.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup160.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup176.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup192.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup208.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup235.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup214.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup224.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup237.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1136.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1120.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1104.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup108.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup096.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1472.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1408.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1344.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1152.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1088.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup896.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup768.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup640.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup576.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup001.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup009.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup017.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup025.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup080.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup084.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup160.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup176.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup192.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup208.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup235.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup214.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup224.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup237.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1136.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1120.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1104.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup108.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup096.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1472.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1408.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1344.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1152.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1088.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup896.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup768.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup640.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup576.hlsl">
      <EnableDebuggingInformation Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</EnableDebuggingInformation>
      <DisableOptimizations Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectSceneColorInput.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectSceneDepthInput.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3712.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3776.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3712.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3776.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3712.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3776.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3712.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3776.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectVehicleModelNative.hlsli" />
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2496.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2496.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2496.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2496.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2624.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2624.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2624.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3648.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3648.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3648.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3648.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2624.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3840.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3840.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3840.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3840.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3840.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3904.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3904.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3904.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3904.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3904.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4480.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4480.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4480.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4480.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4480.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3968.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4032.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4096.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4160.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4224.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4288.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3968.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4032.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4096.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4160.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4224.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4288.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3968.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4032.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4096.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4160.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4224.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4288.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3968.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4032.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4096.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4160.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4224.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4288.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3968.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4032.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4096.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4160.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4224.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4288.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4352.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4352.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4352.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4352.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4416.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4416.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4416.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4416.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4416.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4544.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4544.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4544.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4544.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4544.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4352.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4608.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4608.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4608.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4608.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4608.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4672.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4672.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4672.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4672.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4672.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4736.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4736.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4736.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4736.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4736.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4800.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4800.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4800.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4800.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4800.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4864.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4928.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4864.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4928.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4864.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4928.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4864.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4928.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4864.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4928.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4992.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4992.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4992.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4992.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4992.hlsli" />
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5056.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5056.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5120.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5056.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5120.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5056.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5120.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5056.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5120.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5120.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\GuideAIDocument.h" />
    <ClInclude Include="..\Public\GuideAITool.h" />
    <ClInclude Include="..\Public\MaharakaAITool.h" />
    <ClCompile Include="..\Private\MaharakaAITool.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\GuideAIDocument.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\GuideAITool.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
    <ClCompile Include="..\Private\GuideAITool_Combat.cpp">
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles />
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5184.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5184.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5184.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5184.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5184.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5248.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5248.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5248.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5248.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5248.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5312.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5312.hlsl">
      <DisableOptimizations Condition="'$(Platform)'=='x64'">false</DisableOptimizations>
      <AdditionalOptions Condition="'$(Platform)'=='x64'">/O1 %(AdditionalOptions)</AdditionalOptions>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5312.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5312.hlsli" />
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5312.hlsli" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\MapGpuScene.h" />
    <ClCompile Include="..\Private\MapGpuScene.cpp" />
  </ItemGroup>
</Project>

```

## Client/Default/Client.vcxproj.filters 완전 교체본

변경 종류: 기존 파일 전체 교체. 위치: 파일의 처음부터 마지막 줄까지. 무관한 기존 내용은 보존한다.

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" ToolsVersion="4.0">
  <ItemGroup>
    <Filter Include="00.MainApp">
      <UniqueIdentifier>{15368830-7c72-413d-a000-0a1f7b1be562}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels">
      <UniqueIdentifier>{7bdb574f-1642-46a6-9c4a-2de9b918c674}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\00. Lobby">
      <UniqueIdentifier>{82c14815-ff74-4634-8e69-11e1d363979b}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\01. CharacterSelect">
      <UniqueIdentifier>{90eb2e93-684d-4600-b517-266b3e6c7c9f}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\02. Bern">
      <UniqueIdentifier>{812b4a10-ea33-4813-90a1-6f295ebb9ae7}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\04. KoukuSaydonArena">
      <UniqueIdentifier>{1b61a221-773c-4b61-9693-50cfdfd47852}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\04. ValtanArena">
      <UniqueIdentifier>{a4c9a668-668d-40d1-95bb-17c06b9257fa}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\05. Loading">
      <UniqueIdentifier>{de01aad6-a07c-47d8-a504-d56ab362410e}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\05. Loading\Loader">
      <UniqueIdentifier>{c407bb7a-0764-43ac-abfc-a5b8dba1f941}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Levels\06. Development">
      <UniqueIdentifier>{22145cb1-0a78-4f58-9c5c-fd2bf0402701}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects">
      <UniqueIdentifier>{4c84a0ed-a665-4d3f-ab10-9cf28db3ef67}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character">
      <UniqueIdentifier>{d5864814-d199-42de-9bce-7329814f62a7}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\00. LanceMaster">
      <UniqueIdentifier>{f8e0d2c8-a70a-4338-9e85-443f90e003db}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\00.ActorAndParts">
      <UniqueIdentifier>{b6468ff3-affc-550e-a939-b54b50263427}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\01. GunSlinger">
      <UniqueIdentifier>{3b7c1f42-6d95-4a08-b1e7-2c9048a5f6d1}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\01.Equipment">
      <UniqueIdentifier>{d3fc0d47-9ebb-5a2d-89f4-060899db5ab5}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\02. Artist">
      <UniqueIdentifier>{a708339b-9aaf-468a-bfb9-c2d18436186e}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\03. Slayer">
      <UniqueIdentifier>{15752942-3ae9-4fcd-91ce-48f56a75769b}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\03.CombatPresentation">
      <UniqueIdentifier>{d6191f1f-97c6-55fa-a39e-2a75f963b5e4}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\04. DimensionMaster">
      <UniqueIdentifier>{34e43f1d-330a-473a-a21f-b446190b9a05}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\05. Warlord">
      <UniqueIdentifier>{7b3c1d68-9f24-4a51-8e07-2c9d61af5d34}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\06. GuardianKnight">
      <UniqueIdentifier>{b653d5e3-89cd-4b68-a235-88f0129d94bd}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\01. Boss">
      <UniqueIdentifier>{2d44837b-7b11-42f4-9d69-d18a0e209670}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\01. Npc">
      <UniqueIdentifier>{52dc0b5e-f812-4835-a4c9-d7896396ef00}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\02. World">
      <UniqueIdentifier>{5f174343-f8c8-48ca-9e8a-611019312830}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\02. World\Map">
      <UniqueIdentifier>{7a44eaf0-e8d2-45cd-99f8-c9c5ddf4475c}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI">
      <UniqueIdentifier>{57537602-afc9-4af5-9022-c48584d4bcb1}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\00.LayoutInputAndDrawing">
      <UniqueIdentifier>{7593062b-9fd5-5136-a913-3c7b64a9308f}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\01.CombatHUD">
      <UniqueIdentifier>{24eb4109-dc95-5ad3-b1ec-d1f5c849708b}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\02.CharacterAndCustomizing">
      <UniqueIdentifier>{cf0ca5f8-934f-5fcd-8bcb-acd2df6d7ca9}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\03.InventoryAndEquipment">
      <UniqueIdentifier>{03aa5e55-4202-52d5-a4f5-a19e01f6e3b8}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\04.PartyChatAndNameplates">
      <UniqueIdentifier>{8d87b58a-e2b8-5cf2-8bf6-edbdc5c34698}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\05.WorldAndNavigation">
      <UniqueIdentifier>{88354c1c-d7a5-57dd-a381-81c5609dd775}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\06.MenuAndOptions">
      <UniqueIdentifier>{0b26318d-d54b-5085-a183-a168c7220b15}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\04. UI\07.RaidAndResults">
      <UniqueIdentifier>{dd634be6-5a9b-5fc1-9c22-a05f9ff24dab}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\06. Camera">
      <UniqueIdentifier>{6131ea7b-c294-4923-9bdf-06dc6c9a0bc7}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\06. Camera\Free">
      <UniqueIdentifier>{ab809c0d-7e47-4143-8399-63dabb6b67ae}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools">
      <UniqueIdentifier>{ba06c327-ea61-4e3a-8671-349b2cbe8ee1}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map">
      <UniqueIdentifier>{1f961324-6c44-4b0c-9079-b04d64f14300}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map\00.EditorAndWorkspace">
      <UniqueIdentifier>{0ab40396-c3ac-5645-88a0-66f98b2319b1}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map\01.WorldTools">
      <UniqueIdentifier>{140ed703-b3fb-5b20-a2eb-dd3d039920c4}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map\02.Navigation">
      <UniqueIdentifier>{007bd5e0-4b61-5a38-8197-7405f1c578c1}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map\03.Destruction">
      <UniqueIdentifier>{3b442227-9ba8-5739-89eb-2a13c442f953}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\00. Map\04.PublishAndResources">
      <UniqueIdentifier>{ae0b0f8b-762f-50fc-b748-42dcb0adea0c}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\01. Boss">
      <UniqueIdentifier>{4f3fa45e-b58d-4558-9c4b-9a0219ab36b2}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\01. Boss\00.ToolsAndAudition">
      <UniqueIdentifier>{0917fa17-2057-56a7-aecf-c8c05ce0c4a2}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\01. Boss\01.FlowAndDocuments">
      <UniqueIdentifier>{529511e3-2fc7-5075-aad8-8fd091fa253e}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect">
      <UniqueIdentifier>{adf379c4-e0ce-4567-964a-62e2e0b4807c}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect\00.Window">
      <UniqueIdentifier>{c35cd6d1-103f-5151-9624-5670f6efbaea}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect\01.Resources">
      <UniqueIdentifier>{00624b55-5122-530b-a316-4761fe6e8b41}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect\02.Editing">
      <UniqueIdentifier>{20bf08d6-8f3b-50b9-9058-c6b1e6c8bebe}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect\03.Preview">
      <UniqueIdentifier>{d4e6128e-276c-5907-9bd8-e60a5da6a63b}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\02. Effect\04.SaveLoad">
      <UniqueIdentifier>{3ec85f31-16fd-5d0e-ac22-176db0390ac6}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\03. UI">
      <UniqueIdentifier>{8a51f635-e0bc-45cf-88c5-524cac099261}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\04. Animation">
      <UniqueIdentifier>{42322243-9f4d-4daa-9a43-c0122697c314}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\04. Animation\00.Editor">
      <UniqueIdentifier>{2fc29c35-6797-5214-80e1-9ed26dc56b4f}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\04. Animation\01.TargetAndPreview">
      <UniqueIdentifier>{9b25bf2c-b029-5d9a-bbf8-d32396f5f663}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\04. Animation\02.BindingAndCueDocuments">
      <UniqueIdentifier>{635075be-e920-5ab6-ae85-134ac402b36a}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer">
      <UniqueIdentifier>{6f28d0c9-bbd3-42f8-9b77-040d9c18034e}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer\00.SessionAndTimeline">
      <UniqueIdentifier>{6dab9463-0961-5f2d-8030-c77a71188099}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer\01.Resources">
      <UniqueIdentifier>{8e073e22-adbf-5072-9951-6562ccd3341e}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer\02.Character">
      <UniqueIdentifier>{7bd16b33-3313-513e-b76d-894a7c9782ee}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer\03.Valtan">
      <UniqueIdentifier>{5a41d99f-56dd-51f9-a9d3-20a5315c9180}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\05. Sequencer\04.ShadowDocuments">
      <UniqueIdentifier>{e2d51b17-d0a7-523c-ac40-f351e1bff685}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\06. Effect V2">
      <UniqueIdentifier>{ca71a633-aed2-4aaf-82dc-975cd7e1ea9a}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\06. Effect V2\00.Window">
      <UniqueIdentifier>{51d4fe32-aad7-5c84-b731-90038d0c5b27}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\06. Effect V2\01.CompositionAndSession">
      <UniqueIdentifier>{1a9a7dc7-7488-5177-b00a-53c97f91a040}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\06. Effect V2\02.ResourcesAndTimeline">
      <UniqueIdentifier>{281d2cec-822c-502e-bb26-842d7f7176ee}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\06. Effect V2\03.Camera">
      <UniqueIdentifier>{296eacf3-db66-5d25-8ee6-c6d39253a8f6}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\07. Balance">
      <UniqueIdentifier>{2d1ffcb7-3167-4dbb-9a42-4e3de9f2c3a3}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\08. Camera">
      <UniqueIdentifier>{06d4acd2-79b2-40f7-ad78-e8edf14fafc8}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\09. Profiler">
      <UniqueIdentifier>{54438a9a-db59-4b03-b528-eed43569c670}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\10. Rendering">
      <UniqueIdentifier>{712047c8-13b6-47cf-9076-061b23819598}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\10. Rendering\Authoring">
      <UniqueIdentifier>{fee8a512-41fa-5f9d-a21b-cc9a3b04eadd}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\11. KoukuSaydon">
      <UniqueIdentifier>{66029b30-ef31-4639-b647-bd5ddb807f03}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument">
      <UniqueIdentifier>{7d0bb198-5fee-553e-b875-016f2e9ebf59}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition">
      <UniqueIdentifier>{3b414685-b51f-50ff-b3c2-a4435b4fb0ef}</UniqueIdentifier>
    </Filter>
    <Filter Include="03. Tools\12. AI">
      <UniqueIdentifier>{0707e02d-64d5-5280-99db-24fabc53a12e}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. Network">
      <UniqueIdentifier>{82310efc-790a-485f-b427-faf8083dc8e8}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. Network\00.Transport">
      <UniqueIdentifier>{52007484-be3b-56e5-9b5a-f7aeec338561}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. Network\01.Commands">
      <UniqueIdentifier>{8c130ead-367d-5d2c-8f2e-66612132e23e}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. Network\02.Replication">
      <UniqueIdentifier>{6ae88ca4-0629-5028-b155-5ec341ddac1f}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation">
      <UniqueIdentifier>{0f801293-33b1-5059-9a7e-0ee2dab133e6}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\00.Assets">
      <UniqueIdentifier>{768fec01-0d13-5484-ab88-d1d0a97ff9bd}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\00.Assets\Boss">
      <UniqueIdentifier>{8cd5818b-2b51-5df0-9cc4-06c0055e2846}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\00.Assets\CatalogsAndServices">
      <UniqueIdentifier>{d7c85061-a6d6-5d57-8aa2-f2456061b3a9}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\00.Assets\GameplayCatalogs">
      <UniqueIdentifier>{579b643c-638a-5d26-a5a7-06d906f123cd}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\00.Assets\GenerationAdmission">
      <UniqueIdentifier>{463e18a3-302b-536a-9728-f11de5faf644}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1">
      <UniqueIdentifier>{3329e9ed-2272-5ae8-ace3-2315ff08ff84}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\CatalogAndLoading">
      <UniqueIdentifier>{19194cfa-dab2-55c3-9a2c-69f9f24612f0}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\DocumentAndValidation">
      <UniqueIdentifier>{6a201b1d-3a3f-56ab-98dc-a42672871b6f}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\MaterialsAndPrograms">
      <UniqueIdentifier>{bf90f722-02c6-5768-95d2-c6f79841d8c5}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\PlaybackAndSimulation">
      <UniqueIdentifier>{691b2b68-18f0-5ea4-8ecb-3126d4193527}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\Renderer">
      <UniqueIdentifier>{468e3d0c-e2cb-56e4-aa97-848b96f733e8}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\Renderer\00.Renderer">
      <UniqueIdentifier>{2fd891fd-ac32-5b33-b396-d4f2b6427a44}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\Renderer\01.Preparation">
      <UniqueIdentifier>{e01aaa52-f4f8-54ff-8abe-a2465cd5dc0e}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\Renderer\02.MaterialBinding">
      <UniqueIdentifier>{cd0e1cb8-d469-5a51-98a4-1c0bee31a666}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\01.EffectsV1\Renderer\03.GeometryAndParticles">
      <UniqueIdentifier>{105e9427-31ec-58f8-a472-0ed6d6f1f488}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\02.EffectsV2">
      <UniqueIdentifier>{c5de3b8e-3592-5303-b229-d9f1052c07f3}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\03.ActionTimelineAndCues">
      <UniqueIdentifier>{5139eec2-a6a7-50ee-9824-59c70eeba708}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\03.ActionTimelineAndCues\Documents">
      <UniqueIdentifier>{efe85af0-6d72-5d11-b032-3db0e6a258bc}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\04.Cinematics">
      <UniqueIdentifier>{2a66b1d3-3866-5955-b14e-ce26cac7a9e0}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\05.Rendering">
      <UniqueIdentifier>{90ed0247-4570-555b-9cab-afc62d032ac1}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\05.Rendering\Materials">
      <UniqueIdentifier>{ce63c6b3-4c06-55aa-a532-97750e16d407}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\05.Rendering\Materials\CharacterAndEquipment">
      <UniqueIdentifier>{bd553e96-6dc9-56b6-aaab-814430d2cb1a}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\05.Rendering\Materials\Map">
      <UniqueIdentifier>{7f1899cd-75dd-5ab2-a9b9-c5e5563d3de5}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\05.Rendering\ProfilesAndLights">
      <UniqueIdentifier>{01f93a72-7f0c-5a90-adcb-4ea1e8589688}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\06.World">
      <UniqueIdentifier>{b49f0dfc-a5de-54a4-9ccc-984589e35153}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\06.World\DocumentsAndCatalogs">
      <UniqueIdentifier>{841a173a-92c5-5455-801f-1634576a90af}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\06.World\RuntimeAndLoading">
      <UniqueIdentifier>{1ed1ebe6-2ef1-5930-848c-17efb671862b}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Presentation\07.CombatAndBoss">
      <UniqueIdentifier>{efb09f93-8eeb-5859-8d40-2b9109cd10d4}</UniqueIdentifier>
    </Filter>
    <Filter Include="06.DataAccess">
      <UniqueIdentifier>{590c35fd-3707-5ee9-9bfc-af459f318733}</UniqueIdentifier>
    </Filter>
    <Filter Include="06.DataAccess\UserSettings">
      <UniqueIdentifier>{705a6cb1-809c-513d-8382-98a12e6b4c9e}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles">
      <UniqueIdentifier>{7dcb7381-90c8-40c3-8bdf-1d48e55d2e74}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\00.RenderPipeline">
      <UniqueIdentifier>{42b9771f-bf63-5744-bec4-961a0f783944}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials">
      <UniqueIdentifier>{009f08c2-34f8-57f4-8e7d-1ab6fce8da1b}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\Character">
      <UniqueIdentifier>{1548766d-b1ce-5645-b683-6d1fb95f2f42}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding">
      <UniqueIdentifier>{d54f829a-c0df-5f25-8959-6514f20bffad}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\Character\02.Lighting">
      <UniqueIdentifier>{7f7e950d-4164-54ef-9ad8-e2369d239b43}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\EquipmentAndSpecial">
      <UniqueIdentifier>{95ca6193-546f-5994-8360-5f2a5fd74f3d}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\MapAndWorld">
      <UniqueIdentifier>{eab1cbf6-733b-5632-8488-aef0c0990b53}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\ModelPasses">
      <UniqueIdentifier>{99aa09c2-f279-5545-810a-f92062baff37}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\01.Materials\ModelPasses\Variants">
      <UniqueIdentifier>{935fcfdb-657a-5501-83bf-0ebaddf1f0d3}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects">
      <UniqueIdentifier>{5950a983-1e63-52a4-b875-3bbbef651323}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\01.CarriersAndCommon">
      <UniqueIdentifier>{0332caa5-6c2c-5bdc-b2cc-e322358a117a}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native">
      <UniqueIdentifier>{6bd9db4a-b5a2-5f20-8e33-9868fac16fb7}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\Artist">
      <UniqueIdentifier>{5556ca20-2c50-520e-926d-974a6a99d4f0}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\Kouku">
      <UniqueIdentifier>{fb26ced1-8899-5d4a-be7a-c0396445c285}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\LanceMaster">
      <UniqueIdentifier>{ec283fcc-9ddc-5b7f-8379-c2bc69670d5f}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\Shared">
      <UniqueIdentifier>{1d89e9b8-dadc-5cd6-9067-3d9c60f23c96}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\Warlord">
      <UniqueIdentifier>{94709707-bb92-5adf-8658-3b5611a8f3f0}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\02.Native\World">
      <UniqueIdentifier>{b937c283-924c-5214-91d9-01f416394758}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\03.V2">
      <UniqueIdentifier>{758b9743-7ab8-524f-8589-3820b55e0912}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\05.TypedFamilies">
      <UniqueIdentifier>{4ef8e68f-d917-5263-8e25-f8b22b0f103d}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\05.TypedFamilies\Artist">
      <UniqueIdentifier>{8abf47be-21ec-54ee-9579-7912dd37932e}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster">
      <UniqueIdentifier>{836109c6-e7a3-5b9e-ab69-9a8135fe094f}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster">
      <UniqueIdentifier>{008566fe-ee87-5c30-90ce-65d86b0a84f3}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord">
      <UniqueIdentifier>{d86c3128-b53c-5800-a14f-6831c9e328ce}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\03.UIAndDebug">
      <UniqueIdentifier>{2898c921-e206-53d4-b3fb-48b6c6327b16}</UniqueIdentifier>
    </Filter>
    <Filter Include="97.ShaderFiles\99.Common">
      <UniqueIdentifier>{82ad56b3-57fe-5fe4-8c05-3feb3594b370}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Default">
      <UniqueIdentifier>{d335d9ac-def4-46b8-ac95-c186386f8b8a}</UniqueIdentifier>
    </Filter>
    <Filter Include="99.Defines">
      <UniqueIdentifier>{875044a5-ca30-4aac-9081-e3d4d5b065fe}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.GameObjects\00. Character\07. Customization">
      <UniqueIdentifier>{830a186f-d7ce-54f7-98ab-2dab7269fb9a}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\MainApp.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_Profiler.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_Dragon.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_ValtanAxe.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_SequenceViewer.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_WorldLevel.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\LevelRegistry.cpp">
      <Filter>01.Levels</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RenderingProfileService.cpp">
      <Filter>05.Presentation\05.Rendering\ProfilesAndLights</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AnimationTargetService.cpp">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterPreviewPanel.cpp">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EquipmentAuthoringTool.cpp">
      <Filter>03. Tools\03. UI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EquipmentPresentationCatalog.cpp">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EquipmentPresentationService.cpp">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayableCharacterPreviewContract.cpp">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AnimationAuthoringBridge.cpp">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AnimationSkillBindingDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonAnimationActionDocument.cpp">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonAnimationPatternDocument.cpp">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonCompositionDocument.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Animation.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Camera.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Clipboard.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Collider.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Document.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Helpers.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_History.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Logic.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Patterns.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Presentation.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Preview.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Publish.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Summon.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_Timeline.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonActionWorkbench_WorldScene.cpp">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClCompile>
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_CameraLookup.h">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_Internal.h">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\KoukuSaydonActionWorkbench_TimelineTypes.h">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\KoukuSaydonBossTool.cpp">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonPatternAuditionService.cpp">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternFlowDocument.cpp">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternFlowService.cpp">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanTuningCommandService.cpp">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternPreviewDocument.cpp">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternAuthoringEffectDocument.cpp">
      <Filter>03. Tools\02. Effect</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternEffectCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternEffectCueAuthoring.cpp">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternSoundCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanCombatObjectSoundCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternShakeCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BalanceTool.cpp">
      <Filter>03. Tools\07. Balance</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BalanceTestPanel.cpp">
      <Filter>03. Tools\07. Balance</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanBossTool.cpp">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CameraTool.cpp">
      <Filter>03. Tools\08. Camera</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\LobbyCommandService.cpp">
      <Filter>01.Levels</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\LevelTransitionService.cpp">
      <Filter>01.Levels</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_Development.cpp">
      <Filter>01.Levels\06. Development</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ColosseumMatchView.cpp">
      <Filter>01.Levels\06. Development</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ProfilerCaptureIO.cpp">
      <Filter>03. Tools\09. Profiler</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ProfilerTool.cpp">
      <Filter>03. Tools\09. Profiler</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RenderingBenchmark.cpp">
      <Filter>03. Tools\10. Rendering</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossCompositionDocument.cpp">
      <Filter>03. Tools\05. Sequencer\04.ShadowDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SequencerTool.cpp">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterActionWorkbench.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BoneAnimationDocument.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BoneAnimationWorkbench.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BoneAnimationWorkbench_Viewport.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterModelWorkbench.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterActionCombatDocument.cpp">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClCompile>
    <ClCompile Include="Client.cpp">
      <Filter>98.Default</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_Loading.cpp">
      <Filter>01.Levels\05. Loading</Filter>
    </ClCompile>
    <ClCompile Include="..\private\Loader.cpp">
      <Filter>01.Levels\05. Loading\Loader</Filter>
    </ClCompile>
    <ClCompile Include="..\private\Camera_Free.cpp">
      <Filter>02.GameObjects\06. Camera\Free</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CameraShakeService.cpp">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSequenceDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSequenceToolPanel.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldGameplayDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EncounterPatternReference.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionDocument.cpp">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DestructionSimulationDocument.cpp">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DestructionSimulationRuntime.cpp">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DestructionSimulationController.cpp">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SpawnGroupDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NavGridBaker.cpp">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NavGridPaintDocument.cpp">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NavRuntimeBlockerDocument.cpp">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AssetPreparationBatch.cpp">
      <Filter>01.Levels\05. Loading\Loader</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapAssetCatalog.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapEffectDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapEffectPresentationRuntime.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapLightDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapLightPresentationRuntime.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MaharakaWaterpangPresentation.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapEditorWorkspaceService.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapNavigationContract.cpp">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\LevelNavigationDebug.cpp">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DeployPropCatalog.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapPlacementDocument.cpp">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapPlacementRuntime.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapAuthoringHost.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapPublishRunner.cpp">
      <Filter>03. Tools\00. Map\04.PublishAndResources</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapPlacementEditSession.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapAssetObject.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DeployPropObject.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DeployPropRuntime.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSequencePlayer.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapAssetPreview.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Body_Valtan.cpp">
      <Filter>02.GameObjects\01. Boss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternTree.cpp">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanCinematicEffectLibrary.cpp">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPresentationGenerationAdmission.cpp">
      <Filter>05.Presentation\00.Assets\GenerationAdmission</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternAuditionService.cpp">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RuntimeAssetRoot.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ProjectDataRoot.cpp">
      <Filter>06.DataAccess</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UserSettingsDocument.cpp">
      <Filter>06.DataAccess\UserSettings</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientWindowDisplay.cpp">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SystemOptionRowsDocument.cpp">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DataJson.cpp">
      <Filter>06.DataAccess</Filter>
    </ClCompile>
    <ClCompile Include="..\private\Effect_Tool.cpp">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Distribution.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_CascadeCompiler.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_RuntimeAuthority.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Artist31470ShaderRegistry.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ReconstructedExecution.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Playback.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ParticleUpdatePool.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_LightPresentation.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\00.Renderer</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_MaterialProgramRegistry.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DirectAuthoredSourceIndex.cpp">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Object.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ScreenOverlayPresentation.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ThumbnailCache.cpp">
      <Filter>03. Tools\02. Effect\01.Resources</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Catalog.cpp">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectResourceCatalog.cpp">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_LoadPreparationJob.cpp">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_OccurrenceTuning.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_VisualProgramCorpus.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AnimationEffectCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SoundCueCatalog.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EstherActionSoundCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NpcActionEffectCueDocument.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench.cpp">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench_Details.cpp">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench_Preview.cpp">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClCompile>
    <ClInclude Include="..\Private\ValtanActionWorkbench_EditSupport.h">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\CompositionResourceTree.cpp">
      <Filter>03. Tools\05. Sequencer\01.Resources</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench_Blueprint.cpp">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanActionWorkbench_Timeline.cpp">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ActionCompositionGraphModel.cpp">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossLogicFlowViewModel.cpp">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossLogicFlowRenderer.cpp">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_PresentationService.cpp">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_ProductPrewarmQueue.cpp">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HUDLayoutTool.cpp">
      <Filter>03. Tools\03. UI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_V2.cpp">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectCompositionWorldResource.cpp">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectCompositionModelPreview.cpp">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectCompositionWorkbench.cpp">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectEditingSession.cpp">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringResourceTree.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Camera.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectRecoveryCamera.cpp">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RecoveryCameraAuthoringSession.cpp">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SequenceCameraEditor.cpp">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Timeline.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Resources.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Tracks.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringSequencer_Presentation.cpp">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectAuthoringV2Pane.cpp">
      <Filter>03. Tools\06. Effect V2\00.Window</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Workspace.cpp">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectV2_Object.cpp">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectV2_Catalog.cpp">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectV2_Document.cpp">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EffectV2_Runtime.cpp">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Valtan.cpp">
      <Filter>02.GameObjects\01. Boss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BoneChainSimulation.cpp">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\FaceCustomizeApplier.cpp">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\FaceMorphApplier.cpp">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\FaceSliderDocument.cpp">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Character.cpp">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HitAreaWire.cpp">
      <Filter>02.GameObjects\00. Character\03.CombatPresentation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SkeletalAfterimage.cpp">
      <Filter>02.GameObjects\00. Character\03.CombatPresentation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ActionPresentationTimeline.cpp">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanCinematicCameraController.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanCinematicCameraDocument.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionProjectionDocument.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionDebrisPresentationDocument.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionDebrisPresentationRuntime.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionProjectionRuntime.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Part_Body.cpp">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Part_Vehicle.cpp">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Part_Equipment.cpp">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DeferredMaterialRenderUtils.cpp">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapAssetRenderUtils.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticBatchObject.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticChunkObject.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapStaticProxyBaker.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_Lobby.cpp">
      <Filter>01.Levels\00. Lobby</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_Bern.cpp">
      <Filter>01.Levels\02. Bern</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_KakulSaydonArena.cpp">
      <Filter>01.Levels\04. KoukuSaydonArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_ValtanArena.cpp">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_LanceMaster.cpp">
      <Filter>02.GameObjects\00. Character\00. LanceMaster</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_GunSlinger.cpp">
      <Filter>02.GameObjects\00. Character\01. GunSlinger</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_Artist.cpp">
      <Filter>02.GameObjects\00. Character\02. Artist</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_DimensionMaster.cpp">
      <Filter>02.GameObjects\00. Character\04. DimensionMaster</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_Slayer.cpp">
      <Filter>02.GameObjects\00. Character\03. Slayer</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_Warlord.cpp">
      <Filter>02.GameObjects\00. Character\05. Warlord</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Logic_GuardianKnight.cpp">
      <Filter>02.GameObjects\00. Character\06. GuardianKnight</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Npc.cpp">
      <Filter>02.GameObjects\01. Npc</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NetworkManager.cpp">
      <Filter>04. Network\00.Transport</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientSessionDiagnostic.cpp">
      <Filter>04. Network\00.Transport</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NetObjectRegistry.cpp">
      <Filter>04. Network\02.Replication</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterCatalog.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ActorCatalog.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayableCharacterAssetService.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SquareHoleInstrumentCatalog.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientReplication.cpp">
      <Filter>04. Network\02.Replication</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientReplication_BattleItems.cpp">
      <Filter>04. Network\02.Replication</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientReplication_WaterGun.cpp">
      <Filter>04. Network\02.Replication</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CombatObjectProjectionRuntime.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayerController.cpp">
      <Filter>04. Network\01.Commands</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CombatHUDViewModel.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EstherCutinPresentationService.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RaidBossShowcaseService.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HUDRuntimeView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SkillWindowView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CustomizingCostumeDocument.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CustomizingFacePresetDocument.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CustomizingFaceTextureDocument.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CustomizingIconDocument.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CustomizingView.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterOutfitApplier.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DurabilityHudView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\InventoryView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RepairWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ShopWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\QuickSlotDragView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterInfoWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterPortraitRenderer.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\AvatarBookWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ChatWindowView.cpp">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PartyWindowView.cpp">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldPlayerNameplateView.cpp">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldHealthBarView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldPlayerChatBubbleView.cpp">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PartyInteractionView.cpp">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CombatAnalysisFrameView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\VehicleWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SystemOptionWindowView.cpp">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SystemOptionWindowView_Rows.cpp">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldMapWindowView.cpp">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SongCastGaugeView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HonorTitleWindowView.cpp">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RaidEntryPreviewView.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterSelectWindowView.cpp">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MinimapView.cpp">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuMadnessGaugeView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\StatusEffectTextView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MvpAwardCatalog.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MvpResultView.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\RaidGateProgressView.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SystemMenuButtonsView.cpp">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\InteractKeyPromptView.cpp">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UITextOcclusion.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\DungeonTimerView.cpp">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossImmuneGaugeView.cpp">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UITextureCache.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UILayoutRuntime.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UIInputRouter.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UIWindowDrag.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\UI_Sprite.cpp">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NetworkPlayerCommandSink.cpp">
      <Filter>04. Network\01.Commands</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterSelectionState.cpp">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterRoster.cpp">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CharacterSelectShowcase.cpp">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_CharacterSelect.cpp">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayerSkillCatalog.cpp">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SkillGroundTargetPreview.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClickMoveEffect.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ItemCatalog.cpp">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HonorTitleCatalog.cpp">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPresentationAssetService.cpp">
      <Filter>05.Presentation\00.Assets\Boss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NpcPlacementPresentationService.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NpcPresentationAssetService.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\VehiclePresentationAssetService.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MonsterPresentationAssetService.cpp">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonPresentationAssetService.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonPresentationPlayer.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonPresentationPlayer_LogicPreview.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuCinematicAnimationCatalog.cpp">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonPreviewRootMotion.cpp">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonAnimationBlend.cpp">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NetworkWorldEntityCommandSink.cpp">
      <Filter>04. Network\01.Commands</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Trigger_Box.cpp">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\MainApp.h">
      <Filter>00.MainApp</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ProfilerCaptureIO.h">
      <Filter>03. Tools\09. Profiler</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ProfilerTool.h">
      <Filter>03. Tools\09. Profiler</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RenderingBenchmark.h">
      <Filter>03. Tools\10. Rendering</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RenderingTechniqueGuide.h">
      <Filter>03. Tools\10. Rendering</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RenderingReferenceGuide.h">
      <Filter>03. Tools\10. Rendering</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BossCompositionDocument.h">
      <Filter>03. Tools\05. Sequencer\04.ShadowDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SequencerTool.h">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterActionWorkbench.h">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BoneAnimationDocument.h">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BoneAnimationWorkbench.h">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterModelWorkbench.h">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterActionCombatDocument.h">
      <Filter>03. Tools\05. Sequencer\02.Character</Filter>
    </ClInclude>
    <ClInclude Include="framework.h">
      <Filter>98.Default</Filter>
    </ClInclude>
    <ClInclude Include="Resource.h">
      <Filter>98.Default</Filter>
    </ClInclude>
    <ClInclude Include="targetver.h">
      <Filter>98.Default</Filter>
    </ClInclude>
    <ClInclude Include="Client.h">
      <Filter>98.Default</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Client_Defines.h">
      <Filter>99.Defines</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\LevelRegistry.h">
      <Filter>01.Levels</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RenderingProfileService.h">
      <Filter>05.Presentation\05.Rendering\ProfilesAndLights</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AnimationTargetService.h">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterPreviewPanel.h">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EquipmentAuthoringTool.h">
      <Filter>03. Tools\03. UI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EquipmentPresentationCatalog.h">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EquipmentPresentationService.h">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayableCharacterPreviewContract.h">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AnimationAuthoringBridge.h">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\LobbyCommandService.h">
      <Filter>01.Levels</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\LevelTransitionService.h">
      <Filter>01.Levels</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_Development.h">
      <Filter>01.Levels\06. Development</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumLoadingView.h">
      <Filter>01.Levels\06. Development</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumMatchView.h">
      <Filter>01.Levels\06. Development</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumIntroCutscene.h">
      <Filter>01.Levels\06. Development</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumMatchStart.h">
      <Filter>01.Levels\06. Development</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_Loading.h">
      <Filter>01.Levels\05. Loading</Filter>
    </ClInclude>
    <ClInclude Include="..\public\Loader.h">
      <Filter>01.Levels\05. Loading\Loader</Filter>
    </ClInclude>
    <ClInclude Include="..\public\Camera_Free.h">
      <Filter>02.GameObjects\06. Camera\Free</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CameraShakeService.h">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapTool.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldSequenceDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldSequenceToolPanel.h">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldGameplayDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EncounterPatternReference.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionDocument.h">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DestructionSimulationDocument.h">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DestructionSimulationRuntime.h">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DestructionSimulationController.h">
      <Filter>03. Tools\00. Map\03.Destruction</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SpawnGroupDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NavGridBaker.h">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NavGridPaintDocument.h">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NavRuntimeBlockerDocument.h">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AssetPreparationBatch.h">
      <Filter>01.Levels\05. Loading\Loader</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapAssetCatalog.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapEffectDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapEffectPresentationRuntime.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapLightDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapLightPresentationRuntime.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MaharakaWaterpangPresentation.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapEditorWorkspaceService.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapNavigationContract.h">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\LevelNavigationDebug.h">
      <Filter>03. Tools\00. Map\02.Navigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DeployPropCatalog.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapPlacementDocument.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapPlacementRuntime.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapAuthoringHost.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapPublishRunner.h">
      <Filter>03. Tools\00. Map\04.PublishAndResources</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapPlacementEditSession.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapLoadScope.h">
      <Filter>05.Presentation\06.World\DocumentsAndCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapAssetObject.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DeployPropObject.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DeployPropRuntime.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldSequencePlayer.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapAssetPreview.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Body_Valtan.h">
      <Filter>02.GameObjects\01. Boss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternTree.h">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanCinematicEffectLibrary.h">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanViewAdmission.h">
      <Filter>05.Presentation\00.Assets\GenerationAdmission</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPresentationGenerationAdmission.h">
      <Filter>05.Presentation\00.Assets\GenerationAdmission</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternAuditionService.h">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RuntimeAssetRoot.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ProjectDataRoot.h">
      <Filter>06.DataAccess</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UserSettingsDocument.h">
      <Filter>06.DataAccess\UserSettings</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClientWindowDisplay.h">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SystemOptionRowsDocument.h">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DataJson.h">
      <Filter>06.DataAccess</Filter>
    </ClInclude>
    <ClInclude Include="..\public\Effect_Tool.h">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Animation_Tool.h">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanActionWorkbench.h">
      <Filter>03. Tools\05. Sequencer\03.Valtan</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CompositionResourceTree.h">
      <Filter>03. Tools\05. Sequencer\01.Resources</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CompositionAnimationResource.h">
      <Filter>03. Tools\05. Sequencer\01.Resources</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CompositionWorkbenchSession.h">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CompositionEditing.h">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CompositionTimeline.h">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ActionCompositionGraphModel.h">
      <Filter>03. Tools\05. Sequencer\00.SessionAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BossLogicFlowView.h">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AnimationSkillBindingDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonAnimationActionDocument.h">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonAnimationPatternDocument.h">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonCompositionDocument.h">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonActionWorkbench.h">
      <Filter>03. Tools\11. KoukuSaydon\00.WorkbenchAndDocument</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonBossTool.h">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonPatternAuditionService.h">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternFlowDocument.h">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternFlowService.h">
      <Filter>03. Tools\01. Boss\01.FlowAndDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanTuningCommandService.h">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternPreviewDocument.h">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternAuthoringEffectDocument.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternEffectCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternEffectCueAuthoring.h">
      <Filter>03. Tools\04. Animation\02.BindingAndCueDocuments</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternSoundCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanCombatObjectSoundCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPatternShakeCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues\Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BalanceTool.h">
      <Filter>03. Tools\07. Balance</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BalanceTestPanel.h">
      <Filter>03. Tools\07. Balance</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanBossTool.h">
      <Filter>03. Tools\01. Boss\00.ToolsAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CameraTool.h">
      <Filter>03. Tools\08. Camera</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AnimationPreviewAssets.h">
      <Filter>03. Tools\04. Animation\01.TargetAndPreview</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HUDLayoutTool.h">
      <Filter>03. Tools\03. UI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Tool_V2.h">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectCompositionWorldResource.h">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectCompositionModelPreview.h">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectCompositionWorkbench.h">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectEditingSession.h">
      <Filter>03. Tools\06. Effect V2\01.CompositionAndSession</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectAuthoringResourceTree.h">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectAuthoringSequencer.h">
      <Filter>03. Tools\06. Effect V2\02.ResourcesAndTimeline</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectRecoveryCamera.h">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RecoveryCameraAuthoringSession.h">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SequenceCameraEditor.h">
      <Filter>03. Tools\06. Effect V2\03.Camera</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectAuthoringV2Pane.h">
      <Filter>03. Tools\06. Effect V2\00.Window</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectV2_Object.h">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectV2_Catalog.h">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectV2_Document.h">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectV2_Runtime.h">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectV2_Target.h">
      <Filter>05.Presentation\02.EffectsV2</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Valtan.h">
      <Filter>02.GameObjects\01. Boss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BoneChainSimulation.h">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\FaceCustomizeApplier.h">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\FaceMorphApplier.h">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\FaceSliderDocument.h">
      <Filter>02.GameObjects\00. Character\07. Customization</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Character.h">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HitAreaWire.h">
      <Filter>02.GameObjects\00. Character\03.CombatPresentation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SkeletalAfterimage.h">
      <Filter>02.GameObjects\00. Character\03.CombatPresentation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ActionPresentationTimeline.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanCinematicCameraController.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanCinematicCameraDocument.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionProjectionDocument.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionDebrisPresentationDocument.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionDebrisPresentationRuntime.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionProjectionRuntime.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterSpec.h">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Part_Body.h">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Part_Vehicle.h">
      <Filter>02.GameObjects\00. Character\00.ActorAndParts</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Part_Equipment.h">
      <Filter>02.GameObjects\00. Character\01.Equipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceCharacterMaterialParameters.h">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceEquipmentMaterialPrograms.h">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceMovieMaterialPrograms.h">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceMapForwardMaterialParameters.h">
      <Filter>05.Presentation\05.Rendering\Materials\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceMapWaterMaterialParameters.h">
      <Filter>05.Presentation\05.Rendering\Materials\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SourceMapTranslucentMaterialParameters.h">
      <Filter>05.Presentation\05.Rendering\Materials\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DeferredMaterialRenderUtils.h">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapAssetRenderUtils.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapStaticBatchObject.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapStaticChunkObject.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MapStaticProxyBaker.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_Lobby.h">
      <Filter>01.Levels\00. Lobby</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_Bern.h">
      <Filter>01.Levels\02. Bern</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_KakulSaydonArena.h">
      <Filter>01.Levels\04. KoukuSaydonArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_ValtanArena.h">
      <Filter>01.Levels\04. ValtanArena</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_LanceMaster.h">
      <Filter>02.GameObjects\00. Character\00. LanceMaster</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_GunSlinger.h">
      <Filter>02.GameObjects\00. Character\01. GunSlinger</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_Artist.h">
      <Filter>02.GameObjects\00. Character\02. Artist</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_DimensionMaster.h">
      <Filter>02.GameObjects\00. Character\04. DimensionMaster</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_Slayer.h">
      <Filter>02.GameObjects\00. Character\03. Slayer</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_Warlord.h">
      <Filter>02.GameObjects\00. Character\05. Warlord</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Logic_GuardianKnight.h">
      <Filter>02.GameObjects\00. Character\06. GuardianKnight</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Npc.h">
      <Filter>02.GameObjects\01. Npc</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NetworkManager.h">
      <Filter>04. Network\00.Transport</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClientSessionDiagnostic.h">
      <Filter>04. Network\00.Transport</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClientReplicationEvent.h">
      <Filter>04. Network\02.Replication</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NetObjectRegistry.h">
      <Filter>04. Network\02.Replication</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterCatalog.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ActorCatalog.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayableCharacterAssetService.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SquareHoleInstrumentCatalog.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClientReplication.h">
      <Filter>04. Network\02.Replication</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CombatDebugVisibility.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayerHandGripTransform.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CombatObjectProjectionRuntime.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\LocalMovePrediction.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayerController.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CombatHUDViewModel.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EstherCutinPresentationService.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RaidBossShowcaseService.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HUDRuntimeView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SkillWindowView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CustomizingCostumeDocument.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CustomizingFacePresetDocument.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CustomizingFaceTextureDocument.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CustomizingIconDocument.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CustomizingView.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterOutfitApplier.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DurabilityHudView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\InventoryView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RepairWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ShopWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\QuickSlotDragView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterInfoWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterPortraitRenderer.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AvatarBookWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UILabelFont.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ChatWindowView.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PartyWindowView.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ReplicatedPlayerHealth.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PartyTransferNotice.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldPlayerNameplateView.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldHealthBarView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldPlayerChatBubbleView.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PartyInteractionView.h">
      <Filter>02.GameObjects\04. UI\04.PartyChatAndNameplates</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CombatAnalysisFrameView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\VehicleWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SystemOptionWindowView.h">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SystemOptionWindowView_Internal.h">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldMapWindowView.h">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SongCastGaugeView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HonorTitleWindowView.h">
      <Filter>02.GameObjects\04. UI\03.InventoryAndEquipment</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RaidEntryPreviewView.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterSelectWindowView.h">
      <Filter>02.GameObjects\04. UI\02.CharacterAndCustomizing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MinimapView.h">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuMadnessGaugeView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\StatusEffectTextView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MvpAwardCatalog.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MvpResultView.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RaidGateProgressView.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SystemMenuButtonsView.h">
      <Filter>02.GameObjects\04. UI\06.MenuAndOptions</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\InteractKeyPromptView.h">
      <Filter>02.GameObjects\04. UI\05.WorldAndNavigation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UITextOcclusion.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\DungeonTimerView.h">
      <Filter>02.GameObjects\04. UI\07.RaidAndResults</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BossImmuneGaugeView.h">
      <Filter>02.GameObjects\04. UI\01.CombatHUD</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UITextureCache.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UILayoutRuntime.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UIInputRouter.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UIWindowDrag.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\UI_Sprite.h">
      <Filter>02.GameObjects\04. UI\00.LayoutInputAndDrawing</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayerCommandSink.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NetworkPlayerCommandSink.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterSelectionState.h">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterRoster.h">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\CharacterSelectArenaSpawnGate.h">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CharacterSelectShowcase.h">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Level_CharacterSelect.h">
      <Filter>01.Levels\01. CharacterSelect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayerSkillCatalog.h">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SkillGroundTargetPreview.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClickMoveEffect.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ItemCatalog.h">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HonorTitleCatalog.h">
      <Filter>05.Presentation\00.Assets\GameplayCatalogs</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_CascadeCompiler.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_RuntimeAuthority.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Artist31470ShaderRegistry.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ReconstructedExecution.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Generated\Effect_SourceContractRegistry.generated.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ComponentDocument.h">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Distribution.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_MaterialTemplate.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DimensionMasterQMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DimensionMasterALTVMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DimensionMasterVMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanPresentationAssetService.h">
      <Filter>05.Presentation\00.Assets\Boss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NpcPlacementPresentationService.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NpcPresentationAssetService.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\VehiclePresentationAssetService.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MonsterPresentationAssetService.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonPresentationAssetService.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonPresentationPlayer.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuCinematicAnimationCatalog.h">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonPreviewRootMotion.h">
      <Filter>03. Tools\11. KoukuSaydon\01.BossPreviewAndAudition</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonAnimationBlend.h">
      <Filter>05.Presentation\07.CombatAndBoss</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MonsterPresentationContract.h">
      <Filter>05.Presentation\00.Assets\CatalogsAndServices</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldEntityCommandSink.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NetworkWorldEntityCommandSink.h">
      <Filter>04. Network\01.Commands</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Trigger_Box.h">
      <Filter>02.GameObjects\02. World\Map</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DocumentCodec.h">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Playback.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ParticleUpdatePool.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_LightPresentation.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DocumentRenderer.h">
      <Filter>05.Presentation\01.EffectsV1\Renderer\00.Renderer</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_MaterialProgramRegistry.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_DirectAuthoredSourceIndex.h">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Object.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ScreenOverlayPresentation.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ThumbnailCache.h">
      <Filter>03. Tools\02. Effect\01.Resources</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_Catalog.h">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectResourceCatalog.h">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_LoadPreparationJob.h">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_OccurrenceTuning.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_VisualProgramCorpus.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectAuthoringTransfer.h">
      <Filter>03. Tools\05. Sequencer\01.Resources</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SoundCueCatalog.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EstherActionSoundCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NpcActionEffectCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\AnimationEffectCueDocument.h">
      <Filter>05.Presentation\03.ActionTimelineAndCues</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EffectFailureDiagnostic.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_PresentationService.h">
      <Filter>05.Presentation\01.EffectsV1\PlaybackAndSimulation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_ProductPrewarmQueue.h">
      <Filter>05.Presentation\01.EffectsV1\CatalogAndLoading</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <Image Include="Client.ico">
      <Filter>98.Default</Filter>
    </Image>
    <Image Include="small.ico">
      <Filter>98.Default</Filter>
    </Image>
    <Image Include="cursor.cur">
      <Filter>98.Default</Filter>
    </Image>
  </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="Client.rc">
      <Filter>98.Default</Filter>
    </ResourceCompile>
  </ItemGroup>
  <ItemGroup>
    <None Include="Client.vcxproj">
      <Filter>98.Default</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxTex.hlsl">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshPreview.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMesh.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEstherNpc.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapInstance.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapChunk.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapProxy.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxSkillGroundTargetPreview.hlsl">
      <Filter>97.ShaderFiles\03.UIAndDebug</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticle.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectTrail.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectDecal.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectAnimMeshV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectMeshV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectRectV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectParticleV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectMeshParticleV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectDecalV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_EffectTrailV2.hlsl">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshPreview_V2.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Diagnostic.hlsli">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_MapMaterialSurface.hlsli">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapForwardPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapWaterPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapTranslucentPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapDirectPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceStoneSurface.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceSpecialSurface.hlsli">
      <Filter>97.ShaderFiles\01.Materials\EquipmentAndSpecial</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceLandscapeSurface.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceFoliageWind.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMaharakaStandWind.hlsli">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceFoliageSurface.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectCommon.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectUe3MaterialFamilies.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterQNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterVNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeSampleScene.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeColorResponse.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectSliceSceneDepth.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectRuntimeMaterialPacket.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectStandardColorV1.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLocalDecalAdapter.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDecalReceiver.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Ue3Glasshole02.hlsli">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Ue3Glasshole02Translation.hlsl">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\ClassSelectionTimeline.h">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClassMovieInspection.h">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClassMovieInspector.h">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\ClassMovieInspector.cpp">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClassSelectionPresentation_Inspection.cpp">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\ClassSelectionPresentation.h">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\ClassSelectionPresentation.cpp">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClassSelectionPresentation_Authoring.cpp">
      <Filter>05.Presentation\04.Cinematics</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\ArenaCameraProfile.h">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\ArenaCameraProfile.cpp">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\LightResourceCatalog.h">
      <Filter>05.Presentation\05.Rendering\ProfilesAndLights</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\LightResourceCatalog.cpp">
      <Filter>05.Presentation\05.Rendering\ProfilesAndLights</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\AreaLightAuthoringSession.h">
      <Filter>03. Tools\10. Rendering\Authoring</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\AreaLightAuthoringSession.cpp">
      <Filter>03. Tools\10. Rendering\Authoring</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MainApp_RenderingLighting.cpp">
      <Filter>00.MainApp</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\WorldSequenceObject.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSequencePlayer_Objects.cpp">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Level_KakulSaydonArena_WorldObjects.cpp">
      <Filter>01.Levels\04. KoukuSaydonArena</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldObjectTool.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldLevelHierarchy.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldLevelTool.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSceneTool.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldSceneTool_Animation.cpp">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\WorldSceneTool.h">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\PhysicalResourceCatalog.cpp">
      <Filter>03. Tools\00. Map\04.PublishAndResources</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\WorldSequenceObject.h">
      <Filter>05.Presentation\06.World\RuntimeAndLoading</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldObjectTool.h">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EditorUndoHistory.h">
      <Filter>03. Tools</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldLevelHierarchy.h">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldLevelTool.h">
      <Filter>03. Tools\00. Map\01.WorldTools</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PhysicalResourceCatalog.h">
      <Filter>03. Tools\00. Map\04.PublishAndResources</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_DimensionMasterWRMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterWRNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_DimensionMasterSDMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterSDNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_ArtistMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_WarlordNativeMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_NativeScreenPostMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\Effect_NativeScreenPostMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeInputs.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeCommon.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativePrograms.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup1600.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup1664.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2304.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2368.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2432.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2496.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2560.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2624.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2688.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2752.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2816.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2880.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup2944.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3008.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3072.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3136.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3200.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3264.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3328.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3584.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3648.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3712.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3776.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup448.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup512.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup768.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup832.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectNativeScreenPost.hlsl">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_LanceMasterVAMaterial.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Effect_ShaderFamily.h">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClInclude>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterQ.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterV.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterWR.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterSD.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV064.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV128.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshDimensionMasterALTV192.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist448.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist512.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist768.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist832.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster512.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster576.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster640.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster704.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster768.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterQ.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterV.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterWR.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterSD.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV064.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV128.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleDimensionMasterALTV192.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\DimensionMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist448.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist512.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist768.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist832.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster512.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster576.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster640.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster704.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster768.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Engine_Shader_Defines.hlsli">
      <Filter>97.ShaderFiles\99.Common</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active003RibbonMaterial.hlsli">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active011OuterMaterial.hlsli">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470Active022DecalMaterial.hlsli">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Artist31470RuntimeMaterial.hlsli">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup448.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup512.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup768.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup832.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup064.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup128.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeGroup192.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterVNativeShared.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup512.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup576.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup640.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup704.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup768.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectMeshFamilyCarrier.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectParticleFamilyCarrier.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectV2_Common.hlsli">
      <Filter>97.ShaderFiles\02.Effects\03.V2</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterMaterial.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapCharacterSelectDirect.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapSL10Forward.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapSL10Direct.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceMapCharacterSelectPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterForward.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectNativeScreenUV.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist1600.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshArtist1664.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1152.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1216.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1280.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshLanceMaster1344.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1024.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1088.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1152.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord1984.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshWarlord960.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist1600.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleArtist1664.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Artist</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1152.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1216.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1280.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleLanceMaster1344.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\LanceMaster</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1024.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1088.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1152.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord1984.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleWarlord960.hlsl">
      <Filter>97.ShaderFiles\02.Effects\05.TypedFamilies\Warlord</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup1600.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeGroup1664.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1152.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1216.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1280.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeGroup1344.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1024.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1088.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1152.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup1984.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup960.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_StaticShadowMap.hlsli">
      <Filter>97.ShaderFiles\01.Materials\MapAndWorld</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SceneHeightFog.hlsli">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2304.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2304.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2304.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWorldNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2368.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2432.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2560.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2688.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2752.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2816.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2880.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2944.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3008.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3072.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3136.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3200.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3264.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3328.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3584.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2368.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2432.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2560.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2688.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2752.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2816.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2880.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2944.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3008.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3072.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3136.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3200.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3264.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3328.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3584.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2368.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2432.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2560.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2688.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2752.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2816.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2880.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2944.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3008.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3072.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3136.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3200.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3264.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3328.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3584.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Client_Pch.cpp">
      <Filter>98.Default</Filter>
    </ClCompile>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h">
      <Filter>98.Default</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Effect_Tool_Internal.h">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\Effect_Tool_ResourceBrowser.cpp">
      <Filter>03. Tools\02. Effect\01.Resources</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Detail.cpp">
      <Filter>03. Tools\02. Effect\02.Editing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_MaterialDetail.cpp">
      <Filter>03. Tools\02. Effect\02.Editing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_CatalogPreview.cpp">
      <Filter>03. Tools\02. Effect\01.Resources</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Valtan.cpp">
      <Filter>03. Tools\02. Effect\03.Preview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_ValtanWorld.cpp">
      <Filter>03. Tools\02. Effect\03.Preview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Editing.cpp">
      <Filter>03. Tools\02. Effect\02.Editing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_DocumentIo.cpp">
      <Filter>03. Tools\02. Effect\04.SaveLoad</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Playback.cpp">
      <Filter>03. Tools\02. Effect\03.Preview</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_OccurrenceAuthoring.cpp">
      <Filter>03. Tools\02. Effect\02.Editing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_TransformHistory.cpp">
      <Filter>03. Tools\02. Effect\02.Editing</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_Tool_Helpers.cpp">
      <Filter>03. Tools\02. Effect\00.Window</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Effect_DocumentRenderer_Internal.h">
      <Filter>05.Presentation\01.EffectsV1\Renderer\00.Renderer</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_ResourceStaging.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\01.Preparation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_PreparedDocument.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\01.Preparation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Catalog.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\01.Preparation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Staging.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\01.Preparation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_MaterialBinding.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\02.MaterialBinding</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Geometry.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\03.GeometryAndParticles</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Particles.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\03.GeometryAndParticles</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_Rendering.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\00.Renderer</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_MaterialHelpers.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\02.MaterialBinding</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_GeometryHelpers.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\03.GeometryAndParticles</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentRenderer_CacheHelpers.cpp">
      <Filter>05.Presentation\01.EffectsV1\Renderer\01.Preparation</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase448.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase512.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase768.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchBase832.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchAdditionalArtistCases1600.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchAdditionalArtistCases1664.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Artist</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchWorldNativeCases2304.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\World</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2304.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2368.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2432.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2560.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3584.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2688.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2816.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2880.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2944.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3008.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3072.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3136.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3200.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3264.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3328.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase64.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase128.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectDimensionMasterALTVNativeDispatchBase192.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase512.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase576.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase640.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase704.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase768.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1152.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1216.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1280.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectLanceMasterVANativeDispatchBase1344.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\LanceMaster</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase0.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase960.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1024.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1088.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1152.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeDispatchBase1984.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBasePrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightPrograms.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\MapTool_Internal.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\MapTool_NavigationResources.h">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\MapTool_DestructionAuthoring.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_WorldGameplay.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Cutscenes.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_WorldGameplayPanels.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Navigation.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Area.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_NavigationPanels.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Placements.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_DestructionPanels.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_CameraShots.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_NavigationBake.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MapTool_Helpers.cpp">
      <Filter>03. Tools\00. Map\00.EditorAndWorkspace</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\Animation_Tool_Internal.h">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\Animation_Tool_ValtanPlayback.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_KoukuPlayback.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_ValtanComposition.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_PatternAuthoring.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_CompositionSounds.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_DocumentIo.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_ValtanInspector.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_ValtanPanels.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_KoukuAuthoring.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_SkillBindings.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Animation_Tool_Helpers.cpp">
      <Filter>03. Tools\04. Animation\00.Editor</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\SourceCharacterMaterialParameters.cpp">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </ClCompile>
    <None Include="..\Private\SourceCharacterMaterialParameters_Generated.inl">
      <Filter>05.Presentation\05.Rendering\Materials\CharacterAndEquipment</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_ArtistMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_ArtistMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_LanceMasterVAMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_LanceMasterVAMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_WarlordNativeMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_WarlordNativeMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_DimensionMasterALTVMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_DimensionMasterALTVMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_DimensionMasterQMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_DimensionMasterQMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_DimensionMasterVMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_DimensionMasterVMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_DimensionMasterWRMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_DimensionMasterWRMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <ClCompile Include="..\Private\Effect_DimensionMasterSDMaterial.cpp">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </ClCompile>
    <None Include="..\Private\Effect_DimensionMasterSDMaterial_Tables.inl">
      <Filter>05.Presentation\01.EffectsV1\MaterialsAndPrograms</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectWarlordNativeGroup000.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Warlord</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Effect_DocumentCodec_Artist31470.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_AuthoredElements.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_AuthoringOverrides.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_DetailIo.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_JsonPrimitives.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_MaterialIo.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_MaterialValidation.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_PortableRuntime.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_RuntimeValidation.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_SourceRecipeIo.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_SourceValidation.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Effect_DocumentCodec_Validation.cpp">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClCompile>
    <ClInclude Include="..\Private\Effect_DocumentCodec_Internal.h">
      <Filter>05.Presentation\01.EffectsV1\DocumentAndValidation</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup001.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup009.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup017.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup025.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup080.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup084.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup160.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup176.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup192.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup208.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup235.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup214.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup224.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup237.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1136.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1120.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1104.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup108.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup096.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1472.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1408.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1344.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1152.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup1088.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup896.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup768.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup640.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterBaseGroup576.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\01.SurfaceAndBinding</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup001.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup009.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup017.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup025.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup080.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup084.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup160.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup176.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup192.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup208.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup235.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup214.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup224.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup237.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1136.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1120.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1104.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup108.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup096.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1472.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1408.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1344.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1152.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup1088.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup896.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup768.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup640.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_SourceCharacterLightGroup576.hlsli">
      <Filter>97.ShaderFiles\01.Materials\Character\02.Lighting</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup001.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup009.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup017.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup025.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup080.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup084.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup160.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup176.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup192.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup208.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup235.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup214.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup224.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup237.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1136.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1120.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1104.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup108.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup096.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1472.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1408.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1344.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1152.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup1088.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup896.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup768.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup640.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_Deferred_SourceGroup576.hlsl">
      <Filter>97.ShaderFiles\00.RenderPipeline</Filter>
    </None>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup001.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup009.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup017.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup025.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup080.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup084.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup160.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup176.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup192.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup208.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup235.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup214.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup224.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup237.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1136.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1120.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1104.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup108.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup096.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1472.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1408.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1344.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1152.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup1088.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup896.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup768.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup640.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxAnimMeshBinary_SourceGroup576.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup001.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup009.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup017.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup025.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup080.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup084.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup160.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup176.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup192.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup208.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup235.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup214.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup224.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup237.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1136.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1120.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1104.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup108.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup096.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1472.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1408.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1344.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1152.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup1088.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup896.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup768.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup640.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshBinary_SourceGroup576.hlsl">
      <Filter>97.ShaderFiles\01.Materials\ModelPasses\Variants</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectSceneColorInput.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectSceneDepthInput.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3712.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3776.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3712.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3776.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3712.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3776.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3712.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3776.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\Bin\ShaderFiles\Shader_EffectVehicleModelNative.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Shared</Filter>
    </None>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2496.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2496.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2496.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2496.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku2624.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup2624.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases2624.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3648.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3648.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3648.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3648.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku2624.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3840.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3840.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3840.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3840.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3840.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3904.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3904.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3904.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3904.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3904.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <Manifest Include="Client.manifest">
      <Filter>98.Default</Filter>
    </Manifest>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4480.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4480.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4480.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4480.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4480.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku3968.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4032.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4096.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4160.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4224.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4288.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku3968.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4032.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4096.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4160.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4224.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4288.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup3968.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4032.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4096.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4160.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4224.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4288.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases3968.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4032.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4096.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4160.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4224.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4288.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup3968.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4032.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4096.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4160.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4224.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4288.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4352.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4352.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4352.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4352.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4416.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4416.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4416.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4416.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4416.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4544.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4544.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4544.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4544.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4544.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4352.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4608.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4608.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4608.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4608.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4608.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4672.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4672.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4672.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4672.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4672.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4736.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4736.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4736.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4736.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4736.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4800.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4800.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4800.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4800.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4800.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4864.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4928.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4864.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4928.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4864.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4928.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4864.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4928.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4864.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4928.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku4992.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku4992.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup4992.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases4992.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup4992.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5056.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5056.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5120.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5056.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5120.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5056.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5120.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5056.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5120.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5120.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\GuideAIDocument.h">
      <Filter>03. Tools\12. AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\GuideAITool.h">
      <Filter>03. Tools\12. AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MaharakaAITool.h">
      <Filter>03. Tools\12. AI</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\MaharakaAITool.cpp">
      <Filter>03. Tools\12. AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GuideAIDocument.cpp">
      <Filter>03. Tools\12. AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GuideAITool.cpp">
      <Filter>03. Tools\12. AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GuideAITool_Combat.cpp">
      <Filter>03. Tools\12. AI</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5184.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5184.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5184.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5184.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5184.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5248.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5248.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5248.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5248.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5248.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshKouku5312.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectParticleKouku5312.hlsl">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </FxCompile>
    <None Include="..\Bin\ShaderFiles\Shader_EffectKoukuNativeGroup5312.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeDispatchKoukuNativeCases5312.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
    <None Include="..\Bin\ShaderFiles\Shader_EffectArtistNativeSelectedGroup5312.hlsli">
      <Filter>97.ShaderFiles\02.Effects\02.Native\Kouku</Filter>
    </None>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\MapGpuScene.h"><Filter>02.GameObjects\02. World\Map</Filter></ClInclude>
    <ClCompile Include="..\Private\MapGpuScene.cpp"><Filter>02.GameObjects\02. World\Map</Filter></ClCompile>
  </ItemGroup>
</Project>

```

## 생성 도구와 계약 검사 이관 완전 블록

## 도구의 Data 솔루션 등록 이관 전체 변경 블록

Data 원본 생성·shader 내용은 변경하지 않는다. 등록 helper가 반환하는 `(Path, before, after)` 후보를 각 기존 freshness·backup·write·rollback 경계에 연결한다. 사용자 UI, 생성기 main, publish는 실행하지 않는다.

### Tools/EffectPipeline/build_battle_item_effects.py

모듈 import/상수 범위:

```python
"""Recover four retail battle-item effects through the existing World carriers.

Item/SkillEffect/Projectile and ParticleSoundNew identities are recorded in the
receipt; visual inputs come from the original Cascade/CDO/material closure.
Candidates remain in out until explicit installation. Existing authored edits
are never overwritten by a repeat installation.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import os
import shutil
import sys
import tempfile

import build_gate3_world_auras as world
import build_kouku_pattern_native as native

ROOT = world.ROOT
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
OUT = ROOT / 'out/BattleItemEffects20260929'
TARGETS = {
    'effect.world.item.destruction_bomb':
        ('fx_cm_01.explosion.par_l_steelbomb_exp_01_loc_int', '파괴 폭탄'),
    'effect.world.item.whirlwind_grenade':
        ('fx_cm_01.explosion.par_l_battle_wind_bomb_01', '회오리 수류탄'),
    'effect.world.item.holy_charm':
        ('fx_bs_02.item.par_k_bi_holyamul_01', '성스러운 부적'),
    'effect.world.item.time_stop':
        ('fx_bs_04.state.par_k_timepause_02_l', '시간 정지 물약'),
    'effect.world.item.destruction_bomb.flight':
        ('fx_bs_05.prj.par_l_steelbomb_prj_01', '파괴 폭탄 · 비행'),
    'effect.world.item.whirlwind_grenade.flight':
        ('fx_bs_05.prj.par_l_battle_wind_prj_01', '회오리 수류탄 · 비행'),
}

# Original Projectile 32140/32310 Grenade resource ResScale. This belongs to
# the flight particle-system resource, independently of its StartSize curves
# and the mesh cm-to-m pre-scale; the replicated projectile root stays at 1.
FLIGHT_RESOURCE_SCALES = {
    'effect.world.item.destruction_bomb.flight': 1.1,
    'effect.world.item.whirlwind_grenade.flight': 1.2,
}


```

변경 함수 전체:

```python
def install():
    source = world.source
    contract = source.read(OUT / 'native/merged_native_runtime_contract.json')
    assert not contract.get('deferredPrograms')
    pending = []
    for asset in TARGETS:
        path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        payload = (OUT / 'candidate' / path.name).read_bytes()
        previous = path.read_bytes() if path.exists() else None
        assert previous is None or previous == payload, ('Preserve existing authored edit', path)
        pending.append((path, previous, payload))
    for path in (OUT / 'Resources').rglob('*'):
        if not path.is_file():
            continue
        destination = ROOT / 'Client/Bin/Resources' / path.relative_to(OUT / 'Resources')
        assert not destination.exists() or destination.read_bytes() == path.read_bytes(), destination
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not destination.exists():
            shutil.copyfile(path, destination)
    import install_kouku_gate1_native_shaders as shaders
    shaders.append_reviewed(OUT / 'native')
    for path, previous, payload in pending:
        replace_checked(path, previous, payload)
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    previous = catalog_path.read_bytes()
    catalog = json.loads(previous)
    for asset in TARGETS:
        row = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + asset + '.effect.json')
        existing = next((entry for entry in catalog['effects'] if entry['effectAssetId'] == asset), None)
        assert existing is None or existing == row
        if existing is None:
            catalog['effects'].append(row)
    replace_checked(catalog_path, previous, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode())
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    previous = tree_path.read_bytes()
    text = previous.decode('utf8')
    anchor = next(line for line in text.splitlines(keepends=True) if '"assetId": "effect.world.move_destination"' in line)
    additions = ''.join('    ' + json.dumps(dict(kind='V1', assetId=asset, displayName=label, parentId='world'),
                                         ensure_ascii=False) + ',\n'
                        for asset, (_, label) in TARGETS.items() if '"assetId": "' + asset + '"' not in text)
    replace_checked(tree_path, previous, text.replace(anchor, anchor + additions).encode())
    metadata_paths = [ROOT / 'Data/Effects/Authored' / (asset + '.effect.json') for asset in TARGETS]
    for path, previous, content in stage_data_solution(ROOT, metadata_paths):
        replace_checked(path, previous, content)
```

### Tools/EffectPipeline/build_kouku_spider_counter_restore.py

모듈 import/상수 범위:

```python
"""Project action 4219776 source notifies onto the authored spider-counter clips.

Uses the existing Cascade projector and V1 Effect presentation. The two source
stage documents are installed explicitly; current pattern timing and user edits
are preserved with compare-before-write checks. Native programs are recovered by
the existing native-material pipeline and supplied through --native-material-patch.
"""
from pathlib import Path
import sys
import argparse
import copy
import json

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract

ROOT = source.ROOT
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
ACTION_ID = 4219776
PATTERN_ID = 'KAKULSAYDON_G1_PATTERN_15'
STAGES = {1: ('rpcz00_att_battle_6_02', '눈빛·먼지'),
          2: ('rpcz00_att_battle_6_03', '검정·빨강 바닥과 돌진')}


```

변경 함수 전체:

```python
def stage_install(docs, durations, evidence, replace_authored=False):
    writes = []
    def stage(path, updated):
        current = path.read_bytes() if path.exists() else None
        writes.append((path, current, updated))
    for doc in docs.values():
        path = ROOT / 'Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
        if path.exists() and not replace_authored:
            assert source.read(path) == doc, f'Existing authored edits differ; preserve {path}'
        stage(path, encoded(doc))
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    catalog_bytes = catalog_path.read_bytes()
    catalog = json.loads(catalog_bytes)
    for doc in docs.values():
        row = dict(effectAssetId=doc['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + doc['effectAssetId'] + '.effect.json')
        matching = [r for r in catalog['effects'] if r['effectAssetId'] == row['effectAssetId']]
        assert not matching or matching == [row]
        if not matching:
            catalog['effects'].append(row)
    writes.append((catalog_path, catalog_bytes, encoded(catalog)))
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    # Capture exactly the bytes used for projection, not a second read after it.
    composition_bytes = composition_path.read_bytes()
    composition = json.loads(composition_bytes)
    updated, links = connect(composition, durations)
    writes.append((composition_path, composition_bytes, encoded(updated)))
    metadata_paths = [ROOT / 'Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
                      for doc in docs.values()]
    writes.extend(stage_data_solution(ROOT, metadata_paths))
    source.write(evidence / 'composition_before.json', composition)
    source.write(evidence / 'composition_candidate.json', updated)
    source.write(evidence / 'composition_links.json', links)
    return writes
```

### Tools/EffectPipeline/build_valtan_full_restore.py

모듈 import/상수 범위:

```python
"""Build Valtan source documents through the shared full-restore pipeline.

The library owns original first-LOD Cascade/native material data. This adapter
owns Valtan's installed skeleton and explicit original Action stage selection.
Existing authored Effects are never overwritten, and unresolved source notifies
remain explicit in the receipt instead of acquiring invented attachments.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import json
import math
import os
import re
import sys
import struct
import tempfile

import build_kouku_action_effect_groups as action
import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source
import build_kouku_gate3_slam_mario_restore as exact
from extract_action_effect_notifies import read_length_prefixed_string

ROOT = source.ROOT
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
PROFILE = 'MN_RPBF_00'
BODY = 'Character/Valtan/MN_RPBF_01.wmodel'
ANIMATIONS = Path('Data/Effects/ValtanFullRestoreAnimations.json')


```

변경 함수 전체:

```python
def project_writes(asset_ids, include_animations):
    # 기존 호출자의 후보/백업/rollback 묶음 안에 솔루션 표시 변경도 포함한다.
    paths = [ROOT / 'Data/Effects/Authored' / (asset + '.effect.json') for asset in sorted(asset_ids)]
    if include_animations:
        paths.append(ROOT / ANIMATIONS)
    return stage_data_solution(ROOT, paths)
```

### Tools/EffectPipeline/install_kouku_effect_library.py

모듈 import/상수 범위:

```python
"""Register restored Kouku groups in the V1 library and existing authoring trees.

Only successfully installed source documents are accepted. Existing unrelated
catalog entries, composition edits and project registrations are preserved.
"""
import argparse
import copy
import hashlib
import json
import re
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
NAMES = {
    'staff.flame.full.restore': ('불뿜기', '불뿜기'),
    'doll.flame.full.restore': ('마리오', '기괴한 인형 양쪽 화염'),
    'mario.center.pentagram.full.restore': ('마리오', '중앙 오망성'),
    'mario.center.portal.full.restore': ('마리오', '중앙 진입 포탈'),
    'mario.boss.pentagram.full.restore': ('마리오', '작은 오망성'),
    'mario.center.full.restore': ('마리오', '중앙 오망성와 포탈'),
    'showtime.gun.create': ('쇼타임/기관총', '기관총 생성'),
    'showtime.gun.loop': ('쇼타임/기관총', '기관총 유지'),
    'showtime.gun.end': ('쇼타임/기관총', '기관총 종료'),
    'showtime.gun.shell': ('쇼타임/기관총', '탄피'),
    'showtime.gun.muzzle': ('쇼타임/기관총', '총구 발사'),
    'showtime.gun.signature': ('쇼타임/기관총', '발사 섬광'),
    'showtime.gun.ground': ('쇼타임/장판·조준·폭발', '사격 바닥 표시'),
    'showtime.airstrike.impact': ('쇼타임/공 낙하·폭발', '공습 폭발'),
    'showtime.airstrike.missile': ('쇼타임/공 낙하·폭발', '공습 투사체'),
    'showtime.ball.impact': ('쇼타임/공 낙하·폭발', '공 충돌 폭발'),
    'showtime.ball.red': ('쇼타임/공 낙하·폭발', '빨간 공 폭발'),
    'showtime.circle.impact01': ('쇼타임/장판·조준·폭발', '원형 장판 폭발 01'),
    'showtime.circle.impact03': ('쇼타임/장판·조준·폭발', '원형 장판 폭발 03'),
    'showtime.ball.drop': ('쇼타임/공 낙하·폭발', '공 낙하'),
    'showtime.napalm': ('쇼타임/장판·조준·폭발', '화염 장판'),
    'showtime.target.fixed': ('쇼타임/장판·조준·폭발', '바닥 고정 조준점'),
    'showtime.target.tracking': ('쇼타임/장판·조준·폭발', '바닥 추적 조준점'),
    'showtime.target.end': ('쇼타임/장판·조준·폭발', '조준 종료'),
    'showtime.fire.impact': ('쇼타임/장판·조준·폭발', '화염 폭발'),
    'showtime.bullet': ('쇼타임/기관총', '총알 비행·충돌'),
}


SPLIT_GROUP_REPLACEMENTS = {
    'effect.kouku.gate3.showtime.circle.warning.impact':
        ('effect.kouku.gate3.showtime.circle.warning', '원형 예고·폭발 (이전 합성)'),
    'effect.kouku.gate3.showtime.donut.warning.impact':
        ('effect.kouku.gate3.showtime.innerdonut.warning', '도넛 예고·폭발 (이전 합성)'),
}


```

변경 함수 전체:

```python
def register(manifests, organization, output, install, library_only=False):
    snapshots, staged = {}, {}
    def load(relative):
        path = ROOT / relative
        snapshots[path] = path.read_bytes()
        return json.loads(snapshots[path])
    catalog = load('Data/Effects/EffectCatalog.json')
    tree = load('Data/Effects/EffectResourceTree.json')
    composition = load('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    source_organization = read(organization) if organization else {}
    source_rows = {r['sourceSystem']: r for r in source_organization.get('particleSystems', [])}
    documents = []
    for path in manifests:
        manifest = read(path)
        assert manifest['installed'], f'Source documents are not installed: {path}'
        documents += manifest['documents']
    nodes = {n['id']: n for n in tree['nodes']}
    references = {(r['kind'], r['assetId']): r for r in tree['references']}
    def category(labels):
        parent, path = 'root.v1', []
        for label in labels:
            path.append(label)
            key = 'kouku.category.' + identity('/'.join(path))
            row = dict(id=key, parentId=parent, kind='CATEGORY', displayName=label)
            assert key not in nodes or nodes[key] == row
            nodes[key] = row
            parent = key
        return parent
    known = {r['effectAssetId']: r for r in catalog['effects']}
    resources = {r['assetId']: r for r in composition['presentationResources'] if r['kind'] == 'EFFECT'}
    pattern = None if library_only else next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_32')
    if pattern is not None:
        assert pattern['gateId'] == 'GATE3' and '쇼타임' in pattern['displayName']
    additions = []
    for row in documents:
        path = ROOT / row['path']
        doc = read(path)
        asset = row['effectAssetId']
        assert doc['effectAssetId'] == asset and doc['elements']
        for element in doc['elements']:
            if element['kind'] not in ('light', 'screenPost') and not element.get('sourceRecipe', {}).get('simulationOnly'):
                assert element['material'].get('sourceProfile', {}).get('enabled'), f'Material projection is incomplete: {asset}'
        relative = path.relative_to(ROOT / 'Data').as_posix()
        entry = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=relative)
        assert asset not in known or known[asset] == entry
        if asset not in known:
            catalog['effects'].append(entry)
            known[asset] = entry
        suffix = asset.removeprefix('effect.kouku.gate3.')
        if suffix in NAMES and not row.get('categoryPath'):
            branch, label = NAMES[suffix]
            labels = ['KoukuSaydon', '3관문', '패턴', '세이튼'] + branch.split('/')
            name = '3관문_세이튼_' + branch.split('/')[0] + '_' + label
            if suffix == 'mario.boss.pentagram.full.restore':
                name = label
        else:
            labels = row.get('categoryPath', [])
            name = row['displayName']
            if not labels:
                source_row = source_rows.get(row.get('sourceParticleSystem'), {})
                sequence_uses = source_row.get('sequenceUses', [])
                if sequence_uses:
                    sequence = sequence_uses[0]
                    classification = sequence['classification']
                    gate = classification.get('gateId', 'SHARED')
                    labels = ['KoukuSaydon', {'GATE1':'1관문','GATE2':'2관문','GATE3':'3관문'}.get(gate,'공통'),
                        '연출', classification.get('displayName') or sequence['sourceScene'], sequence['groupName']]
                else:
                    labels = ['KoukuSaydon', '공통', '개별 이펙트', row.get('sourceParticleSystem', asset).split('.')[0]]
        assert len(name.encode('utf8')) <= 256
        if suffix == 'staff.flame.full.restore':
            name = '불뿜기'
            previous = references.get(('V1', asset))
            parent = previous['parentId'] if previous else category(labels)
            nodes[parent]['displayName'] = '불뿜기'
        else:
            parent = category(labels)
        references[('V1', asset)] = dict(kind='V1', assetId=asset, displayName=name, parentId=parent)
        if doc['displayName'] != name:
            snapshots[path] = path.read_bytes()
            doc['displayName'] = name
            staged[path] = payload(doc)
        if asset not in resources:
            resource = dict(resourceId='kakulsaydon.effect.' + identity(asset), displayName=name,
                defaultAnchorKind=row.get('defaultAnchorKind', 'BOSS' if doc.get('sourceModelPreview') else 'MAP'), kind='EFFECT', assetId=asset,
                resourceKind='V1_EFFECT', elementId='', durationMs=row['durationMs'], shape='BOX', colliderKind='GEOMETRY',
                halfExtents=[1,1,1], radiusM=3, halfAngleDegrees=45)
            composition['presentationResources'].append(resource)
            resources[asset] = resource
        else:
            resources[asset]['displayName'] = name
        if pattern is not None and suffix.startswith('showtime.') and not any(o['resourceId'] == resources[asset]['resourceId'] for o in pattern['presentationOccurrences']):
            ordinal = pattern['nextPresentationOccurrenceOrdinal']
            pattern['presentationOccurrences'].append(dict(occurrenceId=pattern['patternId'] + '.presentation.' + str(ordinal),
                resourceId=resources[asset]['resourceId'], startMs=0, durationMs=row['durationMs'], positionOffset=[0,0,0],
                rotationDegrees=[0,0,0], scale=[1,1,1], fadeInMs=0, fadeOutMs=0, dissolveStart=.95, dissolveEnd=1,
                brightnessMultiplier=1, volume=1, followBoss=bool(doc.get('sourceModelPreview')), debugRender=True,
                bone='', boneTarget='BODY', regionId='', cardSymbol='NONE', cardColor='NONE',
                anchorKind=resources[asset]['defaultAnchorKind'], worldId='', logicOccurrenceId='', worldOccurrenceId=''))
            pattern['nextPresentationOccurrenceOrdinal'] = ordinal + 1
        additions.append(dict(effectAssetId=asset, displayName=name, categoryPath=labels, elements=len(doc['elements']),
            durationMs=row['durationMs'], path=row['path']))
    if library_only:
        installed_assets = {row['effectAssetId'] for row in documents}
        for legacy_asset, (replacement, label) in SPLIT_GROUP_REPLACEMENTS.items():
            if replacement not in installed_assets:
                continue
            # Preserve saved IDs and authored payloads; organize the replaced
            # combinations separately from the independent sequencing groups.
            legacy = references.get(('V1', legacy_asset))
            if legacy is not None:
                legacy.update(displayName=label, parentId=category(
                    ['KoukuSaydon', '3관문', '패턴', '세이튼', '쇼타임', '이전 합성']))
            if legacy_asset in resources:
                resources[legacy_asset]['displayName'] = label
    tree['nodes'] = list(nodes.values())
    tree['references'] = list(references.values())
    staged[ROOT / 'Data/Effects/EffectCatalog.json'] = payload(catalog)
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    if library_only:
        value = replace_root_value(snapshots[tree_path], 'nodes', tree['nodes'], rows=True)
        staged[tree_path] = replace_root_value(value, 'references', tree['references'], rows=True)
    else:
        staged[tree_path] = payload(tree)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    if json.loads(snapshots[composition_path]) != composition:
        composition['revision'] += 1
        if library_only:
            value = replace_root_value(snapshots[composition_path], 'presentationResources', composition['presentationResources'], rows=True)
            staged[composition_path] = replace_root_value(value, 'revision', composition['revision'])
            assert json.loads(staged[composition_path]) == composition
        else:
            staged[composition_path] = payload(composition)
    for path, before, after in stage_data_solution(ROOT, [ROOT / row['path'] for row in additions]):
        snapshots[path] = before
        staged[path] = after
    for path, before in snapshots.items():
        assert path.read_bytes() == before, f'Concurrent authoring edit: {path}'
    changed = []
    for path, value in staged.items():
        if value != snapshots[path]:
            changed.append(path.relative_to(ROOT).as_posix())
            if install: path.write_bytes(value)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(payload(dict(installed=install, documents=additions, changedPaths=changed,
        showtimePatternId=pattern['patternId'] if pattern else None,
        showtimeMode='LIBRARY_ONLY_MANUAL_SEQUENCER_TIMING' if library_only else 'INDEPENDENT_GROUP_TUNING_DRAFT',
        manualVisualValidation='USER_PENDING')))
    print(json.dumps(dict(installed=install, groups=len(additions), changedFiles=len(changed))))
```

### Tools/EffectPipeline/sync_kouku_effect_tree.py

모듈 import/상수 범위:

```python
"""Stage Kouku authored library references and their direct-runtime metadata.

Existing classifications and all effect/Composition documents remain intact.
The tree and catalog are published together; payloads still load lazily through
the ordinary runtime codec. No gameplay timelines are authored by this tool.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
from pathlib import Path
import sys

from install_kouku_effect_library import replace_root_value

ROOT = Path(__file__).resolve().parents[2]
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
TREE = ROOT / 'Data/Effects/EffectResourceTree.json'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
COMPOSITIONS = (
    ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json',
    ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
)
ACTORS = {'MN_RPCZ_00': '쿠크', 'MN_RPCT_05': '세이튼',
          'MN_RPCT_06': '대형 세이튼', 'MN_RPCT_07': '쿠크세이튼'}


```

변경 함수 전체:

```python
def stage_project_metadata(inputs):
    # 외부 소비자는 이 함수의 후보 tuple을 기존 freshness/원자 교체 과정에 합친다.
    return stage_data_solution(ROOT, inputs)
```

### Tools/VehiclePipeline/build_vehicle_skill_effects.py

모듈 import/상수 범위:

```python
"""Restore vehicle skill ParticleSystems through the source V1 Effect pipeline.

Source notifies come from the vehicle Action .loa extracts. ParticleSystem
closures are read directly from the installed game packages, reusing the Kouku
full-restore acquisition without its CanonicalSource extract.
"""
from pathlib import Path
import argparse, base64, collections, copy, json, math, re, struct, sys

ROOT = Path(__file__).resolve().parents[2]
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
import build_kouku_gate1_full_restore as source
import extract_ue3_placements as ue3
from extract_ue3_placements import resolve_physical_package
from extract_ue3_effect_material_closure import load_package, find_export
from bake_ghost_valtan_animset import read_sections, skeleton_bones

RELEASE = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
UMODEL = Path('C:/Users/95jus/Downloads/umodel_win32/umodel_lostark_v7.exe')
SCRIPT_PACKAGES = {'engine': RELEASE / 'NE1FENCQ4UNE9ZPRENOQS.u', 'efgame': RELEASE / 'NU1V7NCQ4YAE9ZPJVNOQS.u'}
SPEC = ROOT / 'Tools/VehiclePipeline/VehicleSkills.spec.json'
CATALOG = ROOT / 'Data/Actors/VehicleCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'
VEHICLE_TABLE = Path('C:/Users/95jus/Downloads/SourceData/SourceData/LPK/data2/EFGame_Extra/ClientData/TableData/EFTable_Vehicle.db')
SKIN_VARIANT = re.compile(r'-\d+$')
# Body SkeletalMesh of each installed vehicle model (see the vehicle riding RESULTs).
SOURCE_MESHES = {'Terpeion': ('MN_PMSTG_01', 'mesh.mn_pmstg_01_sk'),
    'SereneStarlightBlessing': ('MN_PMSSM_00', 'mesh.mn_pmssm_00_sk'),
    'RainbowMokoboard': ('MN_PMSMK_00', 'mesh.mn_pmsmk_00_sk'),
    'Aufstehen': ('MN_PMSHE_00', 'mesh.mn_pmshe_00_sk'),
    'SeaUnicornTube': ('MN_PMSUT_00', 'mesh.mn_pmsut_00_sk'),
    'AncientMyth': ('MN_PMSDZ_00', 'mesh.mn_pmsdz_00_sk'),
    'AncientSea': ('MN_PMSDZ_00', 'mesh.mn_pmsdz_00_sk')}


```

변경 함수 전체:

```python
def install_documents(assets, candidates):
    for asset in assets:
        document = candidates / (asset + '.effect.json')
        target = AUTHORED / document.name
        if not target.is_file() or target.read_bytes() != document.read_bytes():
            target.write_bytes(document.read_bytes())
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    text = catalog_path.read_text(encoding='utf-8')
    newline = '\r\n' if '\r\n' in text else '\n'
    rows = [json.dumps(dict(effectAssetId=a, payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=f'Effects/Authored/{a}.effect.json'), ensure_ascii=False)
            for a in assets if f'"effectAssetId": "{a}"' not in text]
    if rows:
        marker = newline + '  ]' + newline + '}'
        assert text.count(marker) == 1
        text = text.replace(marker, ',' + newline + (',' + newline).join('    ' + r for r in rows) + marker)
        json.loads(text)
        catalog_path.write_bytes(text.encode('utf-8'))
    metadata_paths = [ROOT / 'Data/Effects/Authored' / (asset + '.effect.json') for asset in assets]
    for path, before, after in stage_data_solution(ROOT, metadata_paths):
        if before != after:
            assert path.read_bytes() == before, f'Concurrent solution edit: {path}'
            path.write_bytes(after)
```

### Tools/KoukuSaydonPipeline/prepare_flame_unification.py

모듈 import/상수 범위:

```python
"""Stage the shared flame library and authored flame-wave consumers.

Preparation never writes live authoring. Applying a reviewed candidate requires
unchanged input hashes and closed Client/Server processes; a failed replacement
restores every file already replaced. Runtime data remains publisher-owned.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
from install_kouku_effect_library import identity, replace_root_value

COMPOSITION = 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
STAFF = 'effect.kouku.gate3.staff.flame.full.restore'
SHARED = 'effect.kouku.gate3.firebreath.shared'
WAVE = 'effect.kouku.common.flame.wave.full'
KOUKU_WAVE = WAVE + '.koukusaydon'


```

변경 함수 전체:

```python
def prepare(manifests, output):
    output = repository_path(output)
    if not output.is_relative_to(ROOT / 'out'):
        raise ValueError('Preparation output must be under out/')
    snapshots, staged, rows, dependencies = {}, {}, [], {}

    def snapshot(relative):
        path = repository_path(relative)
        key = path.relative_to(ROOT).as_posix()
        if key not in snapshots:
            snapshots[key] = path.read_bytes() if path.exists() else None
        return snapshots[key]

    for manifest_path in manifests:
        manifest = read(repository_path(manifest_path))
        for path, expected in manifest.get('inputHashes', {}).items():
            actual = digest(repository_path(path).read_bytes())
            if actual != expected:
                raise ValueError(f'Generator input changed; regenerate candidate: {path}')
            dependencies[repository_path(path).relative_to(ROOT).as_posix()] = expected
        for row in manifest['documents']:
            candidate = repository_path(row['candidatePath']).read_bytes()
            document = json.loads(candidate)
            asset = row['effectAssetId']
            relative = 'Data/Effects/Authored/' + asset + '.effect.json'
            if row['path'] != relative or document['effectAssetId'] != asset:
                raise ValueError('Effect identity/destination mismatch')
            if any(old['effectAssetId'] == asset for old in rows):
                raise ValueError(f'Duplicate candidate asset: {asset}')
            snapshot(relative)
            staged[relative] = candidate
            rows.append(row)

    catalog_path, tree_path = 'Data/Effects/EffectCatalog.json', 'Data/Effects/EffectResourceTree.json'
    catalog, tree, composition = [json.loads(snapshot(p)) for p in (catalog_path, tree_path, COMPOSITION)]
    before_composition = copy.deepcopy(composition)
    resources = {r['assetId']: r for r in composition['presentationResources'] if r['kind'] == 'EFFECT'}
    nodes = {n['id']: n for n in tree['nodes']}
    references = {(r['kind'], r['assetId']): r for r in tree['references']}
    known = {r['effectAssetId']: r for r in catalog['effects']}

    def category(labels):
        parent, branch = 'root.v1', []
        for label in labels:
            branch.append(label)
            key = 'kouku.category.' + identity('/'.join(branch))
            value = dict(id=key, parentId=parent, kind='CATEGORY', displayName=label)
            if key in nodes and nodes[key] != value:
                raise ValueError(f'Category collision: {key}')
            nodes[key], parent = value, key
        return parent

    for row in rows:
        asset, name = row['effectAssetId'], row['displayName']
        entry = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                     authoringPath='Effects/Authored/' + asset + '.effect.json')
        if asset in known and known[asset] != entry:
            raise ValueError(f'Catalog identity collision: {asset}')
        if asset not in known:
            catalog['effects'].append(entry)
        reference = references.get(('V1', asset))
        parent = (reference['parentId'] if asset == STAFF and reference else
                  row.get('parentId') or (category(row['categoryPath']) if row.get('categoryPath') else reference['parentId']))
        if parent not in nodes:
            raise ValueError(f'Unknown candidate parent: {parent}')
        references[('V1', asset)] = dict(kind='V1', assetId=asset, displayName=name, parentId=parent)
        if asset not in resources:
            resource = dict(resourceId='kakulsaydon.effect.' + identity(asset), displayName=name,
                            defaultAnchorKind=row.get('defaultAnchorKind', 'BOSS'), kind='EFFECT',
                            assetId=asset, resourceKind='V1_EFFECT', elementId='', durationMs=row['durationMs'],
                            shape='BOX', colliderKind='GEOMETRY', halfExtents=[1, 1, 1], radiusM=3,
                            halfAngleDegrees=45)
            composition['presentationResources'].append(resource)
            resources[asset] = resource
        resources[asset].update(displayName=name, durationMs=row['durationMs'])
    if STAFF in resources:
        resources[STAFF]['displayName'] = '불뿜기'
        references[('V1', STAFF)]['displayName'] = '불뿜기'
        nodes[references[('V1', STAFF)]['parentId']]['displayName'] = '불뿜기'
        relative = 'Data/Effects/Authored/' + STAFF + '.effect.json'
        original = snapshot(relative)
        staged[relative] = replace_root_value(original, 'displayName', '불뿜기')

    changed_patterns = []
    p49 = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_49')
    start, length = clip_start(p49, 'rpct00_att_battle_10_02')
    kouku_wave = resources[KOUKU_WAVE]
    if any(o['resourceId'] == kouku_wave['resourceId'] for o in p49['presentationOccurrences']):
        raise ValueError('Wave integration already exists; inspect instead of duplicating')
    wave_end = start + 1904 + kouku_wave['durationMs']
    if wave_end > length:
        # The fixed timeline owns the remaining decal tail; existing authored
        # animation boxes keep their speed, source range and stage durations.
        p49['durationMs'] = wave_end
    occurrence(p49, kouku_wave, start + 1904, kouku_wave['durationMs'])
    changed_patterns.append(p49['patternId'])
    p43 = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_43')
    start, length = clip_start(p43, 'rpct00_att_battle_9_02')
    if any(o['resourceId'] == resources[SHARED]['resourceId'] for o in p43['presentationOccurrences']):
        raise ValueError('Backstep integration already exists; inspect instead of duplicating')
    occurrence(p43, resources[SHARED], start, min(length - start, resources[SHARED]['durationMs']),
               bone='bip001-head', follow=True)
    changed_patterns.append(p43['patternId'])
    for profile, action, name in [('MN_RPCT_05', 4219820, '세이튼_화염 파동'),
                                   ('MN_RPCT_07', 4219948, '쿠크세이튼_화염 파동')]:
        if any(p['displayName'] == name for p in composition['patterns']):
            raise ValueError(f'Existing user pattern needs explicit rebase: {name}')
        wave = resources[WAVE] if profile == 'MN_RPCT_05' else kouku_wave
        changed_patterns.append(wave_pattern(composition, profile, action, name, wave)['patternId'])
        ref = f'Data/Animation/Reference/KoukuSaydon/{profile}.actionreference.json'
        dependencies[ref] = digest((ROOT / ref).read_bytes())

    # No unrelated user pattern, lane, ID or authored payload can disappear.
    after_by_id = {p['patternId']: p for p in composition['patterns']}
    for p in before_composition['patterns']:
        after = copy.deepcopy(after_by_id[p['patternId']])
        if p['patternId'] in changed_patterns:
            after['presentationOccurrences'] = after['presentationOccurrences'][:-1]
            after['nextPresentationOccurrenceOrdinal'] -= 1
        if p['patternId'] == p49['patternId']:
            if 'durationMs' in p:
                after['durationMs'] = p['durationMs']
            else:
                after.pop('durationMs', None)
        if after != p:
            raise ValueError(f'Unrelated user pattern fields changed: {p["patternId"]}')
    composition['revision'] += 1
    from project_kouku_saydon_composition import (
        _publication_candidate, _saved_pattern_inventory, validate_document, validate_publishable,
    )
    # The regular publisher admits each complete Pattern separately. Existing
    # unfinished editor rows remain visible and must not be "fixed" or deleted
    # merely to validate this change (for example the empty P32 Showtime draft).
    _saved_pattern_inventory(composition, ROOT)
    checked = _publication_candidate(composition, set(changed_patterns))
    validate_document(checked, ROOT)
    validate_publishable(checked, ROOT)
    value = snapshots[COMPOSITION]
    for field in ('presentationResources', 'nextPatternOrdinal', 'revision'):
        value = replace_root_value(value, field, composition[field], rows=field == 'presentationResources')
    value = replace_pattern_records(value, composition['patterns'])
    staged[COMPOSITION] = value
    tree['nodes'], tree['references'] = list(nodes.values()), list(references.values())
    value = replace_root_value(snapshots[tree_path], 'nodes', tree['nodes'], rows=True)
    staged[tree_path] = replace_root_value(value, 'references', tree['references'], rows=True)
    staged[catalog_path] = payload(catalog)
    for path, before, after in stage_data_solution(ROOT, [ROOT / row['path'] for row in rows]):
        relative = path.relative_to(ROOT).as_posix()
        snapshots[relative] = before
        staged[relative] = after

    files = []
    for relative, after in staged.items():
        before = snapshots[relative]
        if before == after:
            continue
        destination = output / 'after' / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(after)
        if before is not None:
            backup = output / 'before' / relative
            backup.parent.mkdir(parents=True, exist_ok=True)
            backup.write_bytes(before)
        files.append(dict(path=relative, beforeSha256=digest(before) if before is not None else None,
                          afterSha256=digest(after)))
    receipt = dict(installed=False, beforeRevision=before_composition['revision'],
                   afterRevision=composition['revision'], files=files, inputHashes=dependencies,
                   changedPatterns=changed_patterns, unchangedUserPatternsAndFields=True,
                   waveAnchor='One BOSS snapshot at first warning; all ten impact points share it',
                   manualVisualValidation='USER_PENDING')
    output.mkdir(parents=True, exist_ok=True)
    (output / 'receipt.json').write_bytes(payload(receipt))
    print(json.dumps(dict(output=str(output), files=len(files), patterns=changed_patterns,
                          beforeRevision=receipt['beforeRevision'], afterRevision=receipt['afterRevision'])))
```

### Tools/KoukuSaydonPipeline/build_source_sequences.py

모듈 import/상수 범위:

```python
"""Project source Matinees into existing Camera/WorldSequence/Composition JSON.

Source packages and previously authored rows are read-only inputs. Boss WANM
clips append atomically to their Character bodies; standalone map props retain
their per-sequence directories. Authoring installation checks byte baselines.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import math
import shlex
import sys
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation
import build_gate2_intro_composition as base
from bake_character_cinematic_clips import DONOR_TO_BODY, install_baked_clip
import bake_reflected_static_props as reflected
from build_gate2_intro_backdrops import reduced_indices

ROOT = base.ROOT
# Data 표시는 솔루션 폴더의 후보만 만들고, 교체는 각 도구의 기존 저장 경계가 맡는다.
sys.path.insert(0, str(ROOT / 'Tools/Build'))
from data_solution import stage_data_solution
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
from apply_world_object_material_source import assign_material_source, assign_map_material_source, inherit_material_source
OUT = ROOT / 'out/KoukuSourceSequenceRestore20260912'
AREA = base.AREA_DIR
CONFIGS = [
    dict(id=4, name='1관문_통합_시퀀스', prefix='kouku.gate1.full', scene='SCENE03A',
         matinee=365, data=857, duration=41488, start=0, gate='GATE1', combat=True),
    dict(id=5, name='2관문_클리어', prefix='kouku.gate2.clear', scene='SCENE02A',
         matinee=63, data=117, duration=35368, start=0, gate='GATE2', combat=False),
    dict(id=6, name='2관문_카드미로', prefix='kouku.gate2.maze', scene='SCENE04A',
         matinee=328, data=393, duration=11950, start=0, gate='GATE2', combat=False),
    dict(id=7, name='3관문_진입', prefix='kouku.gate3.intro', scene='SCENE02A',
         matinee=63, data=117, duration=35368, start=16710, gate='GATE3', combat=True),
]


```

변경 함수 전체:

```python
def project_registration(effect_paths):
    """솔루션 Data 링크 후보를 만들어 기존 설치의 baseline 검사에 합친다."""
    return stage_data_solution(ROOT, effect_paths)
```

### Tools/EffectPipeline/build_saydon_circus_world.py

모듈 import/상수 범위:

```python
"""Build out-only Saydon circus World motions and source-backed V1 groups.

The source callback graph owns five splits; the user owns the +90 +/-45 degree
directions and one-bounce presentation. Existing WorldSequence group/emissions
own every object and effect clock. This adds no gameplay or network authority.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import sys

import build_kouku_circus_ball_groups as circus
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = circus.ROOT
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
from apply_saydon_ball_world_motions import geometry

OUT = ROOT / 'out/SaydonCircusWorld20260917'
WORLD = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate1.circus.'
WORLD_PREFIX = 'world.object.kouku.saydon.circus.'
MI_PREFIX = 'world.object.instance.kouku.saydon.circus.'
SQ_PREFIX = 'sequence.kouku.saydon.circus.'


```

변경 함수 전체:

```python
def build_launch(output):
    """Derive an upward ball with a lower rainbow/star wake; never install it."""
    from build_kouku_dove_pizza_candidates import module, vector, native_references
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidates must stay under out'
    source_path = AUTHORED / (PREFIX + 'rainbow.drop.effect.json')
    baseline_bytes = source_path.read_bytes()
    baseline = json.loads(baseline_bytes.decode('utf-8-sig'))
    asset = PREFIX + 'ball.launch'
    document = copy.deepcopy(baseline)
    selected = [e for e in document['elements']
                if '.par_k_ppct_vividfracture_ball_04.' in e['sourceNode']]
    by_emitter = {e['sourceNode'].rsplit('_', 1)[-1]: e for e in selected}
    # Keep the saved four-element ball, including the user's removal of sprite35.
    assert set(by_emitter) == {'5', '30', '33', '34'}, 'Inspect new saved ball elements before deriving launch'
    identities = {e['id']: 'circus.launch.' + hashlib.sha256(e['id'].encode()).hexdigest()[:20]
                  for e in selected}
    source_system = by_emitter['5']['sourceRecipe']['particleSystemOccurrenceId']
    identities[source_system] = asset + '.project-authored-upward'
    document['elements'] = remap(selected, identities)
    document.update(effectAssetId=asset, displayName='세이튼 / 쓰리투원투하 | 공 발사', modelCues=[])
    document.pop('sourceModelPreview', None)
    document.pop('sourceAnchorAnimations', None)
    for element in document['elements']:
        element['groupId'] = asset
        element['actionCueAttachment']['enabled'] = False
    by_emitter = {e['sourceNode'].rsplit('_', 1)[-1]: e for e in document['elements']}
    mesh, rainbow = by_emitter['5'], by_emitter['30']
    mesh_asset = next(r['assetId'] for r in mesh['resources'] if r['slotId'] == 'meshModel')
    measured = geometry(mesh_asset)
    size_module = next(m for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulesize')
    size = next(d for d in size_module['distributions'] if d['propertyPath'] == 'startsize')
    assert size['operation'] == 1 and size['lookupTableNumElements'] == 1
    size_xyz = size['lookupTable'][2:5]
    assert max(size_xyz) - min(size_xyz) < 1.e-6
    assert mesh['detail']['transform']['scale'] == [1, 1, 1]
    size_scale = size_xyz[0] * mesh['detail']['particle'].get('sourceScale', {}).get('size', 1)
    bottom = (measured['centerM'][1] - measured['halfExtentsM'][1]) * size_scale
    direct = next(m for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocationdirect')
    location = next(d for d in direct['distributions'] if d['propertyPath'] == 'location')
    assert location['operation'] == 1 and location['lookupTableChunkSize'] == 3 and not location['keys']
    samples = [location['lookupTable'][i:i + 3] for i in range(2, len(location['lookupTable']), 3)]
    assert samples == [[0, 0, 1000], [0, 0, -50]], 'Expected current 10.5m / 0.8s source drop'
    reversed_samples = list(reversed(samples))
    location['lookupTable'] = location['lookupTable'][:2] + [v for row in reversed_samples for v in row]
    # EmitterDirect owns the same live particle. Velocity only supplies the
    # source PSA_Velocity facing axis; offsetcentery0 extends the quad below it.
    rainbow['detail']['transform']['position'] = [0, 0, 0]
    for item in rainbow['sourceRecipe']['modules']:
        if item['className'] == 'particlemodulevelocity':
            set_enabled(item, True)
        elif item['className'] == 'particlemodulelocationdirect':
            set_enabled(item, False)
        elif item['className'] == 'particlemodulerequired':
            next(v for v in item['literals'] if v['propertyPath'] == 'offsetcentery')['value'] = 0
    followers = [m for m in rainbow['sourceRecipe']['modules']
                 if m['className'] == 'particlemodulelocationemitterdirect']
    assert len(followers) == 1 and any(v['propertyPath'] == 'runtime.providerelementid'
        and v['value'] == mesh['id'] for v in followers[0]['literals'])
    orbit = next(m for m in rainbow['sourceRecipe']['modules']
                 if m['className'] == 'particlemoduleorbit' and '.ballfollow.' in m['objectPath'])
    offset = next(d for d in orbit['distributions'] if d['propertyPath'] == 'offsetamount')
    offset.update(vector('offsetamount', [0, 0, bottom * 100]))
    # Stars are born at the ball underside, then retain their original sphere,
    # velocity/gravity, color and fade. They are a wake, not a second ball owner.
    for suffix in ('33', '34'):
        star = by_emitter[suffix]
        modules = star['sourceRecipe']['modules']
        provider = next(m for m in modules if m['className'] == 'efparticlemodulelocationemitter')
        literal = next((v for v in provider['literals'] if v['propertyPath'] == 'runtime.providerelementid'), None)
        if literal:
            literal['value'] = mesh['id']
        else:
            provider['literals'].append(dict(propertyPath='runtime.providerelementid', kind='string', value=mesh['id']))
        modules.insert(modules.index(provider) + 1, module(asset + '.' + suffix, 'underside',
            'particlemodulelocation', {'bspawnmodule': True}, [vector('startlocation', [0, 0, bottom * 100])]))
    # Provider first is required for same-tick burst and first star births.
    document['elements'] = [by_emitter[suffix] for suffix in ('5', '30', '33', '34')]
    refs = native_references(document)
    particle_life = mesh['detail']['particle']['lifeTimeSeconds']
    assert max(abs(v - .8) for v in particle_life) < 1.e-6
    assert all(not any('vividfracture_exp_' in m['objectPath'] for m in e['sourceRecipe']['modules'])
               for e in document['elements'])
    duration_ms = math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
        e['detail']['timing']['lifeTimeSeconds'] + e['detail']['timing']['afterImageSeconds'] +
        max(e['detail']['particle']['lifeTimeSeconds']) for e in document['elements']) * 1000)
    tree = read(ROOT / 'Data/Effects/EffectResourceTree.json')
    parent = next(r['parentId'] for r in tree['references'] if r.get('assetId') == baseline['effectAssetId'])
    registration = dict(installed=False, catalogEntry=dict(effectAssetId=asset,
        payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=f'Effects/Authored/{asset}.effect.json'),
        treeReference=dict(kind='V1', assetId=asset, displayName=document['displayName'], parentId=parent),
        compositionResourceProposal=dict(assetId=asset, displayName=document['displayName'], durationMs=duration_ms),
        solutionItemPath='Data/Effects/Authored/' + asset + '.effect.json',
        solutionFolder='Data/Effects/Authored', patternOccurrenceAdded=False)
    # Analytic positions use the retained StartLocation and mesh pivot. Native
    # Codec/Playback/geometry validation is a separate caller-owned checkpoint.
    start_location = next(d for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocation'
                          for d in m['distributions'] if d['propertyPath'] == 'startlocation')['lookupTable'][2:5]
    ys = [mesh['detail']['transform']['position'][1] + (row[2] + start_location[2]) * .01
          for row in reversed_samples]
    receipt = dict(status='OUT_CANDIDATE_PYTHON_CHECKED_NATIVE_PROBE_PENDING',
        basis='USER_REQUESTED_PROJECT_AUTHORED_REVERSE_SOURCE_DROP', sourcePath=source_path.relative_to(ROOT).as_posix(),
        sourceSha256=hashlib.sha256(baseline_bytes).hexdigest(), effectAssetId=asset,
        sourceCurveUE3Cm=samples, authoredCurveUE3Cm=reversed_samples, flightSeconds=particle_life[0],
        riseMeters=ys[1] - ys[0], centerStartYM=ys[0], centerEndYM=ys[1],
        startBottomYM=ys[0] + bottom, lowerAttachmentOffsetM=bottom,
        meshGeometry=measured, sourceSizeMultiplier=size_scale,
        ballWorldDiameterM=[2 * v * size_scale for v in measured['halfExtentsM']],
        elementIds={k: e['id'] for k, e in by_emitter.items()}, elementCount=len(document['elements']),
        durationMs=duration_ms, resources=refs, preserved='Saved ball model/scale and all four native materials, colors, particle lifetimes; source leaves/drop/World/Composition unchanged.',
        placement='Local +Y rise; the user authors the effect root angle and owner height. Rainbow uses ball underside, retained source +Y velocity facing and explicit lower pivot. Stars retain their native wake motion.',
        impactIncluded=False, clientRun=False, visualApproval=False, liveWrites=False)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'source-drop.before.json').write_bytes(baseline_bytes)
    write(output / (asset + '.effect.json'), document)
    write(output / 'registration.entries.json', registration)
    write(output / 'candidate.receipt.json', receipt)
    assert source_path.read_bytes() == baseline_bytes, 'Saved drop changed during candidate generation'
    print(json.dumps(dict(output=str(output), elements=len(document['elements']), riseMeters=ys[1] - ys[0],
                         durationMs=duration_ms, installed=False), ensure_ascii=False))
```

### Tools/KoukuSaydonPipeline/test_kouku_saydon_animation_pattern_document_contract.py

모듈 import/상수 범위:

```python

import sys as _cpp_domain_sys
from pathlib import Path as _CppDomainPath
_cpp_domain_sys.path.insert(0, str(_CppDomainPath(__file__).resolve().parents[1] / "Build"))
from cpp_source_domains import read_source_text
import copy
import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REFERENCE_ROOT = ROOT / "Data" / "Animation" / "Reference" / "KoukuSaydon"
AUTHORED_ROOT = ROOT / "Data" / "Animation" / "Authored" / "KoukuSaydon"
HEADER = ROOT / "Client" / "Public" / "KoukuSaydonAnimationPatternDocument.h"
SOURCE = ROOT / "Client" / "Private" / "KoukuSaydonAnimationPatternDocument.cpp"
ANIMATION_TOOL_SOURCE = ROOT / "Client" / "Private" / "Animation_Tool.cpp"
PROJECT = ROOT / "Client" / "Default" / "Client.vcxproj"
FILTERS = ROOT / "Client" / "Default" / "Client.vcxproj.filters"
SOLUTION = ROOT / "Framework.sln"

PROFILES = ("MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00")
ROOT_FIELDS = {
    "schema",
    "formatVersion",
    "profileId",
    "referenceRevision",
    "authority",
    "nextPatternOrdinal",
    "patterns",
}
PATTERN_FIELDS = {
    "patternId",
    "displayName",
    "sourceActionId",
    "nextOccurrenceOrdinal",
    "clips",
}
CLIP_FIELDS = {
    "occurrenceId",
    "stageId",
    "slotId",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "endPolicy",
}
END_POLICIES = {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW"}


```

변경 함수 전체:

```python
def test_project_and_solution_register_the_new_contract(self):
        project = PROJECT.read_text(encoding="utf-8-sig")
        filters = FILTERS.read_text(encoding="utf-8-sig")
        solution_lines = [line.strip() for line in SOLUTION.read_text(encoding="utf-8-sig").splitlines()]
        for entry in ("KoukuSaydonAnimationPatternDocument.h", "KoukuSaydonAnimationPatternDocument.cpp"):
            self.assertEqual(project.count(entry), 1, entry)
            self.assertEqual(filters.count(entry), 1, entry)
        for profile in PROFILES:
            entry = "Data\\Animation\\Authored\\KoukuSaydon\\" + profile + ".patternbindings.json"
            self.assertEqual(solution_lines.count(entry + " = " + entry), 1, entry)
            self.assertTrue((ROOT / entry.replace("\\", "/")).is_file(), entry)
            self.assertNotIn(profile + ".patternbindings.json", project)
            self.assertNotIn(profile + ".patternbindings.json", filters)
```

### Tools/ValtanPipeline/test_bern_entrance_camera_contract.py

모듈 import/상수 범위:

```python
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


```

변경 함수 전체:

```python
def test_solution_exposes_single_source_document(self) -> None:
        entry = r"Data\Encounters\Bern\BernEntranceCamera.json"
        solution_lines = [line.strip() for line in read("Framework.sln").splitlines()]
        self.assertEqual(1, solution_lines.count(entry + " = " + entry))
        self.assertTrue((ROOT / entry.replace("\\", "/")).is_file())
        self.assertNotIn("BernEntranceCamera.json", read("Client/Default/Client.vcxproj"))
        self.assertNotIn("BernEntranceCamera.json", read("Client/Default/Client.vcxproj.filters"))
```

### Tools/EffectPipeline/test_valtan_full_restore_animation_metadata.py

모듈 import/상수 범위:

```python
from pathlib import Path
import base64
import hashlib
import json
import struct
import tempfile
import unittest
from unittest import mock
import xml.etree.ElementTree as ET

import build_valtan_full_restore as builder


```

변경 함수 전체:

```python
def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.root_patch = mock.patch.object(builder, 'ROOT', self.root)
        self.root_patch.start()
        self.addCleanup(self.root_patch.stop)
        self.addCleanup(self.temp.cleanup)
        for suffix in ('', '.filters'):
            path = self.root / ('Client/Default/Client.vcxproj' + suffix)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'<Project>\r\n  <ItemGroup><None Include="preserve.json" /></ItemGroup>\r\n</Project>\r\n')
        (self.root / 'Framework.sln').write_bytes(
            b'Microsoft Visual Studio Solution File, Format Version 12.00\r\n'
            b'Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Notes", "Notes", "{00000000-0000-0000-0000-000000000001}"\r\n'
            b'\tProjectSection(SolutionItems) = preProject\r\n'
            b'\t\tpreserve.txt = preserve.txt\r\n'
            b'\tEndProjectSection\r\nEndProject\r\nGlobal\r\nEndGlobal\r\n')

def test_installed_source_metadata_preserves_effect_and_project_edits(self):
        path, row = self.install_fixture(400440, 0, 'Att_Battle_11_01', 1.7999999523162842)
        before = path.read_bytes()
        count = builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertEqual(count, 1)
        self.assertEqual(path.read_bytes(), before)
        result = json.loads((self.root / builder.ANIMATIONS).read_bytes())
        self.assertEqual(result['effects'][0]['animationClips'], [dict(clipName='mesh_att_battle_11_01', playMs=1800, loop=False,
            previewWallMs=1800, previewWallBasis='SOURCE_ANIMATION_WINDOW')])
        for suffix in ('', '.filters'):
            xml = (self.root / ('Client/Default/Client.vcxproj' + suffix)).read_bytes()
            self.assertIn(b'Include="preserve.json"', xml)
            self.assertNotIn(b'ValtanFullRestoreAnimations.json', xml)
            ET.fromstring(xml)
        solution = (self.root / 'Framework.sln').read_text(encoding='utf-8-sig')
        self.assertIn('preserve.txt = preserve.txt', solution)
        entry = r'Data\Effects\ValtanFullRestoreAnimations.json'
        self.assertEqual([line.strip() for line in solution.splitlines()].count(entry + ' = ' + entry), 1)
        self.assertTrue((self.root / entry.replace('\\', '/')).is_file())
        builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertEqual(path.read_bytes(), before)

def test_replace_failure_restores_prior_files_and_removes_new_mapping(self):
        _, row = self.install_fixture()
        projects = {path: path.read_bytes() for path in (self.root / 'Client/Default').iterdir()}
        solution = self.root / 'Framework.sln'
        projects[solution] = solution.read_bytes()
        actual_replace = builder.os.replace

        def fail_solution_replace(source, target):
            if target == solution:
                raise OSError('injected solution install failure')
            return actual_replace(source, target)

        with mock.patch.object(builder.os, 'replace', side_effect=fail_solution_replace):
            with self.assertRaisesRegex(OSError, 'injected'):
                builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertFalse((self.root / builder.ANIMATIONS).exists())
        for path, before in projects.items():
            self.assertEqual(path.read_bytes(), before)
        self.assertFalse(list(self.root.rglob('*.staged')))

def test_concurrent_solution_edit_rejects_before_metadata_replacement(self):
        _, row = self.install_fixture()
        writes = [builder.animation_metadata_write([row], [row['effectAssetId']])] + builder.project_writes([], True)
        changed = self.root / 'Framework.sln'
        changed.write_bytes(changed.read_bytes() + b'# another editor\r\n')
        with self.assertRaisesRegex(AssertionError, 'Concurrent edit'):
            builder.commit_writes(writes)
        self.assertFalse((self.root / builder.ANIMATIONS).exists())
        self.assertIn(b'another editor', changed.read_bytes())

def fail_solution_replace(source, target):
            if target == solution:
                raise OSError('injected solution install failure')
            return actual_replace(source, target)
```

### Tools/Build/test_build_domain_pipeline_receipts.py

모듈 import/상수 범위:

```python
#!/usr/bin/env python3
"""Focused contracts for build-domain fingerprint and freshness receipts."""

from __future__ import annotations

import json
import re
import shutil
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / "Tools/Build/BuildDomainPipeline.psm1"
MANIFEST = ROOT / "Tools/Build/BuildDomains.json"


```

변경 함수 전체:

```python
def setUp(self) -> None:
        self.document = json.loads(MANIFEST.read_text(encoding="utf-8"))
        self.domains = {row["id"]: row for row in self.document["domains"]}

def test_solution_exposes_kouku_saydon_world_and_navigation_authoring(self) -> None:
        project = (ROOT / "Client/Default/Client.vcxproj").read_text(encoding="utf-8-sig")
        filters = (ROOT / "Client/Default/Client.vcxproj.filters").read_text(encoding="utf-8-sig")
        solution_lines = [line.strip() for line in
                          (ROOT / "Framework.sln").read_text(encoding="utf-8-sig").splitlines()]
        paths = (
            r"Data\Navigation\LV_LUT_MIDNIGHTC_ED.navsource",
            r"Data\Navigation\LV_LUT_MIDNIGHTC_ED.navpaint",
            r"Data\Worlds\LV_LUT_MIDNIGHTC_ED\Gameplay.world.json",
            r"Data\Worlds\LV_LUT_MIDNIGHTC_ED\StageMarkers.json",
        )
        for path in paths:
            self.assertEqual(solution_lines.count(path + " = " + path), 1, path)
            self.assertTrue((ROOT / path.replace("\\", "/")).is_file(), path)
            self.assertNotIn(path, project)
            self.assertNotIn(path, filters)
```

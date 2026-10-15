# Data 솔루션 표시 통일과 A00 실제 읽기 결과

## 요청과 실제 변경

2026-10-12 사용자는 네 제품 프로젝트의 ConfigurationType·컴파일·링크·진입점을 배우면서, root Data와 Client 96.DataFiles가 같은 대상인지 확인하고 원본을 솔루션 직속에서 탐색하도록 요청했다. 이전 미커밋 변경을 기준으로 필요한 표시 등록만 교체했다. 구현 전 전체 변경 코드는 [PLAN](2026-10-12_DATA_SOLUTION_VISIBILITY_PLAN.md)에 보존했다.

현재 `Client > 96.DataFiles`는 root Data를 가리키던 가상 필터였다. 실제 `Client/Bin/DataFiles`와 `Server/Bin/DataFiles`는 publisher 출력이므로 원본과 합치거나 이름을 바꾸지 않았다. 파일 내용이 같을 수 있는 경우에도 원본과 게시본의 소유·검증·소비 시점은 구분한다.

- `Framework.sln` 직속 이름을 물리 폴더와 같은 `Data`로 정하고 실제 파일 2,400개를 SolutionItems 상대 링크로 연결했다. 루트 포함 솔루션 폴더 148개가 물리 하위 구조를 표현한다.
- 기존 Client의 Data None 1,658개(한 항목은 WorldHierarchy JSON wildcard), filters 연결 1,658개와 96.DataFiles 필터 정의 226개를 제거했다. 실제 Data를 이동·복사·삭제하지 않았다.
- 조사한 Data 2,412개 중 writer.lock 5개, tmp 2개, backup 4개, previous 1개는 표시에서만 제외하고 보존했다. 그 밖의 Git 추적·비무시 파일을 대상으로 한다.
- 기존 제품 4개와 하네스 6개는 그대로다. Data 폴더는 추가 C++ 빌드 프로젝트나 런타임 서비스가 아니다.
- `Tools/Build/data_solution.py`가 Data 목록을 결정적으로 생성한다. 같은 물리 상대 경로는 같은 UUID5를 사용하며 다른 솔루션 프로젝트·빌드 맵을 보존한다. `--check`는 쓰지 않고 최신 여부를 검사한다.
- 설치 도구는 `stage_data_solution(root, paths)`의 변경 후보를 자기 transaction에 합친다. 새 함수는 직접 쓰지 않는다. CLI는 교체 직전 원문 재확인 후 원자적으로 솔루션 파일 하나를 바꾼다.

## 직접 검증한 범위

| 확인 | 결과 |
|---|---|
| 초기 이관 전후 비Data 솔루션 등록·구성 비교 | 기존 실제 프로젝트 10개와 비Data Global 부분 보존 |
| Client XML 의미 비교 | 지정 Data None/Filter 외 항목·속성·값 동일 |
| 실제 Data 링크 | 2,400개 경로 존재, 중복 0, Client Data 중복 등록 0 |
| 폴더 GUID/부모 관계 | 148개 Data 폴더, 전체 GUID 158개 고유 |
| helper 격리 검사 | 8개 PASS: 멱등성·Git 선택·후보·경계·CAS·check 등 |
| MSBuild SolutionFile.Parse | PASS, native project 10개 + solution folder 148개 |
| MSBuild 속성 평가 | Client Debug x64 Application, 기존 Client/Bin/Debug/Client.exe 경로 유지 |
| 제품 helper --check | 종료 코드 0, 갱신 불필요 |
| 관련 Python 등록부·직접 회귀 | 아래 도구 이관 절의 8 + 17 + 3개 PASS |
| diff 공백 오류 | 변경 대상 git diff --check PASS |

독립 검토에서 기존 관리 폴더에 수동으로 넣은 `Data/../README.md` 같은 비Data 링크를 제거할 수 있는 후보 결함을 찾았다. 실제 반영 전에 canonical Data 하위 검증으로 거부하고 기존 파일을 보존하도록 교정했다. 최종 helper SHA-256은 `6c08a51019c4e04941bbeaafc20b8732e8763df03cb287c07d24935bbe2ae235`다.

검증 기록은 `out/DataSolutionMigration/visibility/verification.json`, `msbuild-verification.json`, 초기 원본·최종 후보 manifest에 있다. 이 out 경로들은 현재 PC의 검사 산출물이며 Git 전달 대상은 아니다.

C++·HLSL 컴파일 대상과 옵션의 의미 변경이 없어 제품 재빌드는 하지 않았다. MSBuild 속성 평가와 솔루션 파싱은 C++ 빌드 성공 증거로 표기하지 않는다. Data publisher·자산 생성기·Client·Server·Visual Studio UI는 실행하지 않았다. 최종 Visual Studio 트리의 화면 확인은 사용자가 솔루션을 다시 로드해 수행한다.

## 문서와 학습 연결

AGENTS·CLAUDE·팀 게임플레이 사용서·발탄 인수인계·밸런스 계약의 현재 탐색 안내를 새 Data 위치로 맞췄다. 기존 10월 7일 빈 DataFiles 폴더 제거 RESULT는 당시 기록으로 보존한다. 그 폴더는 SolutionItems/NestedProjects가 없었으므로 이번의 실제 원본 트리와 다르다.

[A00 실제 읽기](2026-10-12_DATA_DOMAIN_ARCHITECTURE_LEARNING_RESULT.md)는 현재 ConfigurationType XML, Engine import lib와 Shared static lib, Client의 명시적 Engine.lib 링크, CRT 이후 wWinMain/main, App의 초기화·반복·종료 책임을 실제 코드에 연결한다. A00 설명도 독립 검토했고 사용자의 이해·직접 실행 완료와는 구분했다.

Visual Studio에서 외부 변경 알림이 나타나면 솔루션을 다시 로드한다. 솔루션 직속 Data에서 실제 원본을 열 수 있다. 파일 추가·삭제 뒤 `python Tools/Build/data_solution.py`로 탐색 목록을 갱신하며 이 작업 자체는 데이터를 publish하지 않는다.

## 생성 도구의 Data 표시 등록 이관

실제 제품 반영 범위는 등록 도구 8개, 후보 receipt 필드 2개를 가진 생성 도구 1개, 직접 영향 계약 테스트 4개 파일이다. 기존 등록 함수 이름과 `(Path, before, after)` 후보 반환은 유지하고 `stage_data_solution`으로 솔루션 Data 링크 후보를 만든다. 각 호출자의 freshness·backup·원자 교체·rollback 경계는 기존 위치에 유지했다. Vehicle 등록 경계에는 교체 직전 솔루션 원문 일치 검사만 보강했다.

- 생성 도구의 무관한 최상위 함수 102개는 AST가 같고, 수정한 설치 함수의 metadata 등록 이외 statement도 같다. 데이터·shader 생성 내용 변경은 없다.
- 지정 설치 흐름과 wrapper의 외부 소비자 4개에는 정상 설치 중 기존 Data 원본을 삭제·이동하는 단계가 없다. 발견된 unlink는 임시 파일 정리 또는 이번 transaction의 신규 파일 rollback이다.
- `build_saydon_circus_world.py`의 후보 receipt는 `projectNoneInclude`/`projectFilter` 대신 `solutionItemPath`/`solutionFolder`로 현재 표시 계약을 설명한다. Tools의 해당 이전 필드 소비자는 검색에서 발견되지 않았다.
- 기존 Valtan 테스트의 세 번째 교체 실패 주입은 등록 파일 수 감소로 도달하지 않았으므로 실제 `Framework.sln` 교체 대상에서 실패하도록 바꿨다. 신규 metadata 제거, 기존 솔루션·비Data Client 항목 보존, 임시 파일 정리와 동시 편집 거부 검사를 유지했다.

검증: 제품 Python 13파일 py_compile 성공, helper와 등록부 연결의 임시 fixture 8개 PASS, 실제 제품 Valtan metadata 회귀 17개 PASS, 저장소의 Animation/Bern/World·Navigation 솔루션 등록 계약 3개 PASS, 담당 13파일 `git diff --check` PASS. 파일 hash·인코딩·개행 증거는 `out/DataSolutionMigration/tool-registration/verification.json`, `scope-verification.json`, `manifest.json`, `applied.json`에 있다.

검증은 등록 함수와 임시 metadata 설치 범위다. 생성기 main, 실제 자원 생성·설치, publisher, Client/UI는 실행하지 않았다. Visual Studio의 사용자가 보는 트리 상태는 별도 화면 확인 대상이다.

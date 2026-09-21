# 실행 데이터 전달과 publish·C++·셰이더 빌드 개선

## G00. 현재 기준과 완료 조건

시작 브랜치는 `pattern-bug-fix-bingo`, HEAD는 `7d5e27ea4`이며 미커밋 변경은 없다.
PR #427의 `Items.bootstrap`은 v2/46행이고, PR #428 뒤의 `CItemCatalog::Load()`와
`Publish-ItemCatalog.ps1`은 v4/7열을 사용한다. Valtan `ClearRewards.bootstrap`도
게시 v1/6행과 reader/publisher v2가 어긋난다. 현재 Server 실행 파일이 이 불일치로
listener를 열기 전에 종료한다. 두 authoring을 공식 publisher로 다시 생성하고
실제 Server의 제한 시간 headless 실행으로 world 초기화와 정상 종료를 확인한다.

이번 요청은 아이템 오류 복구, 게시 데이터 Git 전달 계약, navigation/world publish 및
C++/셰이더 증분 빌드의 한 변경이다. 기존 쿠크 publish 최적화와 shader 분할은 유지하며
실제 측정으로 남은 반복 비용을 확인한다. 전체 파일을 새 구조로 교체하거나 검증을 제거해
속도를 올리는 방식은 사용하지 않는다. Client/UI 실행과 최종 화면 확인은 사용자가 한다.

관련 기존 증거는 `08-21/2026-08-21_SERVER_ITEM_BOOTSTRAP_PREBUILD_FIX_RESULT.md`,
`09-09/2026-09-09_EFFECT_SHADER_BUILD_AND_PLAY_ALL_PERFORMANCE_RESULT.md`,
같은 날짜의 `2026-09-20_KOUKU_PUBLISH_AND_EFFECT_LOADING_RESULT.md`다.

## G01. 아이템 publisher와 Server 소비자

`Tools/GameplayPipeline/Publish-ItemCatalog.ps1`과
`Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1`은 현재 JSON을 검증해 v4와 v2를 생성한다.
두 Server bootstrap은 그 명령의 결과로만 갱신한다.
`Server/Private/ItemCatalog.cpp`, `ValtanClearRewards.cpp`는 잘못된 파일과 지원하지 않는 버전을 구분하고
실제 경로·기대/실제 버전·해당 publisher 명령을 출력한다. 이전 catalog 보존과 엄격한
행 검증은 유지한다. 기존 C++ 파일의 인코딩과 줄바꿈을 유지하며 신규 C++ 파일은 없다.
project/filter 등록을 추가할 필요가 없다.

publisher의 동일 출력은 파일 시각을 바꾸지 않도록 재사용하고, 변경 출력은 기존 파일의
최신성·원자적 교체·실패 보존을 유지한다. 재게시 비용과 첫 생성/재사용 결과를 측정한다.
공용 `Publish-FileTransaction.ps1`에 작은 텍스트 catalog의 검증/교체 경계를 연결하며
`CheckPublished`는 디렉터리/파일을 생성하지 않고 원본 projection과 현재 출력을 비교한다.
버전 상수를 빌드 스크립트에 복사하지 않는다. LF/CRLF는 동등하지만 BOM/손상/다른 행은 거절한다.
Server 직접 시작은 기존 `--headless --bind-address 127.0.0.1 --port 17777
--smoke-timeout-ms 1000` 경로로 제한하며 공유 endpoint를 바꾸지 않는다.

## G02. Git으로 전달하는 실행 데이터 계약

`AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/TEAM`의 현재 정본과
`.gitignore`/`.gitattributes`를 같은 정책으로 맞춘다. 검증된 제품 publisher 출력은
대응 원본 및 소비자 변경과 함께 Git으로 전달한다. receiver는 전달받은 출력이 현재
소비자와 맞으면 재게시하지 않고 빌드·실행한다. 새 원본 편집이나 형식 변경은 변경한
domain의 publisher를 실행한 담당자가 생성물까지 포함한다.

원본과 생성물의 역할, 생성물 수동 편집 금지, Resources의 별도 배포, EXE/DLL/OBJ/PCH,
EngineSDK 및 개인 receipt/cache의 Git 제외는 유지한다. 과거 RESULT는 당시 사실을
보존하고 현재 유효한 지침의 게시 데이터 커밋 금지 문구를 교정한다.
현재 bootstrap과 authoring이 참조하는 Valtan generation 1개는 유지한다. 전체 tracked
참조와 candidate 내부의 독립 사본을 검사해 확인한 과거 Git 사본 115개만 별도 백업 후
제거하고, 기존 untracked 4개 및 `Intermediate` 후보/rollback은 보존한다.

## G03. navigation/world publish 계산

`Tools/WorldPipeline`의 navigation publisher와 그 소비자가 같은 입력에 수행하는
텍스트 파싱·셀 검증·직렬화와 출력 쓰기 시간을 측정한다. PowerShell 셀별 pipeline과
전체 격자 재직렬화 비용을 줄이되 모든 셀 값, 경계/높이 검증, binary layout과
source revision, rollback을 보존한다. 실제 Area의 baseline/candidate를 별도 출력에서
비교하고 출력 동등성 및 기존 ContractTest로 판정한다.

## G04. C++와 셰이더 증분 빌드

`Tools/Build`와 Engine/Client 프로젝트의 실제 timing, OBJ/CSO 변경 수, SDK 복사 및
FXC tracking을 조사한다. 기존 PCH·MP 제한·Client FXC 병렬화·shader 구간 분할을
재사용한다. Engine의 남은 직렬 FXC 구간을 동일 tracking 아래 제한 병렬화하고,
무변경 Product 실행에서 불필요한 rebuild와 runtime publish가 발생하지 않는지 확인한다.
빠른 데이터 준비 검사는 누락뿐 아니라 소비자와 게시 형식 불일치를 밝혀야 한다.
`CppStandardPch.h`의 안정 표준 헤더에 filesystem/set/sstream을 추가해 각 TU의 반복
파싱을 줄인다. 변경이 잦은 GameInstance/packet/authoring 헤더는 PCH 밖에 유지한다.
PCH 생성 비용과 재사용 TU 비용을 별도로 재고 역순 A/B 및 제품 네 프로젝트의
실제 PCH/consumer 최소 컴파일을 확인한다. 한국어 MSBuild의 재컴파일 사유도 기존
`BuildIncrementalDiagnostics.psm1`에서 읽어 이유가 없는 rebuild로 오해하지 않게 한다.

## G05. 검증과 결과 기록

아이템 재게시 및 재사용, 실제 Server 초기화/정상 종료, 버전 불일치 실패와 기존 상태 보존,
navigation binary/semantic 동등성 및 invalid fixture 거절, 변경 프로젝트 XML/JSON/
PowerShell parse, 필요한 Server/Engine/Client 최소 빌드, 변경 후 무변경 증분 빌드,
`git diff --check`를 실행한다. 긴 전체 rebuild와 진단 하네스를 반복하지 않는다.
시간 비교는 같은 입력과 동일 범위를 사용하며 원본 재계산, 캐시 재사용, 실제 compiler
작업을 구분한다. 실제 실행 로그와 수동 미확인 범위는 대응 RESULT에 기록한다.

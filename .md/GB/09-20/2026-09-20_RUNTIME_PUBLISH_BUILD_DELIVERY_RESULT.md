# 실행 데이터 전달과 publish·C++·셰이더 빌드 개선 결과

## G00. 시작 기준과 Server 복구

`pattern-bug-fix-bingo`, 시작 HEAD `7d5e27ea4`에서 작업했다. 시작 시 미커밋 변경은 없었다.
작업은 2026-09-20에 시작했고 일부 검증은 KST 09-21 자정을 넘겨 수행했다.

PR #427이 Git으로 전달한 Items는 v2/46행, ClearRewards는 v1/6행이었다. PR #428의
현재 Server 소비자는 각각 v4/7열, v2/class별 행을 요구한다. 아이템만 재게시한 첫 실행은
다음 보상 형식 불일치를 드러냈고, 두 원본을 해당 publisher로 재게시한 뒤 정상화됐다.

| 파일 | 수정 후 실제 출력 |
|---|---|
| `Server/Bin/DataFiles/Items/Items.bootstrap` | `LOSTARK_ITEM_BOOTSTRAP`, v4, 46행 |
| `Server/Bin/DataFiles/Valtan/ClearRewards.bootstrap` | `LOSTARK_VALTAN_CLEAR_REWARDS_BOOTSTRAP`, v2, 24행 |

아이템 최초 복구는 343ms였다. 네비게이션 전체 재생성이나 Server의 JSON 직접 읽기는
필요하지 않았다. 기존 Debug Server의 `--headless --bind-address 127.0.0.1 --port 17777
--smoke-timeout-ms 1000` 실행은 모든 world simulation 초기화와 listener 생성 후 exit 0이었다.
사용자의 명시적인 실행 요청 뒤 제품 Server PID 68672를 `0.0.0.0:7777`로 열고 listener를
확인한 다음 Client PID 32764를 `LOSTARK_SERVER_HOST=192.168.0.14`로 열었다.
Client 화면 조작이나 시각적 성공 판정은 하지 않았다.

근거는 `out/RuntimePublishBuild20260920/server-initial-repair.log`, `server-repaired.log`,
`server-user-session.pid`, `client-user-session.pid`다. PID는 이 실행 당시의 값이다.

## G01. 현재 형식 검사와 원자적 게시

두 publisher에 `-Mode CheckPublished`를 추가했다. 각 publisher가 기존 원본 검증과
serialization을 그대로 사용해 현재 게시본과 비교하므로 별도 버전 상수를 복제하지 않는다.
검사는 read-only이며, 누락 경로에 디렉터리도 만들지 않는다. LF/CRLF는 동등하게 비교하고
BOM, 손상된 UTF-8, 다른 버전 및 같은 버전의 변경된 행은 거절한다.

두 publisher는 `Publish-FileTransaction.ps1`의 공용 작은 텍스트 catalog 경계를 사용한다.
동일한 출력은 mtime을 바꾸지 않는다. 변경 출력은 기존 destination mutex를 얻고 source
snapshot과 destination hash를 재확인한 뒤 `File.Replace`로 교체한다. 실패 전 원본은
유지한다. 성공 후 rollback 사본의 정리만 잠금으로 실패하면 남긴 경로를 경고하고 이미
성공한 교체를 실패로 오표시하지 않는다. 기존 손상 출력도 명시적 Publish로 복구할 수 있다.

`ItemCatalog.cpp`, `ValtanClearRewards.cpp`는 형식 오류와 버전 불일치를 구분하고
기대/실제 버전, 파일 경로와 해당 단일 publisher 명령을 출력한다. 현재 reader 버전과
행 검증을 낮추거나 legacy reader/fallback을 추가하지 않았다. 기존 ASCII/CRLF를 유지했으며
새 C++ 파일이나 project/filter 등록은 없다.

실제 두 CPP를 VS2022 Community/C++20으로 out driver와 컴파일·링크했다. 현재 생성물
Load, Item v2/Rewards v1 거절 진단, 실패 뒤 기존 item/starting equipment/reward 목록 보존,
정상본 복원 뒤 reload를 통과했다. 제품 EXE/DLL은 실행 중이므로 교체하지 않았다.
근거: `out/RuntimePublishBuild20260920/catalog-consumer-compile.log`,
`catalog-consumer-result.log`.

`Tools/GameplayPipeline/test_published_catalog_freshness.py`의 6개 테스트가 통과했다.
현재/재게시 mtime, missing read-only, stale schema/row/BOM/invalid UTF-8와 복구,
LF checkout, source 동시 수정 거절, 성공 뒤 backup 잠금 정리 경계를 검사한다.

## G02. Git 전달과 불필요한 게시 기록

AGENTS, CLAUDE, gotchas와 현재 TEAM 전달·Area·아키텍처·Valtan·gameplay 문서를
검증된 `Client/Bin/DataFiles`, `Server/Bin/DataFiles`를 같은 PR에 전달하는 정책으로 통일했다.
다른 PC는 맞는 snapshot을 받았다면 매 pull마다 모든 publisher나 navigation bake를
반복하지 않는다. schema를 바꾼 작성자가 해당 domain 생성물까지 갱신한다.
EXE/DLL/OBJ/PCH, Resources, EngineSDK, 개인 cache 및 staging/rollback은 계속 제외한다.
생성물을 손으로 고치는 금지와 Data 원본의 저작 책임도 유지했다.

`.gitignore`를 실제로 이미 추적되던 PR #427의 정책에 맞춰 신규 게시 파일도 일반
`git add`로 노출되게 했다. `.navgrid`는 binary `-text`를 명시했고 기존 Map LFS 규칙은 유지했다.
과거 PLAN/RESULT의 당시 설명은 역사적 증거로 보존했다.

Valtan generation 폴더 120개 중 현재 bootstrap/authoring과 일치하는 것은
`7d56442513800b7862d58b2cffe330b828a2d7c7ae50f5a945d2882e43b85a05.json` 1개다.
전체 tracked 참조, 테스트, candidate 사본/소비 경로를 조사하고 현재 소비하지 않는
tracked 사본 115개(2,839,154bytes)를 out에 백업한 뒤 제거했다. 기존 untracked 4개와
`Intermediate/ValtanTuningCandidates`의 독립 복구 사본은 변경하지 않았다.
이 정리는 배포 불필요 파일 제거이며 runtime 속도 향상으로 계상하지 않는다.

근거: `out/published-generation-reference-audit.json`,
`out/RuntimePublishBuild20260920/obsolete-generation-cleanup.json`, `obsolete-generations-backup/`.
제품 Server가 실행 중인 뒤 추가 Debug Server smoke는 기존 activation mutex가 정상 거절했다.
이를 초기화 실패 회귀로 보거나 두 번째 startup 성공으로 기록하지 않는다.

## G03. navigation 게시 비용

`Publish-ServerNavigation.ps1`의 셀별 숫자 파싱·binary 직렬화·인접 높이·연결성 순회를
같은 스크립트의 C# helper로 옮겼다. paint/region/placement 검증과 Area transaction은 유지한다.
새 외부 executable, cache나 navigation 형식은 추가하지 않았다.

| 같은 입력과 범위 | 기존 | 변경 후 |
|---|---:|---:|
| 전체 6 Area + 7 detail grid, 1,395,182셀 게시 | 142.809초 | 7.612초 |
| 같은 변경 후 반복 | — | 7.072초 |

Server/Client 84개 출력의 SHA256가 전후 및 반복 실행에서 모두 일치했다. ContractTest는
v1/v2, quoted token, 순서 독립성, duplicate/out-of-range/invalid flags/nonfinite/truncated row,
unresolved height, disconnected row-wrap/no-walkable, paint와 placement 경계를 통과했다.
WorldGameplay Validate는 5.936초 PASS였으며 추가 병목 증거 없이 해당 코드까지 바꾸지 않았다.
모든 성능 실험 출력은 `out/NavigationPublishPerf20260920/` 아래에만 생성했다.

## G04. 셰이더 병렬 처리와 build 준비 검사

Engine FXC는 직렬이고 Client만 4병렬인 실제 MSBuild 평가를 확인했다.
`ProductToolchain.props`로 두 프로젝트의 FXC 기본값을 통일하고 Client 중복 선언을 제거했다.
명시 override, 기존 PCH·C++ `/MP8`, `UseMultiToolTask=false`, SDK 및 FX tracking은 유지한다.

| 같은 Engine shader 9개, 격리 출력 | 실행 시간 |
|---|---:|
| 기존 직렬 | 246.416초 |
| FXC 4병렬 | 84.885초 |
| 변경 없는 재실행 | 0.257초, CSO 쓰기 0 |
| 출력 하나 누락 후 해당 파일 복구 | 0.370초 |

8개 CSO는 byte-exact다. Debug Shader_Cell의 두 embedded DXBC는 SPDB/debug checksum만
다르고 실제 instruction/reflection/signature 및 나머지 effect bytes는 동일했다.
기존 X4000 경고는 유지했고 컴파일 오류는 없었다. 기본 제품 파일은 교체하지 않았다.
근거: `out/BuildPipelineOptimization20260920/`의 `fxc-benchmark.json`,
`fxc-output-comparison.json`, `parallel-nochange.json`, `parallel-missing-output.json`.

Product runner는 파일 존재 검사 뒤 두 catalog의 CheckPublished를 같은 PowerShell host에서
호출한다. compile 결과와 `invalidRuntimeInputs`/`runtimeDataChecks`를 구분한다.
실패한 첫 catalog 때문에 나머지 검사를 건너뛰지 않으며 runtime 데이터를 게시하지 않는다.
rewards domain의 tool dependency에도 공용 transaction helper를 추가했다.
마지막 read-only Product `-SkipBuild`는 302ms, Item 121ms/Rewards 33ms PASS,
missing/invalid 0이었다. 이 수치는 C++ build 시간이 아니다.

## G05. 검증 경계와 사용자 작업 보존

PowerShell AST, 변경 프로젝트 XML/manifest JSON, build focused 검사 6개, 실제 두 CPP
최소 컴파일, publisher 회귀 6개, navigation ContractTest/출력 동등성, `git diff --check`를
확인했다. Client/Server를 연 뒤 기본 제품 C++ 전체 링크나 full diagnostic은 실행하지 않았다.
실행 중 작성한 사용자의 RenderingProfiles 원본/runtime 변경은 이번 작업으로 덮거나 정리하지 않는다.

최근 C++ 대량 빌드에서 Client의 217개 OBJ가 실제 갱신된 기록은 확인했지만, 이 기록을
불필요한 shader 재컴파일로 해석하지 않는다. 기존 PCH/MP를 이번 새 개선이라고 계상하지 않는다.

## G06. C++ 반복 헤더 파싱과 재컴파일 이유

최근 실제 diagnostic의 217개 OBJ는 수정 의존성 216개와 새 `ClientWindowDisplay.cpp`
tracking 생성 1개로 설명된다. 수정 이유는 GameInstance.h 105개, PacketMessages.h 78개,
PacketType.h 13개, ArenaCameraProfile.h 6개, 직접 CPP/UI 헤더 수정 14개다. 당시 toolchain
상태가 동일한 것도 확인했다. 캐시 실패나 매번 무조건 전체 compile이라는 근거는 없다.

기존 사유 parser가 한국어 `컴파일됩니다` 기록을 놓쳐 receipt의 reasons가 비어 있었다.
`BuildIncrementalDiagnostics.psm1`은 이 문구와 대응 영어 이유도 기존 bounded 기록에 포함한다.
진단 개선은 컴파일 시간 감소로 세지 않는다.

실제 Client 의존성에서 filesystem 285개, sstream 291개, set 149개 TU가 해당 표준 헤더를
PCH 밖에서 반복 파싱했다. `CppStandardPch.h`에 세 헤더만 추가했다. 변동이 잦은 게임/
패킷/저작 헤더는 PCH에 추가하지 않았다.

같은 compiler/options/source의 격리 PCH 및 CPP 4개를 기존→후보와 후보→기존 순서로
컴파일했다. 아래는 warm 역순 비교이며 프로젝트 전체 build 시간으로 확대하지 않는다.

| 대표 실제 CPP | 기존 | 변경 후 |
|---|---:|---:|
| CameraTool | 3.949초 | 3.107초 |
| MapAuthoringHost | 3.675초 | 2.694초 |
| ValtanActionWorkbench | 4.561초 | 3.595초 |
| EffectV2_Document | 1.968초 | 1.250초 |
| 합계 | 14.153초 | 10.646초 |

첫 PCH 생성은 약 0.700→1.525초, 크기는 약 79→146MB로 늘었다. 이 비용은 PCH를
생성할 때 발생하고 이후 TU가 재사용한다. 변경 commit을 처음 빌드할 때 관련 OBJ가 한 번
재컴파일되는 것이 정상이다. 실행 중 제품의 기본 PCH/OBJ/PDB/EXE는 쓰지 않았다.

Engine/Shared/Server/Client 각각의 실제 PCH와 대표 consumer를 Debug/Release로
컴파일한 16건이 모두 exit 0이었다. 이는 최소 compile 호환성 검증이며 제품 전체 link나
Client 화면 검증을 대신하지 않는다. 근거는 `out/BuildPipelineOptimization20260920/`
`cpp-rebuild-reasons.json`, `cpp-pch/results.json`, `warm-results.json`,
`product-compatibility.json`, `release-compatibility.json`다.

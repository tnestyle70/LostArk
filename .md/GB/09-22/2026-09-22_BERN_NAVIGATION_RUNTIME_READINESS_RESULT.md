# Bern Navigation 런타임 준비 검사 RESULT

## G01 완료 범위

작업 브랜치 `codex/bern-navigation-runtime-fix`, 시작 정본 `4044a7893`에서 기존 Product 준비 검사를 확장했다. `0ebd23cd1` 이후 Navigation 원본·publisher·게시 파일의 변경은 없음을 확인했다. 첫 명령의 LAN sync는 client / 192.168.0.14 / not-listening으로 완료했다.

`Test-ProductNavigationInputs`는 기존 `Invoke-BuildAndRegression.ps1` 내부 함수다. BuildDomains의 필수 기본 grid와 양쪽 runtime `.navregions`에서 선언한 세부 grid를 읽는다. grid 20-byte header·유효 크기·정확한 payload 길이, 필수 policy/blocker 파일의 존재·header·ID·grid/step 일치를 확인한다. 오류는 기존 `runtimeDataChecks`와 `invalidRuntimeInputs`에 파일 경로와 함께 기록하며, 다른 grid 검사를 계속한다. 자동 publish나 새 validator/harness, C++/HLSL 변경은 없다. 일반 Product의 컴파일 PASS와 준비 상태 분리는 유지한다.

## G01 확인된 전달 누락

`667886ffb`는 Bern3 원본과 authoring 영역 목록을 추가했다. `4b7fdaf25`는 Client/Server runtime 영역 목록을 3개로 바꿨지만 Bern3 runtime 파일은 Git에 포함하지 않았다. 저장소 전체 이력에서도 해당 Bern3 runtime 파일이 없었다. 당시 `.gitignore`는 이미 DataFiles를 허용했으므로 Git 제외 규칙을 누락 원인으로 기록하지 않는다. Product의 이전 필수 파일 목록은 기본 grid만 확인했고 세부 목록의 참조를 검사하지 않았다.

## G01 수정 파일

- `Tools/Build/Invoke-BuildAndRegression.ps1`: 기존 Product 준비 검사 연결과 파일별 진단.
- `Tools/Build/test_build_profile_contract.py`: 기존 테스트 클래스에 정상·누락·잘림·sidecar·manifest 사례 추가.
- `Tools/Build/README.md`, `CLAUDE.md`, `AGENTS.md`: 통합 담당자가 현재 읽기 전용 검사 범위만 좁게 갱신.
- `.md/GB/gotchas.md`: 반복 방지 절을 기존 byte prefix 보존 append.
- 같은 주제 PLAN/RESULT: 정확한 반영 코드와 검증 증거.
- 공식 Bern 게시 runtime 파일은 통합 담당자가 별도로 준비했다. C++ project/filter 변경은 없다.

## G01 실행한 검증

1. 기존 build-profile 테스트의 Navigation 메서드: 정상, 세부 grid 누락, header 잘림, payload 잘림, 기본 grid 누락, policy 누락, blockers 누락, policy 불일치, 안전하지 않은 region ID, manifest 잘림의 10개 subcase PASS. 좌표는 0이 아닌 큰 소수 원점 `1000.1239/-250.9876`을 사용했다. 모든 사례에서 검사 전후 fixture bytes가 동일했다.
2. 기존 Item/Valtan reward 준비 검사 테스트도 함께 PASS. 수정 검사 때문에 기존 catalog 실패 보고나 무게시 계약이 변하지 않았다.
3. 실제 현재 worktree의 게시 Navigation 34개 grid에 같은 함수를 실행해 34 PASS / 0 FAIL. 증거: `out/BernNavigationReadiness20260922/actual-runtime-checks.json`.
4. 공식 게시 Bern 30파일의 사본에 같은 함수를 실행했다. 정상 8grid PASS, 실제 Bern3 grid 누락은 해당 파일 1 FAIL, 마지막 byte를 자른 Bern3 grid도 해당 파일 1 FAIL. 원본 30개 SHA256은 전후 동일하다. 증거: `out/BernNavigationReadiness20260922/actual-bern-cases.receipt.json`.
5. 최초 실물 검사에서 double/text와 float/binary 원점 비교가 16개 오탐을 드러냈다. 임계값을 느슨하게 하지 않고 Server와 동일한 float32 파싱으로 교정한 뒤 전체 34개를 다시 통과했다.
6. PowerShell AST parse, 변경 Python compile, 관련 `git diff --check` PASS. 전체 C++/셰이더 빌드는 하지 않았다.

## G01 공식 게시와 실제 Server 확인

통합 담당자는 원본 폴더에서 공식 Bern Publish를 실행해 30개 출력 중 기존 23개 byte 동일·누락 7개 신규를 확인했다. 격리 worktree에도 같은 공식 게시를 실행했고 원본과 30개 SHA256이 일치한다. worktree Git 신규 출력은 10개다: Bern3의 Client3/Server4 파일 7개와, 원본에는 이미 있었으나 Git에는 없던 base/Bern/Bern2의 Server `.navsurface` 3개다. 새 출력 수와 원본 PC에 새로 생긴 파일 수를 구분한다.

통합 담당자의 별도 실제 Server 확인은 `C:/Users/user/Desktop/LostArk/Server/Bin/Debug/Server.exe --headless --bind-address 127.0.0.1 --port 27777 --smoke-timeout-ms 500`으로 6world 초기화·listen·exit0, timeout 없음·프로세스 종료를 확인했다. 증거는 원본 `out/BernNavigationRepair20260922/server-check.receipt.json`과 `all-world-startup.stdout.log`다. 이 에이전트는 Server/Client를 실행하지 않았다.

## G01 검사의 한계와 다음 반영

이번 Product 검사는 참조·파일 길이·sidecar header 준비 검사다. 모든 cell의 walkable/높이, blocker body 의미, 층 겹침, world spawn 정합성 및 원본 최신성을 검사하는 전체 publisher/Server 검증은 대체하지 않는다. `.navsurface`는 기존 Server의 optional 입력 계약을 유지한다. Client UI와 사용자의 플레이 화면은 실행·판정하지 않았다. 원본 작업 폴더에는 Tool·public 안내·PLAN/RESULT 등 8파일을 기존 원본과의 일치 및 SHA 재확인 후 백업·원자 교체로 반영했다. gotchas의 기존 사용자 변경은 byte prefix를 보존해 새 절만 추가했다. 원본 Git index는 변경하지 않았고, 별도 기능 브랜치로 전달한다. 설치 증거는 원본 out/BernNavigationRepair20260922/source-installed.receipt.json이다.

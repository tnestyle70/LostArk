# Server Item Bootstrap Pre-build 수정 구현 계획

작성일: 2026-08-21

## 목표

현재 `origin/main`과 같은 코드 기준에서 Visual Studio로 Server를 직접 빌드·실행해도
`Data/Items/ItemCatalog.json`이 `Server/Bin/DataFiles/Items/Items.bootstrap`으로 publish되어
`Missing item bootstrap`으로 world simulation이 종료되지 않게 한다.

## 현재 실측과 변경 범위

- 작업 시작 HEAD `6d192e98585c33f355274155e69783a4045a6e1a`는 fetch 뒤 `origin/main`과 같다.
- `CItemCatalog::Load()`는 모든 world room 초기화에서 `Items.bootstrap`을 필수로 읽고,
  `CServerApp::Run()`은 room stage가 끝난 뒤에야 TCP listener를 연다. 따라서 보고된 종료는
  네트워크 오류가 아니라 listener 이전 runtime data 생성 누락이다.
- item authoring과 `Publish-ItemCatalog.ps1`은 7개 item을 정상 검증하지만
  `Server.vcxproj`의 `PublishGameplayRuntime` target이 publisher를 호출하지 않는다.
- `Publish-BalanceRuntimeSet.ps1`은 item을 포함한 runtime output 6개를 promotion하지만
  failure injection 범위는 아직 0~5라 마지막 item promotion rollback을 검증할 수 없다.
- `Server/Bin`은 Git 제외 runtime 출력이므로 생성물을 커밋하거나 Server가 authoring JSON을
  직접 읽는 fallback을 추가하지 않는다.
- 기존 미커밋 `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.*.json` 두 파일과 LAN endpoint
  계약은 수정하지 않는다.

## 구현 순서

1. `Server/Default/Server.vcxproj`의 기존 `PublishGameplayRuntime` target에서
   `Publish-ItemCatalog.ps1 -Mode Publish`를 호출한다. 기존 world publisher는 optional
   spawn-group runtime 출력도 소유하므로 통합 publisher로 대체하지 않는다.
2. `Publish-BalanceRuntimeSet.ps1`의 `FailureAfterPromote` 범위를 0~6으로 맞춘다.
3. `CLAUDE.md`와 `.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`의 item authoring,
   pre-build, runtime output 6개와 rollback 계약을 실제 구현에 맞게 교정한다.

## 검증

- item catalog와 balance runtime set `Validate`
- repository 내부 임시 OutputRoot에서 `FailureAfterPromote 6`을 실행해 기존 6개 hash 복원과
  staging/rollback 잔재 0건 확인
- 기존 `Items.bootstrap`이 없는 상태에서 Server x64 Debug build 후 파일 생성·header/row 수 확인
- Server project XML parse
- `Server.exe --contract-test`
- bounded Server startup에서 world simulation 초기화, listener 생성, exit code 0 확인
- 종료 뒤 잔류 Server process/listener 없음, `git diff --check`

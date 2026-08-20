# Server Item Bootstrap Pre-build 수정 결과

작성일: 2026-08-21

대응 계획서:
`.md/GB/08-21/2026-08-21_SERVER_ITEM_BOOTSTRAP_PREBUILD_FIX_IMPLEMENTATION_PLAN.md`

## 구현 상태

- 작업 시작 HEAD `6d192e98585c33f355274155e69783a4045a6e1a`가 fetch 뒤 `origin/main`과
  같은 것을 확인하고 `codex/server-bootstrap-lan-0821` 브랜치에서 작업했다.
- `Server/Default/Server.vcxproj`의 `PublishGameplayRuntime` target에
  `Publish-ItemCatalog.ps1 -Mode Publish`를 연결했다.
- `Publish-BalanceRuntimeSet.ps1`의 output이 gameplay 1 + world 4 + item 1의 6개가 된 실제
  상태에 맞춰 `FailureAfterPromote` 허용 범위를 0~6으로 교정했다.
- item authoring, pre-build, runtime bootstrap과 6-output rollback 계약을 `AGENTS.md`,
  `CLAUDE.md`와 관련 TEAM 문서에 반영했다.
- 기존 world publisher를 통합 publisher로 교체하지 않았다. 기존 경로가 생성하는 optional
  `*.spawngroupsbootstrap`을 계속 보존한다.
- 사용자 기존 미커밋 Valtan world destruction JSON 두 파일과 LAN endpoint 계약은 작업 범위에서
  제외했다.

## 원인

`Data/Items/ItemCatalog.json`과 publisher는 존재했지만 Server project pre-build가 item publisher를
호출하지 않았다. `Server/Bin`은 Git 제외 runtime 출력이므로 clean 또는 오래 사용하지 않은
worktree에는 `Items.bootstrap`이 없었다.

Server는 `CServerApp::Run()`에서 listener를 열기 전에 BERN 등 world room을 stage하고,
`CGameRoom`의 `CItemCatalog::Load()`가 `Server/Bin/DataFiles/Items/Items.bootstrap`을 필수로 읽는다.
따라서 누락 시 네트워크 bind 이전에 `World simulation failed to initialize`와 exit code 1로 종료됐다.

## 자동 검증

### 통과

- 수정 전 runtime 상태: `Items.bootstrap=False`, TCP 7777 listener 0, Server process 0
- Server x64 Debug build: PASS
  - pre-build 출력: `Item catalog Publish succeeded: 7 items`
  - 생성 header: `LOSTARK_ITEM_BOOTSTRAP\t2\t7`
  - 실제 data row: 7
  - `Server.exe` compile/link: PASS
- 생성된 bootstrap만 다시 삭제한 뒤 소스가 up-to-date인 증분 Server build: PASS
  - item publisher 재실행과 7-item bootstrap 재생성 확인
  - `Server.vcxproj` up-to-date build 완료
- item catalog Validate: `7 items` PASS
- balance runtime set Validate: PASS
- `FailureAfterPromote 6` rollback fixture:
  - 예상 injected failure 확인
  - 기존 output hash 6개 복원 PASS
  - staging/rollback 잔재 0
  - 임시 fixture 삭제 확인
- JSON, Server project XML, PowerShell AST parse: PASS
- bounded Server startup:
  - `Listening on 127.0.0.1:7777 ...`
  - exit code 0
  - 종료 뒤 Server process 0, TCP 7777 listener 0
- `git diff --check`: PASS

### 통과하지 않은 전체 회귀

fresh Server build 뒤 `Server.exe --contract-test`는 다음 기존 gameplay contract 1건 때문에
exit code 1이었다.

```text
[FAILURE] Accept one exact ACTIVE axe wall-contact row
failures : 1
```

이번 변경은 Server C++ gameplay, Valtan data와 wall-contact publisher를 수정하지 않는다.
`Items.bootstrap` 누락 재현과 정상 Server startup은 별도로 닫혔지만, 위 범위 밖 current-main
contract mismatch를 전체 PASS로 기록하지 않는다.

## 수동 검증 경계

Client/UI는 실행하지 않았다. 사용자가 Visual Studio project를 Reload하거나 다시 연 뒤 Server를
빌드·실행하면 pre-build가 item bootstrap을 먼저 생성한다. 화면·Valtan 진입은 사용자 판정 범위다.

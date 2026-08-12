# 2026-08-13 Worktree Consolidation / Main Promotion Plan

## 0. 목적

96개 등록 worktree를 모두 감사하되, 현재 `main`에 없는 **유효한 고유 계약만** 하나의 통합 PR로 승격한다. 이미 동등 패치가 있는 review 복제본, 폐기된 Artist F executor, 구형 network/Character Select 구현, stale generated catalog는 병합하지 않는다. PR 병합 뒤 모든 후속 Artist F family/canary 작업은 새 `origin/main`에서 시작한다.

이 계획은 worktree 삭제나 branch 삭제를 허가하지 않는다. 다른 담당자의 미커밋 변경도 정리하거나 되돌리지 않는다.

## 1. 기준선과 보존 경계

- 1차 감사 기준: `origin/main=8d76693e03f4c4c0c3939c7b71b9e3de8680441a`
- 실제 통합 시작 기준: `origin/main=eaaf772814e78ddcfc4a4dfb46b3c32cc6337bb2`
- 통합 branch: `codex/worktree-consolidation-20260813`
- 통합 worktree: `C:/Users/user/.codex/worktrees/worktree-consolidation-20260813/LostArk`
- 현재 Artist F captain의 dirty slice와 새 main이 동시에 바꾼 파일은 다음 6개다.
  - `CLAUDE.md`
  - `Client/Private/Loader.cpp`
  - `Client/Private/MainApp.cpp`
  - `Engine/Private/Model.cpp`
  - `Engine/Public/Model.h`
  - `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- 위 6개는 file-wide `ours/theirs`로 해결하지 않는다. 새 main의 NPC/Lance/Valtan 계약과 Artist F 계약을 실제 호출자 단위로 함께 보존한다.
- `Data/Animation/Authored/Warlord/Warlord.animevents`와 `Tools/ModelAssetConverter/__pycache__/`는 통합 대상이 아니다.

## 2. 감사 결론

| 분류 | 수량/대상 | 처리 |
|---|---:|---|
| 등록 worktree | 96 | 전수 감사 기록 |
| 이미 main 포함 | 58 | 병합하지 않음 |
| detached review 복제/ancestor | 13 | 병합하지 않음 |
| patch-equivalent named branch | 19 | 병합하지 않음 |
| Artist F material authority | `acf6a994..fee93749` 4 commits | 승격 |
| R8 offline raw inventory | `b765fcb8`, `9b7ac2dc` | project visibility를 닫은 뒤 승격 |
| 현재 Artist F Track A/runtime slice | captain dirty worktree | fee 30파일과 사용자 변경을 제외해 한 계약으로 승격 |
| typed source runtime materializer | `4ffe1102` | 기존 PLAN의 폐기 판정에 따라 제외 |
| Character Select server unification | `f7607be5` | 현 protocol v15보다 오래된 회귀성 구현이므로 제외 |
| detached `43e5` | Aug-02 network/server WIP | legacy quarantine, 제외 |
| temp `artist-f-old10` | stale generated catalog | main 이력과 동일하므로 제외 |

## 3. 구현 게이트

### G00. 최신 main 동기화 / topology 재확인

1. `git fetch --prune` 후 실제 통합 기준 SHA를 고정한다.
2. 각 후보에 `git cherry`와 `git merge-tree`를 다시 실행한다.
3. 새 main과 dirty slice의 path overlap을 고정하고 충돌 해결 대상을 제한한다.

### G01. Artist F 현재 계약 봉인

1. current captain에서 material-authority 30파일, Warlord 이벤트, build/intermediate를 제외한다.
2. Track A ShaderMap/DXBC replay, runtime parity, actual first-draw 관련 코드·데이터·문서·harness만 stage한다.
3. deep focused gate를 재실행하고 하나의 검증 가능한 commit으로 봉인한다.

### G02. 유효 worktree 계약 통합

1. material-authority 4커밋을 통합 branch에 merge한다.
2. G01 commit이 포함된 captain branch를 merge한다.
3. 겹치는 6파일은 최신 main과 Artist F 양쪽 계약을 semantic merge한다.
4. R8 두 커밋을 merge하고 `Client.vcxproj/.filters`의 `96.DataFiles` 등록을 교정한다.
5. 제외 대상은 RESULT에 commit/branch와 근거를 기록한다.

### G03. 자동 검증

최소 검증은 다음 순서로 실행한다.

1. JSON/XML parse와 R8 Python unit/validator
2. Artist F main ShaderMap identity / original DXBC / runtime source replay / runtime material v2
3. Engine Debug/Release, `UpdateLib.bat` Debug/Release, 필요한 harness/Client Debug/Release
4. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
5. `git diff --check`

Client나 Effect Tool은 에이전트가 실행·조작하지 않는다. 화면 PASS는 사용자 판정으로만 남긴다.

### G04. Main 승격 / 재동기화

1. 검증된 통합 branch를 push하고 PR로 `main`에 병합한다.
2. 병합 직전에 `origin/main`이 다시 전진하면 재동기화하고 G03을 반복한다.
3. 병합 뒤 local `main`과 Artist F 후속 worktree를 새 `origin/main`으로 동기화한다.
4. 새 `codex/artist-f-family-canaries-20260813` branch/worktree에서 Artist F G01 cache coverage부터 재개한다.

## 4. 완료 조건

- 유효한 고유 계약만 main ancestry에 존재한다.
- 중복·폐기·구형 worktree의 제외 근거가 RESULT에 남는다.
- Warlord 사용자 변경과 build/intermediate가 commit에 포함되지 않는다.
- R8 원본 JSON은 `96.DataFiles`에 노출되고 validator가 통과한다.
- 최신 main의 NPC/Lance/Valtan 변경과 Artist F runtime 양쪽 focused gate가 통과한다.
- PR 병합 SHA와 후속 Artist F branch 기준 SHA가 RESULT에 기록된다.

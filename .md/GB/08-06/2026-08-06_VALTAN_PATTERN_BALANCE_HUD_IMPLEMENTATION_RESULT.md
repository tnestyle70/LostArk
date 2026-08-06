# LostArk 발탄 패턴 Balance Tool·서버 권위·Font HUD 구현 결과

- 작성일: 2026-08-06
- 브랜치: `codex/effect-tool-reboot`
- 기준 커밋: `6267819` (`origin/main` fast-forward 반영 뒤 작업)
- 대응 계획: `.md/GB/07-31/2026-07-31_LOSTARK_VALTAN_TUNING_DIRECTION_PLAN.md`
- 구현 판정: 1차 수직 슬라이스 완료
- 전체 ProjectAudit 판정: 동시 작업 중인 Effect 변경 3건 때문에 전체 PASS 미달. 발탄/world/HUD 신규 check는 PASS.

## 1. 완료한 계약

### 1.1 데이터와 Balance Tool

- `BossProfiles.json`을 formatVersion 3으로 올리고 발탄의 `maximumHealthBars=160`을 정본화했다.
- `ValtanEncounter.json`을 formatVersion 2로 올리고 아래 10개 pattern을 등록했다.
  - 일반 pool: 돌진, 4방향 벽 생성, 점프 후 도끼 투척, 회전 원형 공격, 3연속 공격
  - HP 줄 queue: 130줄 6방향, 130줄 전방향 즉사, 80줄 지형 파괴, 76줄 무력화, 33줄 지형 파괴
- pattern별로 selection mode, 줄 범위/trigger/order, weight, 최대 연속 사용, 선택 거리,
  telegraph/active/recovery, Server hit shape/radius/count/interval, 독립 damage profile을 저장한다.
- `sourceActionId/sourceStage`는 원본 Action 근거 metadata다. 확정하지 못한 76줄 Action과
  130줄 세부 Stage는 0/빈 값으로 보존해 추측 binding을 만들지 않았다.
- Balance Tool에서 위 필드를 편집하고 live boss 줄 gate, weight 비율, threshold armed/reached,
  현재 Server snapshot에서 선택된 semantic action을 확인할 수 있다.
- Save는 기존 원자적 staging/rollback을 유지하고 provenance 동기화 뒤 publish한다.

### 1.2 publish와 Server authority

- `Gameplay.bootstrap` v2에 BOSS 160줄과 10개 PATTERN row를 생성한다.
- `World.bootstrap` v6은 boss placement와 encounter ID만 소유한다. 이전의 첫 pattern 복제 필드를 제거했다.
- `CGameplayCatalog`가 pattern row를 parse/validate하고 실패 시 이전 commit을 복원한다.
- HP 줄 계산은 `ceil(currentHp * maximumHealthBars / maximumHp)`이고 0 HP는 0줄이다.
- 한 tick에 여러 줄을 넘으면 높은 줄부터 queue하며 같은 130줄은 `triggerOrder=1` 6방향,
  `triggerOrder=2` 전방향 즉사 순서다. trigger pattern은 encounter당 한 번만 queue한다.
- 일반 pattern은 현재 줄, target 거리, 최대 연속 사용으로 후보를 제거한 뒤 weight로 결정적으로 선택한다.
- `CIRCLE`은 Server XZ 평면 반경 판정이며 hit count/interval에 따라 여러 번 피해를 적용한다.
  `NONE`인 76줄 무력화 상태는 플레이어 피해를 만들지 않는다.

### 1.3 제품 HUD

- 좌하단 `##RuntimeCombatHUD` ImGui 창과 HP/Mana ProgressBar, skill text를 제거했다.
- authored HUD 이미지 위에 Font Manager `Font_YG330`으로 다음 숫자만 표시한다.
  - HP: 현재 / 최대
  - MANA: 현재 / 최대
- 발탄 snapshot이 유효하면 화면 상단에 Font Manager로 이름, 현재/최대 줄, 현재/최대 HP를 표시한다.
- 줄 수는 Client가 임의 상태를 만들지 않고 replicated current/max HP와 검증된 boss profile의
  maximumHealthBars로 계산한다.

## 2. 실제 수정 경계

- Client: `BalanceTool.h/.cpp`, `CombatHUDViewModel.h/.cpp`, `MainApp.h/.cpp`
- Server: `GameplayCatalog.h/.cpp`, `WorldBootstrap.h/.cpp`, `ServerWorldEntity.h`,
  `GameRoom.cpp`, `ValtanBrain.cpp`, `ServerGameplayContractTests.cpp`
- Data: `BossProfiles.json`, `DamageProfiles.json`, `ValtanEncounter.json`, provenance receipt
- Tools: gameplay/world publisher, provenance exporter/updater, ProjectAudit
- 문서: 기존 발탄 방향 PLAN 갱신, 이 RESULT 추가

## 3. 자동 검증 결과

### 3.1 데이터/publish

- `Update-BalanceProvenanceReceipt.ps1`: PASS, 신규/변경 field 240개를 `PROJECT_TUNED`로 동기화
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
  - player profile 5
  - runtime skill row 88
  - damage profile 71
  - boss 1
  - boss pattern 10
- `Publish-BalanceRuntimeSet.ps1 -Mode Validate`: PASS
- `Publish-BalanceRuntimeSet.ps1 -Mode Publish`: PASS
- world validate/publish: BERN 7, VALTAN_ARENA 5, TRAINING_GROUND 4,
  CHARACTER_SELECT_ARENA 5 placements PASS

### 3.2 Debug

- Engine x64 Debug: PASS
- UpdateLib Debug: PASS
- Shared/NetworkProtocolHarness/ClientFrontendHarness x64 Debug: PASS
- Server x64 Debug + `--contract-test`: PASS, failures 0
- Client x64 Debug: PASS
- 추가 Server rollback contract:
  - 130줄 두 pattern order queue PASS
  - 130줄 원형 피해 1회 및 damage event PASS
  - corrupt Gameplay.bootstrap 거부 뒤 기존 10 pattern catalog 보존 PASS
- 실제 시작 smoke: Debug Server와 Client를 정본 working directory에서 실행하고 10초 뒤
  Server/Client process 생존과 `Client` window title 확인 PASS. 확인 뒤 시작한 process만 종료했다.

### 3.3 Release

- Engine/UpdateLib/Shared/NetworkProtocolHarness/ClientFrontendHarness/Server/Client x64 Release: PASS
- Server Release `--contract-test`: PASS, failures 0
- gameplay/world publish와 navigation validate: PASS

### 3.4 ProjectAudit

Release report에서 이번 변경과 직접 연결된 check는 모두 PASS다.

- `server.world-bootstrap-boundary`: PASS
- `gameplay.valtan-health-bar-pattern-contract`: PASS
- `ui.combat-font-hud-contract`: PASS

전체 ProjectAudit는 아래 동시 Effect 작업의 dirty worktree 때문에 3건 실패했다.

- `projects.data-source-visibility`: expected 220, project/filters 218
- `effect.g09-authoring-world-runtime-boundary`
- `effect.g09-cross-document-contract`: `effect.dimensionmaster.skill.2050500.effect.json` invalid

해당 Effect C++/JSON/project 변경은 이 작업 시작 뒤 다른 세션에서 생성됐고 이 변경 단위에서
되돌리거나 보정하지 않았다. 따라서 전체 ProjectAudit PASS라고 기록하지 않는다.

## 4. 수동 검증 상태

- Lobby 시작 및 process 생존: 확인 완료
- F1 Balance Tool에서 10개 pattern 노출/직접 편집: 빌드 완료, 실제 클릭 smoke 미실행
- Valtan 입장 뒤 상단 줄/HP와 HUD HP/Mana 픽셀 위치: 수동 화면 확인 미실행
- 130/80/76/33줄 실제 장시간 전투 smoke: 미실행. Server contract로 queue/피해만 자동 검증

## 5. 이번 단위에서 완료하지 않은 원작 표현

아래 항목은 pattern 이름과 Server 상태가 존재하지만 원작 표현까지 완료된 것은 아니다.

- 4방향 벽 actor 생성과 벽 충돌/파괴
- 80/33줄 실제 arena mesh 제거, navigation 재구축, snapshot world state
- 76줄 무력화 gauge와 player stagger damage 누적
- 6방향 line/sector, 전방향 안전지대 등 원작 shape
- notify 기반 red effect zone, particle collider 검증
- pattern별 원본 clip sequence와 weapon/effect collider binding

다음 수직 슬라이스는 Action decoder 결과의 Stage/notify를 presentation 문서로 cook하고,
Server semantic stage와 Client animation/effect/red-zone을 연결한 뒤 실제 지형 mutation을 별도 world
event 계약으로 구현해야 한다.

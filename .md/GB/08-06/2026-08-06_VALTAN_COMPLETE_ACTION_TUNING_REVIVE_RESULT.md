# 발탄 전체 Action 튜닝·입장 보호·Revive 구현 결과

- 작성일: 2026-08-06
- 브랜치: `codex/effect-tool-reboot`
- 대응 구현 계획서: `2026-08-06_VALTAN_COMPLETE_ACTION_TUNING_REVIVE_IMPLEMENTATION_PLAN.md`
- 대응 코드 계획서: `2026-08-06_VALTAN_COMPLETE_ACTION_TUNING_REVIVE_DETAIL_PLAN.md`
- 구현 판정: Server 권위 pattern stage·collider·입장 보호·Revive 수직 슬라이스 완료
- 자동 검증 판정: Debug/Release 전체 regression과 ProjectAudit 75개 PASS

## 1. 완료한 Action과 패턴 계약

`ValtanEncounter.json`을 formatVersion 3으로 올리고 `NORMAL_8P_2022` 원본 구간의
63개 Action ID를 31개 semantic pattern, 115개 ordered stage로 분류했다.

- 원본 Action 범위: `420601~420647`, `420651~420666`
- 일반 선택 pool: 23개 pattern
- HP 줄 scripted mechanic: 8개 pattern
- 제외 범위: 헬 `420667~420674`, 밈 섬 `420675`, 싱글 `420676`,
  7주년 `420677~420678`
- `420648~420650`은 선택한 원본 Action 구간에 존재하지 않으므로 가짜 Action을 만들지 않았다.

HP 줄 scripted queue는 다음 순서로 정본화했다.

| 줄 | mechanic |
|---:|---|
| 159 | 부위 파괴·오프닝 |
| 130 | 전멸기·바닥 공격 |
| 105 | 4방향 기둥 |
| 80 | 1차 지형 파괴 |
| 76 | 구슬 무력화 |
| 64 | 중앙 잡기·카운터 |
| 33 | 2차 지형 파괴 |
| 15 | 유령 전환 |

Action 하나를 pattern 하나로 복제하지 않고, 성공·실패·그로기·예외·Normal 변형을 해당
semantic pattern의 stage 또는 `sourceActionIds`로 묶었다. 공용 Action은 일반 pattern과
scripted mechanic 양쪽에서 재사용할 수 있다.

## 2. Server 권위 stage와 collider

- `PATTERN`은 줄 범위, trigger/order, weight, 최대 연속 사용, 선택 거리를 소유한다.
- `PATTERNSTAGE`는 stage kind, duration, semantic action, collider 형상, hit 횟수·간격,
  damage profile을 소유한다.
- Server는 `WINDUP -> ACTIVE -> RECOVERY`를 fixed tick 시간으로 전환한다. Client animation
  종료 신호로 pattern 종료나 다음 선택을 결정하지 않는다.
- pattern이 끝나면 Server가 scripted queue를 먼저 확인하고, 없으면 줄·거리·연속 제한을
  통과한 일반 후보에서 결정적 weighted selection을 다시 수행한다.
- XZ boss-local 판정으로 `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS` collider를 지원한다.
- Shared snapshot은 `patternId`, `patternSequence`, `patternStageIndex`, 현재 stage action을 전달한다.
- Valtan damage profile 23개를 추가해 전체 damage profile은 94개다.

## 3. 입장 즉사 방지와 Revive

Valtan Arena에 들어온 플레이어는 `isCombatReady=false`로 시작한다. Server의 target selection과
stage hit loop가 보호 중 플레이어를 모두 제외하며, Server가 승인한 첫 이동 또는 스킬 시작에서만
보호를 해제한다. 로딩 속도나 Client frame에 따라 발탄이 먼저 공격하는 경합을 제거했다.

Balance Tool의 `Revive at death position` 버튼은 local HP를 직접 바꾸지 않는다.

```text
Balance Tool
-> IPlayerCommandSink::Request_RevivePlayer
-> C2S_REVIVE_PLAYER
-> Server room command
-> 사망 여부·session·sequence 검증
-> 현재 위치/yaw 유지
-> HP/resource/action/path/cooldown 복원
-> isCombatReady=false
-> Server snapshot 반영
```

살아 있는 플레이어의 요청, 오래된 sequence, 다른 session의 revive는 적용하지 않는다.

## 4. Balance Tool에서 바로 튜닝 가능한 항목

- 31개 pattern의 source Action 목록과 일반/scripted 선택 정책
- 최소·최대 줄, trigger 줄과 순서
- selection weight, 최대 연속 사용, 최소·최대 거리
- 115개 stage의 kind, duration, semantic action
- collider shape와 radius/inner radius/angle/length/half width
- hit count, hit interval, Server damage rate
- live player HP와 combat-ready 보호 상태
- live boss pattern ID, sequence, stage index, action
- 사망 시 Server 권위 Revive 요청

Save/Reload는 encounter JSON과 provenance, gameplay publisher 검증을 통과한 데이터만 runtime
입력으로 반영한다. publish 뒤 Server를 재시작해야 새 밸런스가 제품 runtime에 적용된다.

## 5. 자동 검증 결과

### 5.1 데이터와 publisher

- balance provenance 동기화: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
  - player profile 5
  - skill definition 88
  - damage profile 94
  - Valtan pattern 31
  - Valtan pattern stage 115
- Action coverage, stage 연속성, shape별 필드, damage reference, hit timing 검증: PASS
- JSON parse와 `git diff --check`: PASS

### 5.2 protocol과 Server contract

- Shared protocol version 12 writer/reader round-trip: PASS
- revive 정상·zero sequence·truncated payload harness: PASS
- player `isCombatReady`와 boss pattern/sequence/stage snapshot harness: PASS
- Valtan 신규 입장자의 IDLE·무피해 보호 contract: PASS
- 130줄 scripted stage 실행 contract: PASS
- NetworkProtocolHarness Debug/Release: failures 0
- Server Debug/Release `--contract-test`: failures 0

### 5.3 전체 build/regression

- `Invoke-BuildAndRegression.ps1 -Configuration Debug`: PASS
- `Invoke-BuildAndRegression.ps1 -Configuration Release`: PASS
- 최종 80/33줄 데이터 교정 뒤 Debug/Release `-SkipBuild` regression 재실행: PASS
- Engine, UpdateLib, Shared, NetworkProtocolHarness, ClientFrontendHarness, Server, Client: PASS
- ProjectAudit: 75 checks PASS

## 6. 실제 Server runtime smoke

Debug Server에 실제 TCP client로 Valtan 입장·이동·스킬·사망·Revive packet을 보내 검증했다.

### 입력 전 보호

```text
ENTRY_PROTECTION_PASS hp=5500 samples=20
FIRST_INTENT_PASS combatReady=True sawPattern=True
```

입장 뒤 20개 snapshot 동안 HP 5500이 유지됐고, 첫 유효 이동 뒤에만
`combatReady=true`와 발탄 pattern 진행이 관찰됐다.

### 사망 위치 Revive

```text
DEATH_PASS hp=0 pos=(147.75, 23.019485..., -126.954994...)
REVIVE_PASS hp=5500/5500 ready=False pos=(147.75, 23.019485..., -126.954994...)
```

실제 Server room/network 경로에서 사망 전후 위치가 동일하고, 최대 HP 복원과 재보호가 확인됐다.
smoke에서 시작한 Server process만 종료했다.

## 7. 수동 검증과 남은 표현 경계

자동화와 실제 Server network smoke는 완료했다. 다만 Debug Client의 F1 Balance Tool을 사람이
열어 각 control을 클릭하고 화면 픽셀을 확인하는 GUI smoke는 실행하지 않았다.

다음 항목은 이번 변경에서 완료로 처리하지 않는다.

- 63개 Action의 원본 clip sequence·notify를 Client presentation에 1:1 연결
- 원본 ParticleSystem, Trail, Material, ViewShake, `PlayDecalEffect`의 정확한 stage binding
- red effect zone과 Server collider의 시각적 겹침 캡처 검증
- 80/33줄 실제 arena mesh 파괴와 dynamic navigation/world replication
- 76줄 무력화 gauge와 성공·실패 branch
- 잡기·카운터 성공 판정과 유령 분신의 별도 replicated actor

따라서 현재 상태는 pattern 선택, 줄 trigger, Server 시간, collider, damage를 Balance Tool에서 바로
튜닝할 수 있는 상태다. 원작 연출 fidelity와 실제 지형·무력화·카운터 기믹은 Action decoder의
stage/notify 결과를 presentation/world event 계약에 연결하는 후속 수직 슬라이스다.

## 8. Git 상태

작업 시작 전부터 Effect·Map 관련 대규모 미커밋 변경이 같은 worktree에 존재했고 현재도 병행 변경이
섞여 있다. 다른 담당 변경을 포함해 자동 stage/commit하지 않았으며, 이 결과 문서는 실제 코드·데이터와
실행한 검증만 기록한다.

## 9. 2026-08-07 최신 main 재조정 결과

- 기준 commit: `f2c96cf` (`origin/main`과 일치)
- UI PR #58: 10/10 파일 및 Lobby·Skill Window·authored HUD 계약 보존
- Monster/SpawnGroup PR #59: 87/87 파일 및 spawn/despawn·brain·trigger 계약 보존
- Player/Warlord/HOLD PR #60: 84/84 파일, Warlord 16 skill/binding 및 HOLD release 계약 보존
- DimensionMaster Effect PR #61: 424/424 파일, 16 catalog/runtime Effect와 fail-closed load 계약 보존
- protocol version: revive snapshot/command가 추가된 실제 wire 계약에 맞춰 `12`
- 구형 ImGui `##RuntimeCombatHUD`/`ProgressBar`를 제거하고 authored HUD와 Font Manager text만 유지
- Git 관리 Data 등록: expected/project/filters `543/543/543`, 누락·초과 0

최신 main과 충돌을 해결한 뒤 실행한 정본 Debug 회귀 결과는 다음과 같다.

```text
Gameplay: player profiles 6, runtime skill rows 125, damage profiles 108
Valtan: patterns 31, stages 115
World: Bern 7, Valtan Arena 12, Training Ground 4, Character Select 5
NetworkProtocolHarness: failures 0
Server --contract-test: failures 0
Effect Tool final: code 50, documents 16, resources 392, cues 8
ProjectAudit: 76/76 PASS
Invoke-BuildAndRegression.ps1 -Configuration Debug: PASS
Invoke-BuildAndRegression.ps1 -Configuration Release: PASS
```

동시에 진행 중인 DimensionMaster D Effect 파일과 `Tools/ModelAssetConverter/__pycache__`는 이 변경의
stage 대상에서 제외했다. 최신 main 재조정 뒤 Debug Client의 수동 GUI smoke는 별도로 실행하지 않았다.

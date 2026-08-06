# 발탄 전체 Action 튜닝·입장 보호·Revive 구현 계획서

- 작성일: 2026-08-06
- 브랜치: `codex/effect-tool-reboot`
- 문서 유형: 구현 계획서
- 기준 프로필: `NORMAL_8P_2022`
- 선행 결과: `2026-08-06_VALTAN_PATTERN_BALANCE_HUD_IMPLEMENTATION_RESULT.md`
- 완료 조건: 누락 Action group을 모두 저작 데이터에 보존하고, Server stage 실행과 Balance Tool 편집, 입장 보호, 사망 위치 Revive, publish/harness/Debug·Release 검증까지 실제 소비자를 연결한다.

## 1. 목표와 완료 경계

현재 10개 발탄 pattern은 `telegraph -> active -> recovery` 한 묶음과 `NONE/CIRCLE` 판정만 가진다.
이번 변경은 원본 `MN_RPBF_00.loa`의 `420601~420666` Normal 2022 관련 Action을 selectable pattern,
scripted mechanic, 성공·실패 branch의 `sourceActionIds`로 빠짐없이 분류한다.

Action 하나를 pattern 하나로 기계적으로 만들지 않는다. 반격 성공, 저지 성공, 그로기, 예외 처리,
백업, Normal 변형은 같은 semantic pattern의 stage 또는 source metadata로 묶는다. 헬, 싱글,
밈 섬, 7주년 변형은 Normal pool에 넣지 않는다.

이번 구현의 제품 완료 범위는 다음과 같다.

- pattern마다 ordered `stages[]`를 저장하고 Server가 30 Hz로 순서대로 실행한다.
- stage는 `WINDUP`, `ACTIVE`, `RECOVERY`와 `NONE`, `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS` 판정을 가진다.
- 일반 pattern은 줄·거리·가중치·연속 제한으로 선택하고, scripted pattern은 HP 줄 crossing queue가 우선한다.
- Shared snapshot은 `patternId`, `patternSequence`, `stageIndex`, `stage actionId`, `actionStartTick`을 보낸다.
- Balance Tool은 pattern과 stage 수치를 전부 편집하고 live stage를 표시한다.
- Valtan Arena 입장 플레이어는 첫 유효 이동 또는 스킬 의도 전까지 발탄 target/hit 후보가 아니다.
- Balance Tool Revive는 서버 명령으로 죽은 위치에서 HP와 자원을 복원하고 다시 첫 입력까지 보호한다.

원본 clip이 없는 Action에는 가짜 animation을 연결하지 않는다. stage semantic action은 snapshot까지
전달하되 Client presentation은 기존 안전 clip을 사용하고 missing binding을 격리한다. 실제 지형 mesh
mutation, 무력화 gauge, counter/grab 성공 판정, unresolved `PlayDecalEffect` asset binding은 이 변경에서
완료로 가장하지 않는다.

## 2. 원본 Action 분류

### 일반 pool

| semantic pattern | source Action |
|---|---|
| 휘두르기 | `420601`, `420660` |
| 내려찍기 | `420602`, `420661` |
| 감금 사자후 | `420603` |
| 대쉬 돌진 | `420604` |
| 지진 찍기 | `420605`, `420662` |
| 큰 베기 반격 | `420606`, 성공 `420607` |
| 마력기운 양자택일 | `420608` |
| 4연속 베기 | `420609` |
| 고공 점프 찍기 | `420610` |
| 발구르기 | `420611` |
| 구속 돌진 잡기·내려찍기 | `420612~420614` |
| 지진파 내려찍기 | `420615` |
| 마력구 방어막·무력화 | `420617`, 성공 `420618` |
| 초강력 내려찍기 | `420619`, `420620`, `420656`, `420657` |
| 점프 후 휠윈드 | `420621`, `420663` |
| 워프 돌진 | `420622` |
| 돌진 잡기·사자후 | `420623`, 성공 `420631`, 예외 `420632` |
| 휠윈드 | `420633` |
| 공격하며 후퇴 | `420635`, `420664` |
| 붉은 검기 | `420636` |
| 앞뒤앞 내려찍기 | `420637`, `420666` |
| 두 손 내려찍기 안/밖 | `420638` |
| 낙사 사자후 | `420639` |
| 연속 카운터 | `420640~420647` |

### scripted mechanic와 상태 branch

| mechanic | source Action |
|---|---|
| 지면파괴 사자후 계열 | `420616`, `420658`, `420659` |
| 망령 전환·분신 | `420624~420626`, `420634`, `420652`, `420653`, `420665` |
| 부위 파괴·외벽 충돌 그로기 | `420627`, `420628`, `420654` |
| arena 전체 파괴 | `420629` |
| 130줄 전멸기 | `420630` |
| 3페이즈 전멸기 | `420651` |
| 오프닝 | `420655` |

## 3. 데이터와 런타임 흐름

```text
ValtanEncounter.json formatVersion 3
-> Publish-GameplayBalance exact validation
-> Gameplay.bootstrap v3 PATTERN + PATTERNSTAGE rows
-> CGameplayCatalog parse/validate/stage/commit
-> CValtanBrain weighted/scripted selection
-> ordered stage execution + Server shape hit
-> S2C_WORLD_SNAPSHOT pattern/stage state
-> CCombatHUDViewModel
-> Balance Tool live verification
```

입장 보호와 Revive 흐름은 다음과 같다.

```text
CGameRoom::Join(VALTAN_ARENA)
-> SERVER_PLAYER::isCombatReady = false
-> CValtanBrain target/hit filtering
-> first accepted move or skill -> isCombatReady = true

Balance Tool Revive
-> IPlayerCommandSink::Request_RevivePlayer
-> CNetworkPlayerCommandSink
-> CNetworkManager::Send_Revive
-> C2S_REVIVE_PLAYER
-> ROOM_COMMAND_TYPE::REVIVE_PLAYER
-> CGameRoom::Handle_RevivePlayer
-> same position + full HP/resource + cleared action/path/cooldown
-> isCombatReady = false
-> next snapshot
```

## 4. G별 구현 범위

### G00. 데이터 schema와 Action coverage

- `ValtanEncounter.json`을 formatVersion 3의 `patterns[].stages[]` 구조로 교체한다.
- Normal 2022 Action group의 모든 ID가 한 개 이상의 semantic pattern `sourceActionIds`에 들어가는지
  publisher와 audit가 검사한다. 공용 동작은 일반 pool과 HP mechanic 양쪽에서 재사용할 수 있다.
- 헬 `420667~420674`, 밈 섬 `420675`, 싱글 `420676`, 7주년 `420677~420678`은 제외 목록으로 고정한다.

종료 증거: JSON parse, duplicate/missing Action coverage 실패 fixture, 정상 validate PASS.

### G01. publisher와 Server catalog

- `Gameplay.bootstrap`을 v3으로 올리고 `PATTERN`, `PATTERNSTAGE` row를 생성한다.
- stage index 연속성, stage kind, shape별 필수/금지 수치, damage reference, hit interval을 검증한다.
- load 실패는 기존 catalog를 보존한다.

종료 증거: 정상 publish, bad shape/stage/damage/duplicate rollback contract PASS.

### G02. Server stage와 shape 실행

- `CValtanBrain`이 선택한 pattern의 stage를 순서대로 실행한다.
- `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS`를 발탄 위치·고정 yaw 기준 XZ 판정으로 계산한다.
- pattern 종료 뒤 IDLE에서 scripted queue 우선, 그다음 weighted normal selection을 다시 수행한다.
- 가중 ticket은 `serverTick % totalWeight`에서 room/runtime seed와 pattern sequence를 섞은 결정적 값으로 교체한다.

종료 증거: shape edge hit/miss, stage order, multi-hit, repeat limit, scripted preemption PASS.

### G03. Shared stage snapshot

- protocol version을 올리고 boss snapshot에 `patternId`, `patternSequence`, `stageIndex`를 추가한다.
- writer/reader exact validation과 NetworkProtocolHarness를 갱신한다.
- Client는 모르는 pattern ID를 gameplay fallback으로 바꾸지 않고 live diagnostics에 그대로 보존한다.

종료 증거: 정상 round-trip, bad enum/oversized ID/truncated payload 거부 PASS.

### G04. 입장 보호와 Revive

- Valtan Arena join과 revive 직후 `isCombatReady=false`다.
- 첫 유효 move/skill만 보호를 해제한다.
- 발탄 target selection과 hit loop 모두 보호 중 플레이어를 제외한다.
- revive는 본인 session의 사망 player에만 적용하며 위치는 바꾸지 않는다.

종료 증거: 입장 후 입력 전 피해 0, 첫 이동 뒤 피해 가능, alive revive no-op, dead revive same-position/full-HP PASS.

### G05. Balance Tool

- pattern header에 source Action 목록과 selection 상태를 표시한다.
- stage별 duration, kind, semantic action, shape와 shape 수치, hit count/interval, damage rate를 편집한다.
- live pane에 pattern/stage/sequence와 combat-ready 보호 상태를 표시한다.
- local player가 죽었을 때만 Revive 버튼을 활성화한다.

종료 증거: Save/Reload round-trip, invalid draft 거부, Revive request packet smoke PASS.

### G06. 회귀와 수동 smoke

- gameplay publish validate/publish
- Shared/NetworkProtocolHarness Debug·Release
- Server Debug·Release와 `--contract-test`
- Client Debug·Release
- ProjectAudit와 `git diff --check`
- 실제 Server+Client Valtan 입장, 입력 전 생존, 첫 이동 뒤 전투, 사망, Revive, 같은 위치 재개 확인

## 5. 실패와 rollback

- Data Save는 기존 파일을 staging한 뒤 모든 JSON과 provenance가 성공할 때만 교체한다.
- catalog는 pattern/stage 전체가 검증된 뒤 한 번에 commit한다.
- 잘못된 revive payload는 session을 닫고, 정상 payload지만 현재 상태에서 불가능한 revive는 no-op이다.
- Client presentation 누락은 해당 animation/effect만 격리하고 Server pattern과 snapshot을 중단하지 않는다.

## 6. 완료 보고 기준

RESULT에는 다음을 분리한다.

- 실제 구현된 Action coverage와 stage/shape
- 서버 자동 검증 결과
- Debug/Release 빌드 결과
- 수동 Valtan smoke 결과
- 원본 animation/effect/world mutation 중 남은 표현 경계

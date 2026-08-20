# 2026-08-21 DimensionMaster BA command buffer implementation plan

## 0. 문서 상태

- 문서 종류: `IMPLEMENTATION_PLAN`
- 대상: 차원술사 기본 공격 `2050010`의 타이밍 회귀 복구, 콤보 전진 시점 분리, 다음 명령 보존과 입력 우선순위
- 구현 범위: `Data -> publisher/bootstrap -> Server authority -> Client input -> Balance Tool -> harness`
- 제외 범위: Client 로컬 콤보 예측, 애니메이션 도중 즉시 캔슬, 신규 network packet, Effect 문서 재저작
- 완료 판정: 자동 검증과 Debug/Release 빌드를 통과한 뒤 사용자가 직접 Character Select에서 조작감을 확인한다.

## 1. 문제와 원인

### 1.1 관찰 증상

- 차원술사 BA를 한 번 눌러도 BA1이 끝나는 시점까지 이동과 다른 스킬이 적용되지 않는다.
- LMB를 짧게 눌렀다고 생각해도 100ms hold resend가 BA1의 combo window 안에 들어오면 BA2가 Server에 buffer된다.
- BA2가 시작된 뒤에는 Server가 `MOVE`와 다른 `USE_SKILL`을 폐기하므로 BA2 종료까지 다음 command가 사라진다.

### 1.2 실제 회귀

- `Data/Balance/PlayerSkills.json`의 차원술사 BA1 stage duration은 과거 source 계약의 `1400ms`에서 `4000ms`로 회귀했다.
- source notify와 기존 계획은 BA1 hit `100ms`, combo pre-input `100..510ms`, force end `1400ms`를 가리킨다.
- 현재 `DimensionMaster.skillbindings.json`은 BA1~BA4를 Server combo stage와 정확히 1:1로 나눈다. BA1과 BA2가 하나의 animation binding에 묶인 문제가 아니다.
- 현재 Server는 `hasBufferedComboInput && hasAppliedSkillDamage`가 참이 되는 즉시 다음 stage를 시작한다. `hitTimeMs`가 표현 애니메이션 종료 시점까지 겸하고 있어 BA2가 너무 일찍 시작될 수 있다.
- `GameRoom::Handle_Move`는 action 중 이동 packet의 sequence를 소비한 뒤 command를 버린다. `PlayerSkillSystem::Try_Start`도 같은 COMBO continuation 외 다른 skill을 버린다.
- Client의 LMB release는 `C2S_RELEASE_SKILL`을 보내지 않는다. 따라서 tap/hold 경계를 Server가 알 수 없다.

## 2. 고정할 제품 계약

1. 한 BA stage의 애니메이션은 명시 MOVE/SKILL이 들어와도 그 stage의 `actionDurationMs`까지 유지한다. 중간 프레임에서 이동이나 다른 skill이 Client 임의로 animation을 자르지 않는다.
2. `hitTimeMs`는 damage 시점, `comboAdvanceMs`는 buffered BA가 다음 stage로 갈 수 있는 시점, `actionDurationMs`는 pending explicit command를 commit하는 시점이다. 세 값은 독립이다.
3. LMB release는 현재 BA stage를 즉시 자르지 않지만, 아직 commit되지 않은 동일 BA continuation을 취소한다.
4. COMBO action 중 수신한 다음 command는 action 수명 안에서 하나만 보관한다. 최신 explicit command가 이전 pending explicit command를 덮어쓴다.
5. pending explicit command가 있으면 `comboAdvanceMs`의 BA continuation을 막고 현재 stage의 `actionDurationMs`에서 `death/forced reset > explicit SKILL or MOVE > NONE` 순서로 처리한다.
6. pending MOVE는 commit 시점의 최종 Server 위치에서 navigation을 다시 계산한다. 수신 시점에 path나 위치를 확정하지 않는다.
7. pending SKILL은 수신 시 packet sequence만 소비하고 cooldown/resource를 쓰지 않는다. boundary에서 class, slot, cooldown, resource, stance, aim을 다시 검증한 뒤에만 시작한다.
8. pending command는 death, class change, world transfer, disconnect, fall/forced movement, action replacement에서 반드시 제거한다.
9. Client는 Server snapshot의 `skillId/comboStage/action`만 표현한다. local BA2 진행이나 local locomotion 전환을 만들지 않는다.

## 3. 데이터 계약

### 3.1 `comboAdvanceMs`

- `PLAYER_COMBO_STAGE`와 `PlayerSkills.json.comboStages[]`에 정수 `comboAdvanceMs`를 추가한다.
- 범위는 `hitTimeMs <= comboAdvanceMs <= durationMs`이며 `comboAdvanceMs`는 해당 stage의 마지막 caster 반복 hit와 마지막 projectile spawn 시점보다 빠를 수 없다.
- 다음 stage가 없는 마지막 stage도 값은 `durationMs`로 저장해 stage boundary 의미를 일정하게 유지한다.
- 기존 다른 class는 현재 동작을 의도적으로 유지해야 하는 row를 조사한 뒤 명시 값을 넣는다. parser fallback으로 조용히 `hitTimeMs`를 사용하지 않는다.

### 3.2 explicit command buffer

- COMBO stage 안에서만 pending MOVE/SKILL을 허용한다.
- pending 상태는 packet pointer나 vector index가 아니라 값 복사와 stable skill/slot/sequence로 소유한다.
- 별도의 protocol version 변경은 하지 않는다. 기존 `C2S_MOVE`, `C2S_USE_SKILL`, `C2S_RELEASE_SKILL`을 재사용한다.
- Balance Tool은 `durationMs`, `hitTimeMs`, `comboAdvanceMs`, combo input window를 함께 편집하고 동일 교차 검증을 실행한다.

### 3.3 차원술사 정정값

| Stage | durationMs | hitTimeMs | comboAdvanceMs | input window |
|---|---:|---:|---:|---|
| BA1 | 1400 | 100 | 1400 | 100..510 |
| BA2 | 1500 | 43 | 1500 | 0..410 |
| BA3 | 1067 | 28 | 1067 | 200..1067 |
| BA4 | 1700 | 335 | 1700 | 0..0 |

- BA1 skill binding은 `playMs: 1400`을 명시한다.
- BA1 root-motion stage duration은 `1400`으로 줄이고 exact 1400ms sample까지만 보존한다.
- receipt는 전용 update script로 동기화하며 생성 bootstrap을 직접 편집하지 않는다.

## 4. 구현 단위

### G00. 실패를 실행 계약으로 고정

목적:
- 구현 전에 현재 회귀를 Server/Client harness가 재현하도록 한다.

변경 대상:
- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

검증 항목:
- 차원술사 BA1 exact `4000ms` 회귀를 실패로 잡는다.
- 단일 tap 후 BA2 snapshot이 생성되지 않아야 한다.
- BA action 중 MOVE sequence가 소비만 되고 사라지는 현재 동작을 실패로 잡는다.
- BA1 binding이 1400ms presentation trim을 갖지 않으면 실패한다.

### G01. BA 타이밍 복구와 `comboAdvanceMs`

목적:
- damage와 combo transition의 시간을 분리하고 차원술사 BA1의 4초 control lock을 제거한다.

변경 대상:
- `Data/Balance/PlayerSkills.json`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
- `Data/Animation/RootMotion/DimensionMaster.rootmotion.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- 생성되는 Server gameplay bootstrap

구현:
- publisher schema와 bootstrap generator가 `comboAdvanceMs`를 필수로 검증/출력한다.
- Server parser는 exact property와 범위를 검증하고 partial catalog commit을 금지한다.
- `PlayerSkillSystem::Update`는 elapsed가 `comboAdvanceMs`에 도달하고 모든 caster hit/projectile spawn이 끝났을 때만 buffered next stage를 시작한다.
- pending explicit command가 있으면 buffered next stage를 시작하지 않고 현재 stage `actionDurationMs`까지 유지한다.
- stage duration이 끝날 때까지 다음 stage나 explicit command가 없으면 action을 `NONE`으로 종료한다.
- root-motion sample은 stage duration을 넘지 않으며 BA1 curve를 1400ms에서 정확히 truncate한다.

실패 소비:
- 잘못된 timing은 publisher/Server catalog load에서 전체 staged balance revision을 거부하고 기존 revision을 유지한다.

### G02. LMB release 취소와 pending MOVE

목적:
- tap을 hold로 오인해 BA2가 자동 진행되는 문제와 action 중 RMB 이동 유실을 없앤다.

변경 대상:
- `Client/Public/PlayerController.h`
- `Client/Private/PlayerController.cpp`
- `Server/Public/ServerPlayer.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/PlayerSkillSystem.cpp`

구현:
- Client는 LMB down에서 resolved BA skill ID를 보관하고 up edge에서 기존 `Request_ReleaseSkill`을 보낸다.
- Server release는 `iClientSequence`가 newer인 경우에만 consume한다. 현재 COMBO skill과 ID가 일치할 때 `hasBufferedComboInput`과 buffered aim을 제거하며 현재 stage animation은 유지한다.
- 지연된 이전 action의 release와 duplicate release는 같은 skill ID의 새 action/새 stage buffer를 지우지 않는다.
- action 중 MOVE는 COMBO action이면 단일 pending explicit intent로 stage한다. 최신 MOVE가 이전 pending MOVE를 교체한다.
- pending MOVE가 있으면 이른 BA continuation을 막고 현재 stage `actionDurationMs`에서 action을 종료한 뒤 navigation/projected goal을 새로 계산해 commit한다.
- pending을 지원하지 않는 ACTIVE/HOLD/COUNTER action의 기존 guard는 이 단계에서 풀지 않는다.

### G03. pending SKILL 일반화와 Balance Tool

목적:
- 이동뿐 아니라 quick-slot skill도 같은 명시 입력 우선순위로 보존하고 authoring field를 툴에서 안전하게 편집한다.

변경 대상:
- `Server/Public/ServerPlayer.h`
- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Private/GameRoom.cpp`
- `Client/Public/BalanceTool.h`
- `Client/Private/BalanceTool.cpp`

구현:
- pending intent는 tagged value `NONE/MOVE/SKILL` 하나만 소유한다.
- 진행 중 COMBO와 다른 skill command는 sequence/형식 검증 후 값으로 stage한다.
- commit 때 gameplay catalog를 다시 resolve하고 모든 비용/쿨다운/상태 조건을 재검증한다.
- 재검증 실패는 pending 하나만 격리하고 BA continuation을 뒤늦게 되살리지 않는다.
- Balance Tool은 `comboAdvanceMs`를 표시/저장하고 `hit <= advance <= duration`을 save 전에 검증한다.

### G04. 전체 class COMBO 회귀

목적:
- 차원술사 전용 hardcode 없이 여섯 playable class의 LMB COMBO 계약을 보존한다.

검증 항목:
- 각 class `(characterClass, LMB) -> skillId`가 catalog에서 resolve된다.
- tap은 stage 1 종료, hold는 다음 stage, release는 아직 commit되지 않은 continuation 취소다.
- explicit MOVE/SKILL은 buffered BA를 막고 현재 stage의 전체 animation duration 뒤 실행된다.
- 반복 hit/projectile spawn이 있는 기존 class도 마지막 필수 occurrence 전에 다음 BA로 넘어가지 않는다.
- stale/duplicate sequence, wrong skill release, death/class/world reset에서 pending이 남지 않는다.
- ACTIVE/HOLD/COUNTER의 기존 입력 정책이 바뀌지 않는다.

## 5. 자동 검증 계획

1. JSON/XML/PowerShell parse와 `git diff --check`.
2. `Update-BalanceProvenanceReceipt.ps1` 실행 후 변경 row만 diff 확인.
3. `Publish-GameplayBalance.ps1 -Mode Validate`.
4. `Publish-BalanceRuntimeSet.ps1 -Mode Validate`.
5. Server Debug/Release build와 `Server.exe --contract-test`.
6. ClientFrontendHarness `--skill-binding-fast`와 새 controller/presentation 계약 테스트.
7. NetworkProtocolHarness 회귀. packet shape는 바뀌지 않으므로 protocol version은 올리지 않는다.
8. 정본 `Invoke-BuildAndRegression.ps1` Debug, Release.

## 6. 사용자 수동 확인

- Character Select에서 차원술사 BA를 짧게 tap하면 BA1만 약 1.4초 재생되고 idle로 돌아오는지 확인한다.
- LMB hold는 BA1에서 BA2로 자연스럽게 이어지는지 확인한다.
- BA1 도중 LMB release 후 RMB를 누르면 BA2가 시작되지 않고 BA1 boundary에서 이동하는지 확인한다.
- BA1 도중 quick-slot skill을 누르면 boundary에서 그 skill이 BA continuation보다 먼저 시작하는지 확인한다.
- BA stage 중 root motion과 Effect cue가 1400ms trim에서 pop 또는 순간이동하지 않는지 확인한다.

Client 실행과 visual/조작감 PASS 판정은 사용자가 수행한다.

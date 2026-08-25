# 차원술사 BA 2동작 자동 연계와 LMB/T 확정 입력 수정 구현 결과

## 1. 결론과 현재 판정

원본 read-only reference에는 `_01/_02/_03/_04` 네 physical clip이 그대로 존재한다. 이번 국소
수정은 authored asset을 삭제하는 작업이 아니라 LMB 제품 sequence만 다음 두 Server stage로 닫는
작업이다.

```text
_01 source 0..1800ms / playRate 2.0 = 900ms
-> _03 natural = 1067ms
```

LMB 한 번은 이 두 motion을 Server 권위로 자동 연계하고 stage 2 종료 시 닫힌다. Product Effect
cue는 `ba2 -> ba3`만 연결한다. 이전 Server 자동 stage 3인 `_04 + ba1` 연결은 제거했지만 `_04`
clip과 `ba1` authored Effect asset 자체는 보존했다.
LMB 입력은 command 제출 성공 뒤에만 press를 소비하는 transactional edge gate로 바꿨다. T는 최신
snapshot에서 즉시 시작 가능하거나 실행 중 COMBO 뒤에 예약 가능한 경우 preview를 시작·유지하고,
성공한 confirm LMB가 BA로 새지 않도록 release까지 suppression한다.

| 구분 | 현재 판정 |
|---|---|
| BA 2-motion animation·Server timing·root motion | 구현 완료 |
| Product Effect cue `ba2 -> ba3`, `_04 + ba1` 단절 | 구현 완료 |
| LMB transactional press edge | 구현 완료 |
| T availability preflight·confirm suppression | 구현 완료 |
| focused Python 26 tests | PASS |
| gameplay balance Validate/Publish | PASS |
| balance runtime set Validate | PASS |
| Effect Validate, canonical ResourceRoot | `UNRELATED_DIRTY_BLOCKED`, 범위 밖 `2050180` seal 불일치 |
| Server build | Release PASS, Debug relink는 실행 중인 Server 점유로 보류 |
| Server contract | Release PASS, `failures : 0` |
| Client x64 Debug build | PASS |
| Client x64 Release build | PASS |
| 사용자 BA/T visual fidelity | `PENDING_USER_VISUAL_GATE` |

## 2. 원본 4 clips와 제품 2 motions

원본 `DimensionMaster.clipseq/.clipmap/.animnotify`에는 네 clip이 존재하므로 원본을 세 clip
구조라고 기록하지 않는다. 다만 `pc_sp_m_00_sk_att_battle_1_01` 하나에는 다음 두 attack
occurrence가 함께 존재한다.

```text
SOUND  29ms / 303ms
EFFECT 200ms / 400ms
HIT    100..602ms
```

따라서 `_01` 안의 두 occurrence는 한 Server stage에서 유지한다. `_01`의 제품 구간을 source
`0..1800ms`로 두고 `playRate=2.0`을 적용해 wall duration을 `900ms`로 조정했다. 이어지는 `_03`만
두 번째이자 마지막 motion으로 사용한다.
원본 `_02/_04` 파일을 삭제하거나 reference 자료를 변조하지 않는다.

| 제품 stage | 의미 | binding | wall duration | Server hit | combo advance | input window |
|---|---|---|---:|---:|---:|---|
| 1 | 두 번 찌르기 | `_01`, `playMs=1800`, `playRate=2.0` | `900ms` | `50ms` | `900ms` | automatic `0/0` |
| 2 | 휘두르기 | `_03`, natural | `1067ms` | `28ms` | `1067ms` | terminal `0/0` |

총 presentation wall time은 `1967ms`다. `PlayerSkills.json`의 top-level
`actionDurationMs/hitTimeMs` mirror는 첫 stage와 같은 `900/50ms`다. non-final COMBO의
`inputOpenMs/inputCloseMs == 0/0`과
`comboAdvanceMs == actionDurationMs` 조합만 automatic stage로 인정한다. non-zero input
window를 가진 다른 직업 COMBO는 계속 추가 입력을 요구한다.

Server의 단순 fallback damage 경로는 stage마다 한 번의 damage event를 만들기 때문에 stage 수가
3에서 2로 줄면서 fallback event 수도 3에서 2로 줄어든다. 이번 범위는 animation, input,
Effect occurrence 연결이며 damage profile 증폭, hit-shape 추가 또는 별도 damage rebalance는
포함하지 않았다.

## 3. Product Effect cue와 authored payload 보호

`DimensionMaster.animevents`에는 다음 두 Product cue만 둔다.

| motion clip | Product Effect stable ID |
|---|---|
| `_01 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba2.unified` |
| `_03 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba3.unified` |

두 cue는 `root / follow / action_facing / natural`을 유지한다. 사용자가 Effect Tool에서
손튜닝하는 stable authored 문서를 그대로 소비하도록 연결만 바꿨다. 다음 항목은 수정하거나
삭제하지 않았다.

- `effect.dimensionmaster.skill.2050010.ba1~ba4.unified.effect.json`의 Element와 resource binding
- delay, lifetime, mesh, DDS, material, source recipe와 runtime payload
- 현재 제품 sequence에서 사용하지 않는 `ba1`, `ba4` authored 문서와 `_04` clip
- V0 또는 과거 Effect payload

작업 시작 시 존재하던 다른 작성자와 사용자의 dirty worktree는 그대로 보호했다. 범위 밖 파일을
reset, checkout, clean하거나 기존 변경을 되돌리지 않았고, Effect authored payload를 정리한다는
이유로 덮어쓰지 않았다.

## 4. Root Motion 결과

`DimensionMaster.rootmotion.json`의 `2050010`은 두 제품 stage와 같은 wall clock을 사용한다.

| stage | source | duration | samples | 마지막 forward |
|---|---|---:|---:|---:|
| 0 | 기존 `_01` source `0..1800ms`를 2배속 재타이밍 | `900ms` | 55 | `0.9298` |
| 1 | 기존 `_03` curve, old stage 2에서 재번호화 | `1067ms` | 33 | `0.2802` |

stage 0은 exact `900ms` sample에서 닫히며 source timestamp만 wall time으로 변환하고
forward/lateral/up 값은 보존했다. 기존 `_02`용 old stage 1과 `_04`용 old stage 3 curve는 제품
sequence에서 제거했다.

현재 dirty authored `ba2`의 두 visible element 시작점은 source 기준 약 `0.55s`, `1.26s`이고
2배속 wall 기준 명목 `275ms`, `630ms`다. source-time 1/60초 fixed-step admission까지 반영하면
두 번째 실제 시작은 약 `633.3ms`이므로 `900ms` 경계보다 약 `266.7ms` 앞선다. 따라서 두
occurrence가 모두 시작하며 cue stop mode가 `natural`이므로 stage 전환 뒤에도 이미
시작한 Effect tail은 자연 종료한다. 제품 runtime에서도 `CCharacter::Update_EffectCues()`가 현재
clip의 `fPlayRate`를 `EFFECT_SPAWN_DESC`에 전달하고 `CEffectPresentationService`가 Effect sample
clock을 `wall delta * playback rate`로 진행하므로 이 환산이 실제 재생 경로와 일치한다.

## 5. LMB와 T 입력 수정

### 5.1 LMB transactional edge gate

기존 gate는 command sink가 command를 수락하기 전에 현재 물리 press를 제출 완료로 표시했다.
aim, transform 또는 send가 일시 실패하면 같은 LMB press가 영구히 소비됐고, 반대로 입력 차단 중
시작된 press가 차단 해제 뒤 늦은 BA로 나갈 수 있었다.

수정된 `CBASIC_ATTACK_PRESS_EDGE_GATE` 계약은 다음과 같다.

1. raw physical up만 다음 press를 rearm한다.
2. gameplay input이 차단된 상태에서 시작된 press는 그 release까지 blocker가 소비한다.
3. eligible down은 command candidate만 만들고 아직 소비하지 않는다.
4. `Request_UseSkill` 성공 뒤에만 `Commit_Submission()`으로 press를 소비한다.
5. local command sink의 일시 실패는 commit하지 않아 같은 held press가 다음 frame 재시도된다.
6. 성공 뒤 held 상태에서는 같은 press의 두 번째 BA command를 제출하지 않는다.

두 production gate와 ground-target confirm transaction은 `constexpr` state machine으로 유지하고 같은
구현을 `static_assert` 계약에서 직접 실행한다. 이 compile-time harness는 capture 중 시작된 press,
sink reject 재시도, sink accept 뒤 held 억제, raw release 재무장과 T confirm reject/accept를 fake
request callback으로 검증한다. 따라서 소스 문자열 위치만 확인하는 테스트에 의존하지 않는다.

Client가 animation을 local 선행 재생하지는 않는다. 첫 animation은 계속 Server 승인 snapshot의
action edge에서 시작하므로 정상 network tick 지연을 입력 유실로 숨기지 않는다.

### 5.2 Server automatic combo

Server catalog와 publisher는 non-final `0/0` window를 automatic stage로 읽되 advance와 duration이
같지 않으면 거부한다. `CPlayerSkillSystem`은 automatic stage의 full-motion boundary에서 buffered
LMB 없이 다음 stage로 전환한다. 명시적인 MOVE/SKILL pending command도 automatic chain을 끊지 않고
마지막 motion 종료 뒤 commit한다. manual window를 가진 다른 COMBO는 현재 full-motion 경계에서
pending을 commit하는 기존 buffered-input 계약을 유지한다.

### 5.3 T availability와 confirm suppression

`CPlayerSkillCatalog`은 기존 resource cost와 함께 `identityCost`를 exact `uint32`로 검증·보존한다.
`CCombatHUDViewModel`은 snapshot의 현재 skill ID도 보존해 Server pending 가능 여부를 판단한다. T
preview 시작과 유지는 다음 두 admission을 구분한다.

- action이 `NONE`인 즉시 cast는 alive local player, class, `ACTIVE/GROUND_POINT`, cooldown,
  resource, identity와 required stance를 모두 검사한다.
- 다른 COMBO가 실행 중인 pending cast도 현재 snapshot의 cooldown, resource, identity와 stance를
  만족해야 preview와 confirm을 허용한다. automatic BA에서는 2동작 전체 종료 뒤 Server가 다시
  검증한다.
- Valtan 입장 직후 `isCombatReady=false`는 첫 유효 이동/스킬이 true로 전환하는 상태이므로 T를
  선제 차단하지 않는다.

조건이 사라지면 active preview를 취소한다. valid ground에서
`Request_UseGroundTargetSkill`이 성공한 뒤에는 `Suppress_UntilRelease()`를 먼저 적용하고
preview를 닫는다. 따라서 confirm에 사용한 같은 LMB가 다음 input pass에서 BA로 재해석되지 않는다.
invalid ground와 RMB cancel은 T action 또는 BA를 시작하지 않는다.

## 6. 변경 범위

기존 LMB/T 입력 수정은 그대로 유지했고, 이번 2-stage 국소 수정은 다음 vertical slice를 갱신했다.

- animation/balance data: `DimensionMaster.skillbindings.json`,
  `DimensionMaster.animevents`, `DimensionMaster.rootmotion.json`, `PlayerSkills.json`,
  balance provenance receipt
- Server contract: data-driven runtime은 유지하고 `ServerGameplayContractTests.cpp`의 차원술사 기대값만
  `900/1067ms`, 두 stage로 교정
- focused tests: BA stage split, catalog combo timing, Effect Tool automatic audition,
  ground-target availability·LMB gate
- public handoff/known issue: Animation Tool·Balance Tool owner handoff, gameplay interface handbook,
  GB gotchas와 대응 PLAN/RESULT

Effect authored 문서, Effect runtime payload, T animation binding과 T Effect payload는 이 변경
범위에 포함하지 않았다.

## 7. 자동 검증 결과

### 7.1 Focused Python

다음 네 실행형 focused test의 합계 26 tests가 PASS했다.

| test | 결과 |
|---|---:|
| `test_dimensionmaster_2050010_stage_split.py` | 5 PASS |
| `test_player_skill_catalog_combo_timings.py` | 4 PASS |
| `test_effect_tool_buffered_combo_audition.py` | 11 PASS |
| `test_ground_target_preview_prototype_scope.py` | 6 PASS |
| 합계 | **26 PASS** |

검증 범위는 두 clip group, 2-stage timing과 root motion, `ba2/ba3` stable cue와 `_04 + ba1` 부재,
automatic parse/rollback, 다른 직업 manual COMBO 보존, Animation/Effect Tool 누적 timeline,
production `static_assert`에서 실행되는 blocked/retry/commit/release LMB gate와 fake-confirm
transaction, T availability와 confirm suppression이다.

### 7.2 Publisher

| 명령 영역 | 결과 |
|---|---|
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |
| `Publish-GameplayBalance.ps1 -Mode Publish` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Validate` | PASS |
| `Publish-Effects.ps1 -Mode Validate` | `UNRELATED_DIRTY_BLOCKED` |

Effect Validate는 canonical `Client/Bin/Resources`를 `-ResourceRoot`로 사용했다. animevents
header/count 검사를 통과한 뒤 이번 범위 밖의 기존 dirty
`effect.dimensionmaster.skill.2050180.v1.unified/effect.dimensionmaster.skill.2050180.unified` seal
불일치에서 중단됐다. 이 BA 수정에서 `2050180` 문서나 seal은 건드리지 않았다.

### 7.3 Build와 contract

| 검증 | 결과 |
|---|---|
| Server x64 Debug compile/link | 실행 중인 Debug Server가 `Server.exe`를 점유해 link 보류 |
| Server x64 Release compile/link | PASS |
| Debug `Server.exe --contract-test` | 현재 실행 중인 Server를 종료하지 않아 미실행 |
| Release `Server.exe --contract-test` | PASS, `failures : 0` |
| Client x64 Debug compile/link | PASS |
| Client x64 Release compile/link | PASS |

Release contract는 다음 차원술사 경로를 포함해 `failures : 0`으로 종료했다. Debug는 사용자 실행
프로세스를 종료하지 않았으며, 재시작 시 새 bootstrap의 `900ms` 값을 로드한다.

- `Resolve full automatic DimensionMaster BA motions and stage-aligned root motion`
- `Advance one DimensionMaster LMB through the two project-tuned BA motions`

published `Gameplay.bootstrap`에서도 `2050010`은 top-level `900/50ms`, stage
`0=900/50/900`, `1=1067/28/1067` 두 줄만 존재한다. root motion도 stage 0의 55 samples와 stage
1의 33 samples만 publish됐다.

## 8. 사용자 수동 visual 검증

에이전트는 Client/UI를 자율 실행·조작하거나 화면 fidelity를 대신 판정하지 않았다. 사용자는
Server와 Client를 직접 실행해 다음을 확인한다.

1. LMB 한 번으로 `_01(약 0.9초) -> _03(약 1.067초)`만 추가 클릭 없이 이어지는지 확인한다.
2. 첫 `_01`에서 현재 손튜닝한 `ba2`의 두 visible occurrence, `_03`에서 `ba3` cue가 시작하는지 확인한다.
3. `_03` 종료 뒤 `_04` animation과 `ba1` Effect가 재생되지 않고 BA가 닫히는지 확인한다.
4. LMB를 누르고 있어도 두 번째 BA command가 제출되지 않고 Server automatic stage만 진행하는지 확인한다.
5. F1/ImGui가 잡은 상태에서 시작한 LMB가 capture 해제 뒤 늦게 BA로 발사되지 않는지 확인한다.
6. 평시 또는 BA 진행 중 T preview 뒤 valid 위치 confirm LMB가 T animation을 즉시 또는 BA 종료 뒤 시작하고 같은 click이 BA로 새지 않는지 확인한다.
7. T 사용 불가 상태, invalid ground와 RMB cancel에서 T와 BA가 시작되지 않는지 확인한다.

사용자의 서면 관찰 전 최종 시각 상태는 다음과 같다.

```text
PENDING_USER_VISUAL_GATE
```

## 9. 완료 경계

BA 2-motion data, Server automatic stage, LMB transactional input, T availability/confirm suppression과
focused publisher/test 범위는 구현 완료다. 다음 항목은 완료로 승격하지 않는다.

- 범위 밖 dirty `2050180` Effect seal 때문에 중단된 full Effect Validate: `UNRELATED_DIRTY_BLOCKED`
- 현재 실행 중인 Debug Server 종료·재링크와 새 bootstrap 재로드
- 사용자의 BA/T animation·Effect visual 판정
- 3→2 stage에 따른 별도 damage rebalance
- V0 또는 Effect authored payload 복원

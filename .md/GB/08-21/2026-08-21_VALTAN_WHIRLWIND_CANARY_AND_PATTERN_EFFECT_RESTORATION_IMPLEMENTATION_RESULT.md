# 2026-08-21 Valtan Whirlwind Canary 및 Pattern Effect 복원 구현 결과

## 0. 결과 요약

발탄 Whirlwind action `420633`, source stage `2`, runtime clip
`mesh_att_battle_20_03`의 source carrier 아홉 개는 base 5, baked-edge Trail 3,
first-edge Light 1로 모두 실행 가능한 packet까지 연결됐다.

source/VisualProgram packet은 `9/9`까지 검증됐고 Runtime EffectCatalog의 420633 row도 `1`이다.
runtime 단순화 뒤 formatVersion 3 loader는 `visualProgramSidecarRequired` marker와 무관하게
VisualProgram sidecar를 항상 parse/validate/stage하므로 새 Client code의 실행 분모는 `9/9`다.
현재 게시 catalog 191행은 아직 전환용 legacy 6-field이므로 최종 4-field Full Publish 증거는 별도
통합 단계로 남는다. 화면 fidelity는 새 Client/data에서 사용자가 판정한다.

| 범위 | 구현 | 자동 검증 | 제품/수동 상태 |
|---|---|---|---|
| Whirlwind source denominator | `5 + 3 + 1 = 9/9` | canary/boss/unit PASS | authoring 완료 |
| Baked Trail geometry | 409 samples, 1.2초 clamp | builder check PASS | material은 bounded reconstruction |
| Point Light | firstEdge typed attachment | packet/focused test PASS | sibling-template inference |
| Pattern cue | `0..2133ms` authoring cue | binding/cue focused PASS | runtime effect row 1 |
| VisualProgram corpus/runtime | rows 135 / programs 14 | source runtime/published sidecar byte-identical; corpus schema SHA 1건 stale | marker와 무관하게 always-load |
| Gameplay | combined bootstrap v13 | Validate/Publish PASS | Server restart 필요 |
| Item bootstrap | v2, 7 items 생성 | Server Release/contract PASS | missing bootstrap blocker 제거 |
| Runtime EffectCatalog 420633 | 04:09 legacy 6-field 게시본 | row `1`, payload SHA 일치 | 4-field Full Publish는 deferred |
| 화면 fidelity | 에이전트 미실행 | 자동 PASS 없음 | 사용자 판정 필요 |

새 전용 harness는 만들지 않았다. 기존 builder check, boss validator, Python unit,
ClientFrontendHarness와 Server contract test를 재사용했다.

## 1. Whirlwind 9/9 canary

대상 identity:

```text
patternId        VALTAN_WHIRLWIND
semantic stage   SPIN
actionId         valtan.attack.whirlwind.active
source action    420633
source stage     2
runtime clip     mesh_att_battle_20_03
effect asset     effect.valtan.pattern.420633.active
```

Authored document
`Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json`은 아홉 element를 소유한다.

### 1.1 Base carrier 5개

기존 실행 carrier 세 개에 Dust 두 개를 더해 base-visible 분모를 `5/9`로 닫았다.

| occurrence | carrier | material | 결과 |
|---|---|---|---|
| notify006/emitter7336 | sprite | `fx_d_pa_atta_05_11_ad` | 기존 실행 보존 |
| notify006/emitter7334 | mesh | `fx_s_me_missiletrail_01_1_ts_tr` | 기존 실행 보존 |
| notify006/emitter7335 | mesh | `fx_s_me_missiletrail_01_1_ts_tr` | 기존 실행 보존 |
| notify005/emitter7034 | sprite | `fx_e_pa_fd_04_1_tr` | grouped material와 DynamicParameter 연결 |
| notify005/emitter7035 | sprite | `fx_c_pa_aura_02_tr` | aura profile과 exact DDS 연결 |

emitter7034는 dissolve density, alpha power, emissive tiling, lamp time의 source curve를 보존한다.
emitter7035는 `fx_a_glow_009`, `fx_a_cloud_026` source resource를 사용한다.

### 1.2 AnimationTrail 3개

다음 세 source occurrence는 일반 ribbon이나 root-history camera-facing trail로 바꾸지 않았다.

```text
notify004/emitter5259  fx_d_pa_trail_07_05_tr
notify004/emitter5260  fx_d_pa_atta_09_01_ad
notify004/emitter5258  fx_o_pa_ribbonmaster_03_04_tr
```

세 base row는 hidden/disabled 상태를 유지하고
`ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1` supplemental projection만 활성화한다. 동일한 immutable
history에서 매 시각의 `firstEdge/secondEdge`를 직접 strip geometry로 제출한다.

```text
history ID:
  valtan.420633.animnotify-trails-479.baked-edges
source object:
  mn_rpbf_00_420621_0_3_0.animnotify_trails_479
source package SHA-256:
  09c7968667dbedb43feb3bed18a1985a470f8a8848c468d5baf6bf0980314e91
sample count:
  409
source end:
  3.200000047683716s
playback clamp:
  1.2000000476837158s
artifact SHA-256:
  46bf2e83ace7d798a2ff34489cc4eb223a716ac75159d799f6dd306707112a64
```

geometry/sample identity는 exact evidence다. 세 UE3 parent material은 현재 공용
grouped-translucent 식으로 실행되므로 material fidelity는 `BOUNDED_RECONSTRUCTION`이다.

### 1.3 Point Light 1개

notify009/emitter6823의 `PointLightComponent_1335`를
`LIGHT_BAKED_EDGE_ATTACHMENT_V1` packet으로 연결했다. 기존 transient presentation light 경로를
재사용하며 두 번째 Light manager를 만들지 않았다.

```text
brightness                  10.0
archetype radius            200 UE units
falloff                     2.0
color                       inherited white
packet start                0.07953999936580658s
packet duration             1.120460033416748s
packet end                  1.2000000327825546s
history clamp               1.2000000476837158s
attachment lane             FIRST_EDGE
```

source label `EndControl_01`은 현재 Valtan WModel의 87 bones/0 sockets에서 해석되지 않는다.
FIRST_EDGE 연결은 같은 notify family의 sibling template을 근거로 한
`SIBLING_TEMPLATE_INFERRED`다. source-exact socket attachment라고 기록하지 않는다.

## 2. Animation binding, cue, 시간 소유권

origin/main의 pattern binding parser/runtime 변경을 현재 dirty worktree에 semantic hunk로 통합했다.
binding의 `clip`은 단일 string과 ordered array를 모두 지원하며, array를 가진 action은 Server stage
age로 clip chain을 진행한다.

현재 Whirlwind active의 실제 binding은 단일 clip이다.

```text
actionId: valtan.attack.whirlwind.active
clip:     mesh_att_battle_20_03
```

기존 계획에 있었던 `20_03 -> 20_04 -> 20_03` Whirlwind chain 표기는 실제 authoring 데이터와
일치하지 않아 제거했다.

`Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`에는 다음 제품 cue가 존재한다.

```text
bindingId       cue.valtan.whirlwind.active
patternId       VALTAN_WHIRLWIND
stageId         SPIN
actionId        valtan.attack.whirlwind.active
effectAssetId   effect.valtan.pattern.420633.active
startMs         0
endMs           2133
```

`2133ms`는 Server semantic SPIN stage와 effect cue의 window다. baked history clamp `1.2s`는 source
Trail/Light local sample 범위다. 서로 다른 시간을 하나로 정규화하지 않는다.

## 3. VisualProgram corpus/runtime

기존 corpus/runtime builder를 확장해 Whirlwind typed Trail/Light packet을 같은 class-neutral 경로에
포함했다.

```text
corpus presentation schedules   35
corpus visual rows             135
corpus artifact SHA-256
  5407580676ca5d9123a762b384f24184abcbf61dea13c423634236378f297873

runtime programs                14
runtime rows                   135
runtime artifact SHA-256
  03e6d76db542d2dd78c200da40128203d89bbfaf0ca7eac86154a8210227fe32
```

Trail 세 행은 explicit edge pair geometry를, Light 한 행은 같은 history의 firstEdge attachment를
소비한다. 409-sample history owner는 한 개이며 target별로 별도 복제하지 않는다.

위 corpus/runtime artifact check는 packet 생성과 identity를 증명한다. formatVersion 3 loader는
`visualProgramSidecarRequired`와 무관하게 sidecar를 항상 parse/validate/stage하므로 새 Client code의
구조적 실행 분모는 `9/9`다. 실제 색·밀도·궤적의 visual PASS는 별도 사용자 판정이다.

post-save 감사에서 current source runtime과 published sidecar는 byte-identical이고 Valtan program SHA와
supplemental 4개도 불변이었다. 다만 source corpus input 38개 중 schema raw SHA 1건이 stale해 현재
global corpus `--check`는 provenance 재생성을 요구한다. 이는 420633 semantic drift가 아니며 최종 4-field
full Publish 전에 함께 갱신할 전역 생성물 경계다.

## 4. Gameplay bootstrap v13과 Server

local gameplay v12의 `comboAdvanceMs`와 origin Valtan timing의 `hitDelayMs`를 한쪽 선택으로
덮어쓰지 않고 combined bootstrap v13으로 합쳤다.

```text
SKILLSTAGE:
  actionDurationMs, hitTimeMs, comboAdvanceMs, inputOpenMs, inputCloseMs 유지

PATTERNSTAGE:
  hitIntervalMs 다음, serverDamageProfileId 앞에 hitDelayMs 추가

Valtan hit threshold:
  hitDelayMs + appliedHitCount * hitIntervalMs
```

`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` header는 version 13이고 Server loader, Balance Tool,
contract fixture가 같은 필드 순서를 소비한다.

Gameplay Validate/Publish 결과:

```text
profiles    6
skills    136
patterns   33
stages    124
result    PASS
```

### 4.1 Item bootstrap

`Data/Items/ItemCatalog.json`에서
`Server/Bin/DataFiles/Items/Items.bootstrap`을 생성했다.

```text
header   LOSTARK_ITEM_BOOTSTRAP 2 7
items    7
```

따라서 최초 오류였던 `Missing item bootstrap`의 필수 파일은 현재 존재한다. JSON fallback이나
hardcoded item으로 오류를 숨기지 않았다.

### 4.2 Server build

- Server Release build: PASS
- Release `Server.exe --contract-test`: `failures=0`
- Debug compile: 완료
- Debug link: 실행 중인 Server가 출력 파일을 점유해 `LNK1104`

사용자의 실행 중 Server는 종료하지 않았다. 그 프로세스는 새 gameplay bootstrap v13과
Items bootstrap 생성 이전 실행일 수 있으므로 사용자가 재시작해야 새 계약이 실제 Server process에
적용된다.

## 5. 자동 검증

### 5.1 PASS

| 검증 | 결과 |
|---|---|
| baked history builder/check | 409 samples, artifact SHA 일치 |
| Whirlwind canary check | base 5 + Trail 3 + Light 1, `9/9`, blocker 0 |
| boss pattern mapping validator | PASS |
| canary Python unit | `15/15 PASS` |
| focused Valtan frontend binding/cue | `failures=0` |
| Balance Tool v13 round-trip | PASS |
| VisualProgram corpus check (04:09 checkpoint) | schedules 35, rows 135, SHA 일치 |
| VisualProgram runtime check | programs 14, rows 135, SHA 일치 |
| Gameplay Validate/Publish | PASS |
| Valtan world destruction Validate | PASS |
| Client Debug `/t:ClCompile` | PASS, errors 0 |
| Server Release build | PASS |
| Server Release contract test | `failures=0` |
| 04:09 Full `Publish-Effects.ps1 -Mode Publish` | exit 0, Effects 192, Components 1, visual-program sidecar 파일 생성 |
| Runtime 420633 row/content SHA | row 1, `0c27dee05f03e053f64587ca30e0a73e0be192fd9784b8150fa76179cd2c838d` |
| Client Release full Build | exit 0, `Client.exe` 생성, warnings only |
| `git diff --check` | PASS |

현재 post-save 재감사에서는 baked history/canary/mapping은 계속 PASS지만 global corpus check가 schema raw
SHA 1건 stale로 fail-close한다. 따라서 위 corpus PASS는 04:09 checkpoint이며 최종 publish 직전 재생성이
필요하다.

### 5.2 전환 게시본과 최종 4-field Publish 경계

04:09 snapshot의 full publisher 실행 자체와 runtime row admission은 성공했다.

```text
command result:
  Publish-Effects.ps1 -Mode Publish
  exit 0
  PASS: published 192 Effects, 1 Components, visual-program sidecar

catalog LastWrite:
  2026-08-21 04:09:04
sidecar LastWrite:
  2026-08-21 04:09:04

effectAssetId:
  effect.valtan.pattern.420633.active
runtime row count:
  1
content SHA-256:
  0c27dee05f03e053f64587ca30e0a73e0be192fd9784b8150fa76179cd2c838d
```

이 게시본의 실제 catalog marker와 direct row shape는 다음과 같다.

```text
visualProgramSidecarRequired: false
direct rows: legacy 6-field 191 / canonical 4-field 0
```

runtime 단순화 뒤 direct catalog의 정본은 4필드이고 loader는 기존 6필드를 전환 입력으로만 허용한다.
formatVersion 3 loader는 marker가 `false`여도 VisualProgram sidecar를 항상 parse/validate/stage한다.
현재 source runtime과 published sidecar는 byte-identical이고 420633 supplemental 4개가 동일하므로 새
Client code에서 Trail 3 + Light 1 projection은 구조적으로 연결된다.

다만 04:09 catalog는 현재 Artist/DimensionMaster 저장보다 오래된 전환 게시본이다. 모든 authored 저장이
자연스럽게 끝난 뒤 별도 통합 단계에서 full Publish를 한 번 실행해 4-field catalog를 만들고 Client를
재시작한다. 이번 단계에서는 global publisher를 독점하거나 재실행하지 않았다.

### 5.3 남은 경계

미완료는 다음과 같다.

- Debug Server 재링크와 새 process 재시작
- Debug Client의 현재 전체 tree 최종 link
- 모든 authored 저장 종료 뒤 4-field Full Effect Publish와 Client 재시작
- 제품 Valtan에서 420633 effect 실제 spawn/화면 확인
- 3연 공격, 돌진, donut, 6방향 발판 등 나머지 pattern effect의 source-exact 확장
- 사용자 visual fidelity 판정

## 6. 사용자 전용 화면 검증

에이전트는 Client 또는 Valtan 화면을 실행·조작하지 않았고 캡처를 만들지 않았다.
따라서 `manual first pixel`, `eye smoke`, `visual PASS`, 원작과 같은 색·밀도·궤적·타이밍은 모두
미판정이다.

final 4-field Publish와 Client/Debug Server 재시작 뒤 사용자가 직접 확인할 대상은 다음과 같다.

```text
Lobby -> Valtan -> VALTAN_WHIRLWIND / SPIN
```

화면에서는 base Dust 5 carrier, 세 baked-edge strip, first-edge Light의 존재와 0..2133ms cue 종료를
분리해 판정한다. FIRST_EDGE Light의 위치는 sibling-template inference이므로 source oracle과 다르면
attachment tuning 대상으로 되돌리고 exact로 승인하지 않는다.

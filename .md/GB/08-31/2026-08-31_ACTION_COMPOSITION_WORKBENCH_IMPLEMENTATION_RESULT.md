# Action Composition Workbench 구현 결과

## 결론

이번 변경은 기존 Animation Tool에 Pattern 저작 UI를 계속 누적하지 않고, F1에서 독립적으로
여는 `Action Composition Workbench`를 추가했다. 저작의 루트는 파일 목록이나 animation clip
index가 아니라 stable `Pattern ID`이며, 선택 Pattern 아래의 `Stage ID`와 animation occurrence를
기준으로 gameplay와 presentation 데이터를 한 화면에 join한다.

Workbench 자체는 새 runtime 정본이 아니다. 다음 기존 typed source owner를 stage하고, 공통
writer generation에서 Validate/Publish한 Product를 실제 canonical loader로 다시 읽는 orchestration
shell이다.

```text
Pattern
└─ Stage (Server wall clock / Stage Role)
   ├─ Animation Sequence Slot occurrence[]
   ├─ Server Collider + Hit Schedule
   ├─ Counter success -> same-Pattern GROGGY Stage
   ├─ Player Reaction / Grab Release velocity·duration·yaw
   ├─ Effect invocation
   └─ Sound cue occurrence
```

## 실제 반영 범위

### 1. 독립적이고 resize 가능한 Workbench

- `CActionCompositionWorkbench`를 새 Client tool로 추가하고 project/filter에 등록했다.
- F1 Developer Tools에서 기존 Animation Clip Tool과 별도로 열 수 있다.
- 첫 크기만 viewport에 맞춰 제안하며 `AlwaysAutoResize`, hard minimum, `NoResize`를 사용하지
  않는다.
- canonical join, model preview 또는 Server 상태가 실패해도 전체 창을 early-return으로 숨기지
  않는다. 이전 admitted view는 진단용으로 남기되 모든 Save/재생/Server mutation을 차단한다.
- raw filesystem 수만 건 대신 선택 Pattern과 실제 join된 owner/resource만 표시한다.

### 2. Pattern 중심 Browser와 Sequence Slot

- 57개 canonical Valtan Pattern과 255개 Stage를 stable ID로 읽는다.
- model-independent Valtan animation intake 265개를 별도 Sequence browser에 의미 단위로
  표시한다.
- Sequence를 preview한 뒤 선택 Stage에 `Replace Stage Slots` 또는 `Append to Stage Slots`로
  넣을 수 있다.
- occurrence는 vector index가 아니라 `.composition.clip.NN` stable ID를 사용한다. 삭제 뒤
  Append도 기존 ID와 충돌하지 않는 다음 빈 번호를 선택한다.
- Replace는 같은 clip occurrence를 재정렬할 때 기존 stable ID와 mapping basis를 보존한다.
  다른 clip으로 바꾸면 새 ID와 `PROJECT_AUTHORED` mapping을 발급하며, 기존 Effect·Sound·Shake가
  그 occurrence를 참조하면 명시적 remove/retarget 전까지 fail-closed한다.
- Stage 안에서 occurrence reorder/remove/trim을 지원한다. 마지막 slot이 무한/native-duration
  policy인 동안 뒤에 Append하는 잘못된 clock 구성은 거부한다.
- 선택 animation chain은 기존 Create Pattern transaction으로 전달하며, 생성 성공 뒤 새
  Pattern을 canonical reload해서 Workbench에서 바로 선택한다.

### 3. 내부 공백과 Server Stage clock

- Pattern 시간축의 기준은 Server Stage `durationMs`다.
- animation occurrence의 wall time보다 Stage가 길면 그 차이를 다음 Stage 전
  `HOLD_LAST_POSE` trailing gap으로 표시하고 저장한다.
- 도끼 Pattern처럼 마지막 동작 뒤 공백을 늘리는 작업은 Stage Duration을 늘리는 방식으로
  typed gameplay source에 저장한다.
- 임의의 두 slot 사이에 무명 blank key를 삽입하는 기능은 현재 지원하지 않는다. 의미 있는
  중간 공백은 별도 Stage로 모델링해야 한다.

### 4. Collider와 Effect의 권위 분리

- 기존 Server hit가 있는 canonical Stage에서는 collider를 편집·제거한다. 신규 collider 추가는
  non-WAIT manual audition Stage에서만 허용하며 canonical no-hit Stage에는 임의로 만들지 않는다.
- 지원 shape는 `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS`, `SIX_DIRECTIONS`이며 radius,
  angle, length, half-width와 Damage Profile을 조정한다.
- hit delay/interval/count 또는 기존 explicit hit offset을 같은 Stage clock에서 확인한다.
- push range/duration, knockdown/down duration도 Server player reaction owner에 저장한다.
- Effect row와 Collider row는 Sequencer에서 같은 Stage에 나란히 보이지만, 보이는 Effect mesh가
  hit authority나 collider로 자동 승격되지는 않는다.
- 현재 inline authoring은 Stage당 하나의 hit shape/schedule이다. 다중 collider stack과 Effect
  element에서 collider를 자동 생성하는 기능은 완료 범위가 아니다.

### 5. Counter에서 Groggy로 가는 typed edge

- Counter는 의미 없는 `true/false` 하나로 저장하지 않는다.
- `WINDUP` Stage가 Counter window를 소유하고, 성공 시 같은 Pattern의 정확한 `GROGGY`
  Stage/action stable ID를 가리킨다.
- UI에서 `Stage Role`, `Counter Enabled`, `Counter Success Groggy`를 함께 조정한다.
- 새 topology 저장 순서는 기존 edge disable -> Stage Role 변경 -> 새 edge enable/retarget으로
  고정해 중간 문서가 invalid해지는 회귀를 제거했다.
- GROGGY Stage가 없으면 Counter 활성화를 거부한다.

### 6. 잡기 후 날리기 수치 튜닝

- 기존 `RELEASE_GRABBED_PLAYERS` action이 있는 Stage를 선택하면 다음 수치를 Pattern Detail에
  표시하고 typed gameplay source로 저장한다.
  - Release Velocity: 0..50 m/s
  - Release Duration: 0..5000 ms
  - Release Rotation / Yaw Offset: -180..180 degrees
- 따라서 뒤돌아 잡기 후 반대 방향으로 날아가는 경우 yaw offset을 눈으로 조정할 수 있다.
- 기존 grab/capture/release topology와 왼손 attachment를 보존하는 수치 튜닝이다. 아무 Pattern에
  새 grab topology 전체를 생성하는 기능은 이번 범위가 아니다.

### 7. Sound와 Effect 관계

- Pattern Sound cue는 typed source owner에서 add/edit/remove하고 별도로 Save한다.
- animation occurrence 변경 전에 Sound의 `clipOccurrenceId` dependency를 후보 Stage에 대해
  검증한다. unsaved Sound draft가 있으면 Pattern/Sequence Save와 canonical reload를 차단한다.
- Sound Save는 Pattern canonical writer transaction에 합치지 않는다. 같은 read-generation
  admission과 occurrence dependency validation을 통과한 뒤 기존 Sound owner에 별도 CAS/atomic
  replace하고, committed source와 local reload 상태를 Pattern commit과 분리해 표시한다.
- Sound runtime 적용은 저장 직후로 가장하지 않는다. exact Pattern revision이 Server-active가 된
  뒤 `Retry Apply`로 실제 replay consumer에 적용한다. 적용 전·후 Server revision을 비교하고
  consumer-ready receipt를 그 exact revision에 고정한다. 그 전이나 적용 실패 시 committed source를
  숨기지 않고 runtime apply pending으로 남겨 Complete/Restart/Next를 막는다.
- Effect invocation은 typed Details에서 add/full update/exact remove한다. exact clip occurrence와
  source start/end, stop/repeat, anchor/follow, local position/rotation/scale, scale policy를 canonical
  `Data/Valtan/Valtan.presentation.json`에 저장한다. generated
  `Valtan.patterneffectcues.json`은 projector가 만드는 read-only Product다.
- invocation의 연결·시간·배치와 Effect asset body 편집은 분리한다. 후자는 Save/Publish/Reload된
  admitted occurrence와 전체 cue field가 일치할 때만 선택 `effectAssetId`의
  `Data/Effects/Authored/*.effect.json`을 Effect Tool로 여는 exact deep-link다. draft-only row에
  asset-only fallback은 없다.

### 8. Save, Publish, canonical reload, Server playback

`Save Pattern + Validate + Publish`는 generated
`Valtan.patternbindings.json`이나 `Valtan.patterneffectcues.json`을 직접 저장하지 않는다.

```text
typed source draft
-> validate
-> common writer generation commit
-> Product projection
-> local canonical graph reload
-> immutable runtime candidate publish
-> exact Server-active revision 확인
```

- source/Product commit과 Server runtime activation 상태를 분리해 표시한다.
- presentation generation이 바뀌어 현재 world에서 적용할 수 없으면 known-NACK를 보내지 않고
  `REENTRY_REQUIRED`로 표시한다.
- Complete Play와 Restart는 exact Server-active definition revision이 확인된 경우에만 열린다.
- Restart는 arena 전체 reset이 아니라 현재 또는 완료된 exact occurrence CAS다.
- `Fresh / Restore Arena`는 벽과 arena 상태를 복구하는 별도 명령이다.
- `Queue as Next`는 현재 Pattern을 끊지 않고 선택 Pattern을 기존 Server pending 경로에 넣는다.
- `Pattern Flow...`는 기존 canonical Flow editor를 열며 새로운 중복 Flow 정본을 만들지 않는다.

## 자동 검증 결과

### 데이터와 focused contract

- 현재 소스에서 Effect transaction/Workbench, Sequence identity, canonical typed patch,
  writer/read gate/ownership, Balance, Workbench regression, presentation, Sound, manual Stage와
  topology pipeline focused suite: 164/164 PASS
- Valtan Pattern Master V2: 74/74 PASS
- requested Pattern coverage validator: PASS
  - Product 33 / Encounter 57
  - `silence`, `stone creation/destruction/roar` stable Pattern은 아직 없음
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
  - 57 patterns
  - 255 stages
  - 52 audition rows
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: PASS

### 표준 빌드와 native runtime contract

- Debug / Core: PASS
  - evidence: `out/BuildPipeline/runs/20260830T211800422Z-debug-core-7889f7b0.json`
  - Restart: 28/28
  - Flow: 12/12
  - tuning: 14/14
  - canonical graph native loader: 57 patterns / 255 stages, 3/3
  - Complete Play inventory: 33 patterns
  - Effect invocation production authoring seam: 9/9, cross-clip update 포함
  - Character Select private/shared world live contract: PASS
- Release / Product: PASS
  - evidence: `out/BuildPipeline/runs/20260830T211251730Z-release-product-4354684d.json`
  - Engine, Shared, Server, Client: error 0
  - compiled shader closure: PASS
  - Product Effect WARP pixels: V1=1352, V2=1352

현재 소스의 첫 Release 실행은 Client production seam이 17개라는 오래된 build-profile fixture가
새 source 두 개를 거부해 실패했다. fixture를 exact 19-source 의미 목록으로 교정한 뒤 표준
Release / Product 전체를 다시 실행해 error 0으로 닫았다. Debug Core의 새 PCH-less native test는
Windows `byte`와 `std::byte` 모호성을 드러냈고 기존 harness 방식과 같은 forced include 경계를
적용한 뒤 direct harness와 표준 Debug / Core를 모두 다시 통과시켰다. 최초 실패를 최종 PASS로
덮어쓰지 않고 원인과 재검증을 함께 기록한다.

Debug Core에는 실제 `CValtanPatternTree::Load`를 호출하는
`ValtanPatternAuditionServiceHarness`를 편입했다. 따라서 Python source-token 검사만 통과하고
실제 Pattern 목록·Complete Play가 비는 회귀를 Core 성공으로 처리하지 않는다.

## 요청 Pattern 콘텐츠의 현재 경계

- `VALTAN_FLOOR_WIPE_130`은 첫 타격 뒤 266 ms, interval Stage 500 ms의 Server clock/gap을
  canonical Product에서 읽고 Details로 조정할 수 있다.
- `VALTAN_CATCH_BREATH`의 기존 release action은 현재 24 m/s, 500 ms, yaw 0 deg이며 세 값을
  Details에서 편집할 수 있다. 180도 보정의 실제 화면 방향은 사용자 visual 확인 전이다.
- `VALTAN_SIX_PIZZA`는 target snapshot Effect 경로가 있으나 sector 기준 facing을 replicated
  player yaw로 둘지 Server-locked boss facing으로 둘지 아직 하나의 제품 계약과 native
  data→snapshot→matrix oracle로 확정하지 않았다.
- `VALTAN_WARP`는 500 ms delay, 20 m/s, 8 m의 반복 rush leg를 갖지만 start/end portal을 각각
  저작하는 두 Effect invocation 수직 슬라이스는 아직 없다.
- `VALTAN_TRASH`는 counter/capture/left-hand/branch 골격이 있으나 실패 뒤 다시 기 모으기와
  재돌진으로 이어지는 반복 branch는 아직 없다.
- stagger 76과 triple counter는 Encounter reference이며 Product Pattern/Complete Play 대상이 아니다.
  `VALTAN_STRUGGLING`도 animation Sequence는 있으나 중앙 이동 anchor/flag는 아직 없다.
- silence와 stone creation/destruction/roar는 stable Product Pattern이 아직 없다.

따라서 이번 결과는 요청 Pattern 전부의 콘텐츠 완성이 아니라, 위 콘텐츠를 안전하게 늘리고
튜닝할 수 있는 Pattern 중심 저작 수직 슬라이스와 첫 실제 데이터 적용 범위다.

## 사용자 수동 검증 대기

저장소 규칙에 따라 에이전트가 Client UI를 자율 실행하거나 visual PASS를 대신 판정하지 않았다.
다음 항목은 사용자의 첫 화면 확인 전까지 PASS가 아니다.

1. F1에서 `Action Composition Workbench`가 독립적으로 열린다.
2. 창을 작게 줄이고 다시 늘릴 수 있으며 Details/Sequencer/Data Files가 사라지지 않는다.
3. Valtan Pattern 목록과 Complete Play inventory가 보인다.
4. Pattern 선택 시 Stage, Sequence Slot, gap, Collider, Counter/Groggy, Sound/Effect 관계가 보인다.
5. 기존 Effect 목록과 선택 Pattern의 Effect 관계가 보인다.
6. Save 뒤 재진입/Server-active revision에서 Complete Play, Restart, Next가 동작한다.

## 의도적으로 완료라고 부르지 않는 경계

- Camera/Light/World lane은 inspection/deep-link이며 key add/drag/trim/save adapter가 없다.
- Sound/Camera/World에는 공통 local seek/stop transport가 없다.
- Effect invocation은 typed Details add/update/remove를 지원하지만 일반 timeline block
  drag/trim/key authoring은 아직 없다. Effect asset 내부 element 편집은 Effect Tool deep-link다.
- Stage당 다중 collider, bone/Effect element anchor에서 collider를 새로 만드는 기능은 없다.
- 임의 중간 blank key는 없고 trailing gap 또는 별도 Stage만 지원한다.
- 기존 release action 수치는 편집하지만 새 capture/grab/release topology 전체 생성은 지원하지
  않는다.
- Pattern source와 Sound source는 각각의 typed owner commit이다. dependency admission과 적용
  순서는 공유하지만 하나의 가짜 다중-owner JSON이나 단일 atomic Save로 표현하지 않는다.
- 위 경계 때문에 현재 결과를 Unreal/Unity Sequencer 전체 완성이라고 부르지 않는다. Pattern
  Stage/Animation/Collider/Counter/Grab 수직 슬라이스가 실제 source에서 Server consumer까지
  닫힌 상태다.

## PR 구성 주의

공유 worktree에는 Action Composition과 무관한 DimensionMaster Effect, portal visual carrier,
Level navigation, Effect unlink/build guard 변경이 함께 존재하고, 현재 branch는 `origin/main`보다
3커밋 뒤다. 따라서 아직 PR-ready가 아니며 `git add -A`로 한 커밋을 만들지 않는다. 최소한 다음
검증 단위로 hunk를 분리한다.

1. canonical source/Product writer와 reader admission
2. resizable Workbench shell과 semantic Browser
3. typed Stage/Sequence/Collider/Counter/Grab authoring
4. Create Pattern transaction
5. Complete/Restart/Next/Flow protocol
6. 문서와 focused/native 검증

사용자 화면 검증 결과는 위 자동 검증과 별도로 이 문서에 추가한 뒤 PR 완료 상태를 판정한다.

## 2026-08-31 F1 / Workbench 프레임 회귀 교정

### 구현 상태

- F1 root 창은 최초 크기만 제안하고 `SetNextWindowSizeConstraints`를 사용하지 않는다. 이번 교정에서
  추가했던 좁은 폭 전용 card 전환과 Animation Clip Tool 크기 정책 변경은 회수했다. 즉 F1 크기
  제한 제거만 남겼다.
- Action Composition Workbench의 ImGui 활성 상태 계산은 physical Product admission을 호출하지
  않는다. 기존에는 매 frame
  `Get_ServerActivePatternRevision -> Is_CurrentPresentationBaselineIntact -> presentation artifact
  read/SHA-256` 경로를 실행했다. 현재 Toolbar와 Sound runtime 상태 표시는 memory-only
  `Observe_ServerActivePatternRevision`을 사용하며, exact Product/Sound admission은 Complete Play,
  Restart, Next, Sound Apply의 실제 command edge에서만 실행한다.
- Sequencer는 중앙 작업 공간의 54%를 기본 사용하고 280..680 px 범위로 표시된다. Browser/Preview는
  26%로 줄여 joined track을 주 작업 화면으로 만들었다.
- 선택 Stage의 Sequencer 상단에 `Selected Stage Gap (ms)`를 추가했다. 값은
  `Animation wall + gap = Stage duration`으로 typed Balance draft에 stage되고, 0 ms는 `EXACT`,
  양수는 `HOLD_LAST_POSE`로 저장된다. `Save Pattern + Validate + Publish` 전에는 source draft이며
  generated Product를 직접 수정하지 않는다.

### 최신 자동 검증

- F1 semantic resource / navigation: 8/8 PASS
- F1 arena preservation / resize: 13/13 PASS
- Action presentation Workbench: 43/43 PASS
- Action Composition Sound owner: 14/14 PASS
- Action Composition regression oracle: 21/21 PASS
- 합계: 99/99 PASS
- 변경 파일 대상 `git diff --check`: PASS

### 아직 PASS가 아닌 항목

- 이 교정 뒤 표준 Debug Product 빌드는 아직 실행하지 않았다. 검증 시점에 기존
  `Client.exe` PID 49324와 `Server.exe` PID 53964가 출력물을 점유하고 있어 build guard가 링크를
  차단했다. 프로세스를 사용자가 종료한 뒤 같은 source revision으로 다시 빌드해야 한다.
- Workbench 지속 FPS, 확대한 Sequencer 높이와 gap drag/save 결과는 새 EXE에서 사용자 수동
  검증이 필요하다. 실행 중이던 화면은 수정 전 EXE이므로 이번 교정의 visual 증거가 아니다.

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
- Replace는 exact clip을 먼저 재사용하고, 기존/신규 양쪽에서 유일한 `_start/_loop/_end` 역할만
  같은 논리 occurrence로 재사용한다. 역할 안에서 clip이 바뀌면 `PROJECT_AUTHORED`로 표시하고
  Effect·Sound·Shake source window를 후보 graph에서 다시 검증한다. 반복 loop처럼 역할이
  모호하거나 새로 추가된 box는 새 stable ID를 발급하며 dependency는 임의 대상을 따라가지 않고
  fail-closed한다.
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
- Arena Clone의 공통 Play/Seek/Stop은 Animation, Effect invocation과 collider mirror를 같은 Pattern
  clock에서 검증한다. Sound/Camera/World는 현재 inspection 또는 각 runtime owner 경로이며 clone의
  공통 local transport에 포함됐다고 보지 않는다.
- Effect invocation은 typed Details add/update/remove와 timeline body move를 지원하고,
  `cue_end` invocation은 오른쪽 trim으로 끝 시간을 조정한다. Effect asset 내부 element 편집은
  Effect Tool deep-link다. Sound cue는 point occurrence 이동만 지원하며 가짜 duration trim을 만들지
  않는다.
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

## 2026-08-31 Create Pattern 3 FPS / Details Save 교정

### 재현 원인과 소스 구현

- 사용자가 수정 전 Debug EXE에서 `Create New Pattern` 탭을 열었을 때 2.7~3 FPS를 확인했다.
  `Composition Session` 창이 가려진 것과는 무관하다.
- 탭 render wrapper가 `m_bValtanPatternMasterLoadAttempted`를 검사하면서도 reload 전에 latch를
  세우지 않았다. 그 결과 탭이 열린 동안 매 frame `Reload_ValtanPatternMaster`가 실행되어
  canonical Pattern tree와 Sound/Shake/Combat Object Sound source를 반복해서 읽고 join했다.
- reload 호출 전에 attempt latch를 세워 최초 command edge에서만 file-backed load를 실행한다.
  실패해도 render rate로 자동 재시도하지 않으며 명시적 Reload command가 재시도를 소유한다.
- `Composition Details` 상단에 `UNSAVED PATTERN DRAFT`와 `Save Pattern`을 추가했다. 버튼은
  admitted Pattern draft가 실제로 dirty이고 별도 Sound owner dependency가 깨끗할 때만 열린다.
  Details는 save request만 남기고, 같은 frame의 모든 window가 immutable Pattern/Stage view 사용을
  끝낸 뒤 `Save_Publish_Reload`를 실행한다. 따라서 canonical storage 교체 뒤 stale pointer로 다른
  패널을 계속 그리지 않는다.
- `Pattern` owner 표시는 `Pattern Root / Flow`로 바꾸고, branch/action Logic은 Pattern root와 같은
  것이 아니라 Sequencer `Logic` lane에서 선택해 `Gameplay / Logic / Collider` typed detail로
  편집한다는 안내를 추가했다.

### 이번 교정 자동 검증

- 새 회귀 oracle은 Create Pattern 최초 load latch가 reload보다 먼저 실행되는지 검사한다.
- 새 회귀 oracle은 Details Save가 직접 canonical storage를 교체하지 않고 모든 window render 뒤
  처리되는지 검사한다.
- focused Workbench/F1 suite: 138/138 PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- requested Pattern coverage validator: PASS, Product 33 / Encounter 57
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: PASS,
  57 patterns / 255 stages / 52 audition rows
- 변경 파일 대상 `git diff --check`: PASS

### 빌드와 수동 확인 대기

- 이 소스 교정의 새 Debug/Release EXE는 아직 생성하지 않았다. 검증 시점에 사용자가 확인 중인
  `Client.exe` PID 24936, `Server.exe` PID 60348와 Visual Studio `MSBuild` PID 50784/55788가 같은
  checkout/output을 사용 중이므로 자동 runner를 중첩하지 않았다.
- 사용자가 실행과 VS build를 종료한 뒤 exact source revision에서 Debug `FullDiagnostic`과 Release
  `Product`를 실행해야 한다.
- 새 Debug EXE에서 `Create New Pattern` 탭을 30초 이상 열어 둔 지속 FPS, Details의 dirty 표시와
  `Save Pattern` 결과는 사용자 수동 판정 전까지 visual PASS가 아니다.

## 2026-08-31 Save rollback / Boss Pattern Blueprint / Preview 권위 분리

### Save 실패 재현과 교정

- 사용자 화면의 실제 실패 이유는 Stage duration 8,000 ms가 아니라
  `Pattern Sound dependency does not resolve exactly one candidate Pattern/Stage:
  VALTAN_BACKSTEP_ATTACK/SWEEP`였다. 저장 전 Sound graph 검증이 Complete Play용
  `m_PlayableInventory` 필터 목록을 전체 canonical dependency 목록으로 잘못 재사용해,
  Product에는 존재하지만 개별 Complete Play 대상이 아닌 legacy-compatible Pattern을 누락했다.
- Browser는 계속 Complete Play inventory만 표시한다. Save와 manual Stage topology Sound
  preflight만 `Gimmicks + Rotation` 전체 canonical Pattern을 수집하는 별도 helper를 사용한다.
  화면 목록과 저장 dependency closure가 다시 섞이지 않게 경계를 분리했다.
- Details와 Session Save는 마지막 결과를 `LAST SAVE: SAVED` 또는 `LAST SAVE: FAILED`로
  보존하고, 실패 시 정확한 validator 원인을 바로 아래 표시한다.
- 후속 Composition 즉시 튜닝 slice에서
  `Data/Valtan/Valtan.gameplay.json`의 `VALTAN_HIGH_JUMP/AIRBORNE` 정본을 8,000 ms로
  변경하고 Product를 다시 투영했다. LOOP animation wall과 target-axe lifetime도 8,000 ms를
  따르며, axe hit은 각 object 생성 기준 +1,200 ms를 유지한다.

### Boss Pattern Blueprint와 Details 경계

- 선택 Pattern용 일곱 번째 opt-in 창
  `Composition Boss Pattern###CompositionBossPatternWindow`를 추가했다. JSON이나 catalog를
  render에서 다시 읽지 않고, 이미 조립된 effective `VALTAN_PATTERN_VIEW`를 순수 graph model에
  투영한다.
- graph는 Stage node와 authored/derived branch edge, default/selected/maximum 경로 시간을
  표시한다. node와 edge 선택은 기존 stable selection/Details를 재사용하고, outcome 선택은
  preview-only route다. 임의 wire 생성/삭제는 아직 typed writer가 없어 read-only다.
- manual Pattern의 Stage 추가, 이동, 삭제는 Blueprint의 선택 node 도구로 옮겼다. Details에서는
  Stage duration/gap, motion, collider, reaction, Counter 등 선택 element의 수치만 튜닝한다.
  WAIT/GAP은 animation NONE인 실제 Server Stage clock으로 저장한다.
- pure graph model은 duplicate Stage/action/outcome, dangling target와 cycle을 거부하고 실패 시
  이전 snapshot을 보존한다. 네이티브 계약은 `VALTAN_DASH_CHARGE`의 default 6,050 ms,
  WALL_CONTACT 선택 11,050 ms, maximum 11,550 ms 및 determinism/hit-test를 검증한다.

### Animation source preview와 Server Valtan

- `Selected Sequence` 상세와 Preview/Replace/Append/Create 동작을 긴 virtualized 목록 위로 옮겼다.
- raw `.clipseq/.clipcuts` source는 `Layer_AnimationPreview`의 collision-free
  `isServerAuthoritative=false` Valtan clone에서만 재생한다. 버튼을
  `Preview Sequence on Arena Clone`으로 명시하고 클릭 frame에 Composition preview-owner claim도
  함께 queue해, 다른 domain deep-link 뒤 다음 update에서 preview가 즉시 정지하는 경우를 막았다.
- 실제 Arena 본체는 저장된 stable Pattern만
  `BossTool -> ValtanPatternAuditionService -> C2S PLAY_PATTERN_ID -> Server -> snapshot`으로
  재생한다. 선택 source의 `PresentationSources`를 Complete Play inventory에 역매핑하고, 한 source를
  여러 Pattern이 사용하면 explicit owner combo를 표시한다. 매핑이 없으면
  Create Pattern -> Save/Publish -> Server revision 활성화 뒤 재생하도록 안내한다. raw clip 이름을
  Server authority command로 보내거나 현재 선택 Pattern을 임의 owner로 추측하지 않는다.
- `420605/sequence 3` 지진 찍기는 돌 생성 바닥 찍기의 가장 강한 source 후보이고, 일반 그로기는
  `400430/sequence 0`, 카운터 성공 그로기는 `420631/sequence 3..5` 후보이다. 시각 검증만이면
  Effect asset 내부에 4개 rock element와 5초 지연 explosion element를 저작할 수 있다. 그러나
  개별 위치·판정·수명·despawn을 갖는 제품 돌 4개는 Effect cue가 아니라 Server combat-object
  occurrence여야 한다. 기존 timed hit/lifetime/runtime join 기반은 있으나 boss 기준 고정 4방향
  layout과 일반 combat-object Workbench 저작 adapter는 아직 후속 수직 슬라이스다.

### 이번 자동 검증

- focused Workbench/F1 Python suite: 148/148 PASS
- `ActionCompositionGraphModelContractTests`: 6/6 PASS
- `ValtanPatternAuditionServiceHarness` 전체 실행: exit 0
- 새 Client source MSVC C++20 syntax-only compile: PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- requested Pattern coverage: PASS, Product 33 / Encounter 57
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: PASS,
  57 patterns / 255 stages / 52 audition rows
- Client/Harness project와 filter XML parse: PASS

### 빌드와 사용자 화면 확인 대기

- 현재 `Client.exe` PID 43704, `Server.exe` PID 28916, Visual Studio PID 31472가 실행 중이므로
  Client link를 포함한 표준 build를 중첩하지 않았다. focused native harness만 별도 출력으로
  build/run했다.
- 사용자가 기존 Client/Server를 닫은 뒤 새 Debug build가 필요하다. 새 EXE에서 Save 성공 표시와
  재로드 뒤 8,000 ms 유지, Blueprint node/edge 선택, raw sequence clone 재생, owning saved Pattern의
  Server Valtan 재생을 각각 확인해야 하며 이 visual smoke는 아직 PASS가 아니다.

## 2026-08-31 통합 회귀 교정 및 Debug Product 빌드

### 그래프 lifecycle

- Blueprint는 live Balance generation을 다시 읽지 않고, Sequencer와 같은 immutable Pattern view의
  generation을 입력으로 받는다. 따라서 reload와 render가 같은 frame에 교차해도 stale graph가 새
  generation으로 잘못 고정되지 않는다.
- manual Pattern의 암시적 `TIMEOUT`은 vector의 즉시 다음 Stage로 연결한다. 명시적으로 저작한
  `TIMEOUT` target과 canonical event branch는 그대로 보존한다.
- Stage insert/move/remove 성공 시 preview route override를 비우고 route generation을 올린다.
  `Reset Route`를 제공하며, topology가 바뀐 frame과 rejected projection은 node/edge hit를 받지 않는다.
- edge 선택은 source branch ordinal과 Pattern/Stage/action/outcome/target stable identity를 다시 검증한다.

### source identity와 preview owner

- Create New Pattern intake가 사용자가 고른 `(sourceActionId, sourceSequenceIndex)`를 typed request와
  promotion manifest까지 보존한다. sequence `0`도 Python publisher와 Client parser가 같은 범위로
  인정한다.
- source에서 saved Pattern을 역조회할 때 exact `PRIMARY`만 owner로 인정한다. `REFERENCE`는 재생
  후보로 승격하지 않으며, canonical generation별 reverse index를 캐시해 Resources 창에서 매 frame
  전체 Pattern inventory를 복사·정렬·스캔하지 않는다.
- arena clone이 실제 preview 시작에 성공한 뒤에만 Composition viewport ownership을 요청한다.
- saved Pattern의 Server 재생은 현재 Server-active revision과 Pattern Sound runtime readiness만
  검사한다. 별개의 local draft dirty 상태는 immutable active revision 재생을 막지 않는다.
- Composition을 처음 열 때 Boss Tool canonical graph도 같은 explicit open action에서 stage/reload한다.

### 자동 검증

- focused Python 계약: 165/165 PASS
  - Workbench regression 39
  - presentation contract 45
  - manual topology 11
  - create service 19
  - create Workbench 7
  - Animation Tool master 12
  - Sound owner 14
  - sequence identity 3
  - occurrence timing 6
  - Effect invocation 9
- `ActionCompositionGraphModelContractTests`: 9/9 PASS
- `ValtanPatternAuditionServiceHarness` 전체 실행: exit 0
- `Tools/Build/test_build_profile_contract.py`: 11/11 PASS. 새 native graph source를 허용된 focused
  Client source 표면에 명시했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS
  - Engine, Shared, Server, Client compile/link PASS
  - Product Effect resource-root 8 cases PASS
  - compiled shader closure PASS, V1/V2 WARP pixels 각각 1352
  - receipt: `out/BuildPipeline/receipts/product.debug.receipt.json`
  - evidence: `out/BuildPipeline/runs/20260831T040229448Z-debug-product-a28bb869.json`

### 남은 사용자 판정

- 새 `Client/Bin/Debug/Client.exe`는 생성됐다. Save 후 reload 유지, Blueprint route 선택/초기화,
  clone source preview와 saved active revision Server Valtan 재생의 화면 결과는 사용자가 직접
  확인해야 하며 아직 visual PASS로 기록하지 않는다.

## 2026-08-31 Sequencer 본질 편집 루프 마감

### Sequence와 box 편집

- 선택한 source Sequence가 3 clips이면 Replace/Append 결과도 정확히 3개의 Animation box로
  materialize한다. Groggy의 명시적 start/loop/end HOLD chain만 기존 Server Stage보다 길 때
  start/end를 보존하고 loop 구간을 Stage clock에 맞춘다. start/end/one-shot clip을 반복으로
  추측하지 않는다.
- Animation box body drag는 Stage 내부 순서를 stable occurrence ID 기준으로 바꾼다. Animation
  오른쪽 trim은 source/play window를, Effect body drag와 `cue_end` 오른쪽 trim은 invocation
  occurrence를, Sound body drag는 point timing을 typed owner draft에 반영한다.
- 선택 box 공통 도구에 `Duplicate Selected Box`, `Delete Selected Box`를 추가했다. Animation,
  Effect, Sound 모두 새 stable occurrence ID를 사용하며, dependency 검증을 통과한 뒤에만 draft가
  바뀐다.
- Animation/Effect/Sound lane의 `+`는 선택 Pattern/Stage/Animation box의 stable ID를 보존한 채
  큰 `Composition Resources` 창을 열고 해당 domain tab으로 이동한다. Animation Sequence는
  Replace/Append, Effect와 Sound는 선택 resource를 Stage 뒤에 Add할 수 있다. 긴 catalog는
  `ImGuiListClipper`로 visible row만 그린다.

### 생성에서 다시 편집으로 이어지는 흐름

- `Use for Create New Pattern` 성공 시 `Composition Patterns > Create New Pattern` tab을 자동으로
  열고 focus한다. 이 화면에서 stable Pattern ID와 `Display name`을 편집하고 Validate 후 Apply한다.
- 생성 transaction이 성공하면 canonical reload 후 새 Pattern을 선택하고 `Patterns / Stages` tab으로
  돌아온다. 이어서 다른 source Sequence를 고르고 `Append to Stage Slots`를 누르면 기존 clip 뒤에
  정확한 box가 추가되고 Stage duration이 합산 wall clock만큼 늘어난다.

### 통합 preview와 branch clock

- Sequencer에 Play/Pause/Stop/Restart/Loop와 scrub을 두고 선택 branch의 실제 Stage path만 하나의
  clock으로 만든다. branch 변경은 이전 preview를 정지하고, `Play Selected Stage (All Slots)`는
  해당 Stage가 속한 deterministic path를 찾은 뒤 Arena Clone에서 Animation, Effect와 collider
  mirror를 함께 시작한다.
- Sound native clip duration inventory는 resolved Valtan model별로 캐시하며 Level 변경 또는 model
  소멸 때 폐기한다. Sound resource catalog render에서는 전체 animation scan을 하지 않고 Add command
  시점에 한 번 authoritative validation한다.

### 검증과 남은 실행 경계

- Action Composition focused contract: 93/93 PASS.
- Create service/Workbench까지 합친 관련 contract: 119/119 PASS.
- Client Debug x64 `ClCompile`: PASS. 현재 저장소의 기존 CP949/C4819 warning만 있고 compile error는
  없다.
- 최신 변경 대상 `git diff --check`: PASS(LF/CRLF 안내만 출력).
- Pattern/Animation/Effect draft는 `Save & Apply`, Sound draft는 별도 `Save Sound Owner`가 소유한다.
  저장 버튼을 누르기 전 timeline 변경은 실행 중 Server pattern 정본이 아니다.
- 이 절의 최종 Product link는 사용자가 실행 중인 Client가 output을 점유한 동안 수행하지 않는다.
  새 EXE에서 Create tab 자동 이동, Sequence Append, box Duplicate/Delete/drag/trim, branch preview의
  화면 결과는 사용자 visual 판정 전까지 PASS로 기록하지 않는다.

## 2026-08-31 Create Pattern Python 실행 별칭 수정

- Create Pattern의 `patternId`와 `displayName` 검증을 통과한 뒤에도 Python backend가 시작되지 않던
  원인은 WindowsApps의 정상 `python.exe` App Execution Alias를
  `std::filesystem::is_regular_file()`와 `weakly_canonical()`로 거부한 것이었다. 이 PC의 alias는
  0-byte reparse point지만 실제 `Python 3.14.4`를 실행한다.
- resolver는 `SearchPathW`가 찾은 고정 실행 경로를 `GetFileAttributesW`로 검사해 누락 경로와
  directory는 계속 거부한다. filesystem target resolution 없이 `lexically_normal()`만 적용해
  non-directory App Execution Alias를 `CreateProcessW` 대상으로 인정한다. 사용자 입력은 계속
  JSON request 안에만 전달하며 shell을 사용하지 않는다.
- Composition data-only 창은 preview `m_AssetName`이 비어 있어도 Create baseline SHA를 고정 정본
  `Data/Valtan/Valtan.presentation.debug.json`에서 읽는다. 선택 Sequence를 별도로 saved intake로
  저장할 필요가 없다.
- 같은 clip을 여러 recovered action이 공유할 때 CURRENT_CHAIN의 명시적 `(sourceActionId,
  sourceSequenceIndex)`가 primary presentation identity를 소유한다. `Valtan.animnotify` action은
  exact source tuple이 없는 기존 saved chain의 fallback으로만 사용한다.
- `patternId`는 파일명이 아니며 영문/숫자/`_-.` stable ID다. `.json`을 붙이지 않는다.
  한글 저작 이름은 별도 `Display name`에 입력한다.
- focused Workbench/Create/route 계약: 70/70 PASS. 현재 호스트에서 명시적 WindowsApps
  `python.exe --version`도 exit 0이다. 사용자가 입력한 `VALTAN_GROUND_TICK`, action 400440,
  sequence 0, `mesh_att_battle_11_01` CURRENT_CHAIN 전체 Validate dry-run도 PASS했다.
- 수정본 `Animation_Tool.cpp` Client Debug x64 compile과 `Client.exe` link는 PASS했다. 실행 중인
  Server/Data를 유지하기 위해 정본 Product 재투영은 하지 않았고 Client project만 다시 링크했다.
  새 EXE의 실제 Validate/Apply 화면 결과는 사용자 확인 전이다.

## 2026-08-31 Effect Tool V2 Group 공용 파이프라인 재실측

이 절은 앞 절의 완료 표현을 현재 source와 dirty worktree 기준으로 좁힌다. 다른 세션이 같은
worktree에서 구현 중이므로 아래 `구현 확인`은 현재 파일에 존재하는 경계이고, `미완료` 항목과
이번 dirty diff는 build/harness와 사용자 화면 판정 전까지 완료 증거가 아니다.

### 현재 source에서 확인한 구현

- Effect Tool V2가 `Data/Effects/V2/Authored/*.effectv2.json` leaf와
  `Data/Effects/V2/Groups/*.effectv2group.json` group body/ordered children을 읽고 저장한다.
  group child는 authored leaf만 참조하고 group nesting은 parser/validator가 거부한다.
- `BOSS_VALTAN.effectv2bindings.json` binding은 leaf 또는 group 하나와 stage 또는 clip clock 하나를
  참조한다. Composition Resources는 V2 leaf/group snapshot을 읽고 선택 Stage actionId와
  Stage-local `startMs`로 binding을 append한 뒤 binding 파일을 원자 교체한다.
- group child 배열을 Valtan Pattern JSON에 복사하는 경로는 없다. Product runtime은 binding의
  `groupId`를 읽어 group child를 펼치고 authored leaf를 재생한다.
- local Arena Clone의 `CValtan`은 local Pattern action clock과 immutable catalog snapshot을
  `CEffectV2Runtime::Sync_StageAuthoring`에 전달하고, Server Valtan은 replicated Server action age로
  Product `Sync_Stage`를 호출한다. renderer 구현은 공유하지만 source generation은 분리되며 local
  clone은 Server Pattern이나 Server-active revision을 변경하지 않는다.
- dirty `Animation_Tool.cpp`에는 Sound row의 exact owner/action/occurrence identity를 항상 검사하되,
  Sound wall clock에 영향을 주는 Stage/action/animation occurrence가 바뀐 경우에만 strict timing
  window를 다시 검사하는 no-new-debt 분기가 들어와 있다. 관련 Python oracle도 unchanged unrelated
  Stage와 changed Sound Stage를 구분하는 case를 추가했다.

### 닫힌 V2 Group 편집 계약

- Composition Resources는 `boss.valtan.*` V2 Group을 기본 목록에 먼저 표시하고 direct leaf는
  `Advanced: V2 Individual Leaves` 아래로 격리한다. 다른 owner의 leaf/group은 BOSS_VALTAN
  binding에 새로 추가할 수 없다.
- Effect Tool V2가 leaf body와 ordered group children을 소유하고 Composition은
  `BOSS_VALTAN.effectv2bindings.json`의 `groupId`, exact Stage action, Stage-local `startMs`와 placement만
  소유한다. Pattern JSON에 group children을 펼쳐 복사하지 않는다.
- append/remove/duplicate/update-start는 persisted binding 전체 행을 stable baseline으로 사용하는
  typed mutation이다. non-default bone/follow/rotation/offset/yaw row도 ordinal 없이 정확히 선택하며
  stale·ambiguous match는 파일과 snapshot을 바꾸기 전에 거부한다.
- C++ catalog와 Python validator는 Group이 펼치는 child leaf와 direct leaf가 같은 Stage/clip clock에
  중복 배치되는 새 binding을 거부한다. documents/groups/bindings 전체 read-set의 normalized CAS가
  성공한 뒤에만 binding 파일을 atomic replace한다.
- Sequencer는 V2 Group을 최소 240 px, V2 Leaf를 최소 180 px의 식별 가능한 block으로 그린다.
  이 폭과 sub-row packing은 눈 검증용 projection이며 Stage duration이나 Effect lifetime을 변경하지
  않는다.
- local Arena Clone은 `CEffectV2Catalog`의 immutable authoring snapshot을 명시적으로 받아 Stage를
  restage한다. Effect Tool V2 Save는 Product 전역 runtime cache를 invalidate하지 않는다. 따라서
  저장 직후 local preview와 입장 시 고정된 Product presentation generation을 섞지 않는다.

### local preview와 Complete Play의 실제 차단 근거

추가 실행 진단에서는 world-entry manifest가
`BOSS_VALTAN.effectv2bindings.json`의 기존 2,503 bytes / `ac619...` generation을 고정한 상태에서
실제 authoring 파일이 3,201 bytes / `6b3635...`로 바뀌어 Complete Play admission이 막혔다.
이는 정상 fail-close 증거다. `Reload Complete Play Inventory`는 resource catalog/inventory view를
reload할 뿐 이미 입장한 world의 immutable Server presentation generation을 갱신하지 않는다.

따라서 두 경로를 다음처럼 분리한다.

```text
Arena Clone authoring preview
  Effect V2 immutable authoring snapshot + current path/playhead restage
  -> 저장 직후 local 확인

saved Product Complete Play
  Effect V2 source validation/publish/build
  -> Server restart
  -> arena world re-entry로 새 presentation manifest admission
  -> exact Server-active gameplay revision과 함께 Complete Play
```

catalog reload만 반복하거나 local clone에서 보인 결과를 Server Product 적용 완료로 기록하지 않는다.

### 이번 변경의 자동 검증 상태

- Effect V2/Composition focused tests: 89/89 PASS.
- presentation contract tests: 45/45 PASS.
- Effect V2 actual validator: 81 authored / 80 bindings / 1 group / 45 textures PASS.
- 변경 핵심 C++ 4파일 `/Zs` syntax compile: PASS(기존 C4819 warning만 존재).
- Client Debug x64 `ClCompile`: PASS, error 0 / warning 656. warning은 기존 CP949/C4819 항목이며 이번
  변경의 compile error는 없다.
- `git diff --check`: PASS(LF/CRLF 안내만 출력).
- Product link/Core와 사용자 Arena Clone/Complete Play 화면 검증은 이번 compile-error-free 종료
  범위에 포함하지 않았다. visual PASS와 Server-active 적용 완료는 사용자의 새 EXE 검증 전까지
  기록하지 않는다.

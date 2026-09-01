# 발탄 Restart Flow·Effect Anchor·Sequencer 구현 계획서

> **2026-08-31 감사 교정:** 이 문서의 이전 `완료`, `PASS`, `typed owner Save` 표현은
> 자동 완료 판정이 아니다. 특히 `Valtan.patternbindings.json`과
> `Valtan.patterneffectcues.json`은 `Valtan.presentation.json`에서 projector가 만드는
> read-only Product이므로 직접 저장 대상으로 취급한 설계는 폐기한다. 이후 완료 판정은
> `authoring owner 저장 → projection 뒤 동일 의미 유지 → actual canonical loader admission →
> 같은 revision의 표준 EXE/Data → 실패 시 byte-identical rollback → 사용자 수동 화면 확인`
> 순서로만 한다. 새 패턴 확장은 Gate 0~6이 닫힐 때까지 중단한다.

## 목표

F1 도구에서 저장한 발탄 Flow를 Product 정본으로 Publish하고, 재생 중인 Pattern/Flow를 명시적으로
처음부터 다시 시작하며, 벽·바닥 복구는 별도 `Refresh Arena` Server 명령으로 확인할 수 있게 한다.
`Next Pattern`은 현재 occurrence를 끝까지 실행한 뒤 한 칸 pending 예약을 승격하는 기존 Server 계약을
유지한다.

동시에 `VALTAN_SIX_PIZZA_106`의 sector Effect가 고정 arena center만 보지 않고 Server가 선택해 snapshot으로
복제한 random alive player를 실제 anchor로 사용할 수 있게 한다. Anim Workbench Sequencer는 새 범용
`sequence.json` 런타임을 만들지 않고 Gameplay, Animation, Effect, Sound, Camera, Light, World의 실제 owner
row를 stable ID로 조합하고 각 owner의 typed Save를 호출한다.

F1의 `LostArk Developer Tools` 루트는 도구 선택용 소형 창으로 유지하고, Workbench의 긴 Detail/Sequencer를
루트 창 크기에 합성하지 않는다. 루트 창은 기본 크기와 내부 scroll을 가지며 사용자가 가장자리로 resize한다.
Tools 바로 아래의 `Level Navigation`은 `Lobby`, `Character Select`, `Bern`, `Valtan`, `KoukuSaydon` 이동 의도를
보이되 `Change_Level`이나 socket을 직접 호출하지 않는다. 앞의 네 경로는 기존 Lobby command와
`CLevelTransitionService`를, KoukuSaydon은 Character Select의 기존 Server debug-admission command를 사용한다.

루트 `Resource Files`는 3만 개가 넘는 물리 파일을 기본 authoring UI로 펼치지 않는다. 선택 Pattern이 실제로
소비하는 Gameplay, Animation, Effect, Sound, Camera/World 정본 관계는 Anim Workbench의 `Used by Selected
Pattern`에서 먼저 보이고, 원시 전체 index는 검색 가능한 bounded diagnostics로만 남긴다.

Anim Workbench의 최종 저작 시작점은 기존 Pattern 편집만이 아니다. 작업자가 실제 Valtan WModel의 clip 또는
저장된 Animation Sequence Intake chain을 선택해 눈으로 재생한 뒤 `Create New Pattern`을 누르면, 그 chain을
새 audition-only Pattern의 semantic stage와 animation slot으로 승격하고 같은 화면의 Persistent Detail에서
공백, Counter/Groggy, motion, 회전, collider와 지원되는 특수 로직을 계속 튜닝할 수 있어야 한다.

## 2026-08-30 현재 실측

### 23:24 실제 EXE 회귀 역추적

사용자 실행 화면은 이전 `Reload Flow`/`Save Flow` 라벨을 가진 23:07 Debug Client였고, 물리 Data에는
새 WARP motion object가 먼저 반영되어 있었다. Product build는 실행 중인 Server 출력의 `LNK1104`에서
중단되어 Client link까지 도달하지 못했다. 따라서 old Client의 strict parser가 새
`{ kind, retargetDelayMs, speedMps, distanceM }` motion을 거부했고, 공통 canonical graph admission 실패가
Pattern inventory, Complete Play, Flow slots와 Next 선택을 한꺼번에 차단했다.

또한 Action Presentation Workbench는 `Resolve_Model()==nullptr`에서 즉시 return하여, preview model 하나가
없다는 이유로 model과 독립적인 Pattern Outliner, joined Detail, Sequencer, Data Files까지 전부 숨겼다.
Valtan Arena 진입 시 dedicated Valtan preview를 자동 선택하는 open route도 없었다.

이번 변경은 다음 순서를 강제한다.

1. 표준 Client/Server EXE가 실행 중이면 어떤 publisher/build domain보다 먼저 exact PID/path로 build를 거부한다.
2. schema-changing source는 temp fixture에서 current parser/publisher를 먼저 검증하고 새 Client link가 끝난 뒤
   canonical Data를 atomic promote한다.
3. Workbench는 Valtan Arena에서 dedicated Valtan target을 자동 선택하되, model 준비 실패 시에도 data-only
   Pattern/Detail/Sequencer를 유지하고 preview/edit 버튼만 격리한다.
4. 완료 판정은 build command 호출이 아니라 새 EXE timestamp/receipt, strict graph load, full pattern inventory,
   Complete Play selectable state까지 확인한 뒤 사용자 수동 화면 검증으로 넘긴다.

| 요구 | 현재 실제 상태 | 이번 변경 방향 |
|---|---|---|
| Save Flow → Publish | `Save_FlowDocument`가 atomic 저장 뒤 `Publish_SavedPatternFlow`와 HOT_RELOAD 적용을 자동 제출 | 버튼과 상태를 `Save + Publish` 의미로 명확히 하고 exact Server-active revision 전에는 Restart를 막음 |
| Reload Flow | disk reload와 `Start_Flow(false)`가 한 함수에 섞임 | disk-only `Reload Saved Flow`와 `Restart Saved Flow`를 분리 |
| Restart Saved Flow | 첫 배열 slot부터 시작하며 Server가 boss, destruction, prop, collision, Nav, combat object를 full reset | `Restart Saved Flow (Fresh Arena)`로 사실을 표시하고 exact verdict를 노출 |
| Restart Pattern | `Complete Play`는 boss-only reset 후 선택 Pattern을 재생하지만 active same-owner occurrence 중 새 요청을 Client/Server가 차단 | same-owner restart를 명시적으로 허용하고 다른 owner/Flow/Next는 보존·거부 |
| Refresh Arena | F1 공용 preset에 `FRESH`가 이미 있음 | Boss Pattern/Flow 작업 위치에도 `Refresh Arena`를 노출하되 재생과 합성하지 않고 verdict를 따로 기다림 |
| Next Pattern | occurrence epoch/sequence CAS 한 칸 예약과 resetless terminal 승격 구현 완료 | 이름, pending 상태, Flow 잔여 순서 대체 의미를 명확히 하고 회귀 고정 |
| 중앙 재생 느낌 | reset이 `boss.valtan.center` placement와 고정 audition bait를 사용 | Debug 시작 자세를 명시적으로 표시하고, Pattern 자체의 target/anchor/motion을 실제 데이터로 확장; 좌표를 Client 요청으로 보내지 않음 |
| 피자 Effect | `VALTAN_SIX_PIZZA_106`은 random alive target/facing을 Server가 잠그지만 cue 위치는 `arena.center.facing` | replicated pattern target entity를 cue anchor resolver까지 전달하고 `PATTERN_TARGET_SNAPSHOT` 정책 추가 |
| Sequencer 저장 | Workbench는 실제 owner lane join이며 generic sequence document/consumer 없음 | timing/asset/anchor를 복제하는 가짜 JSON은 만들지 않고 typed Effect cue Save adapter를 추가 |
| Create New Pattern | `Animation Sequence Intake` 저장과 `promote_valtan_animation_chains.py`의 offline 승격은 존재하지만 UI에서 연결되지 않음 | 선택 chain + stable Pattern metadata를 하나의 atomic promotion request로 검증·commit하고 Product candidate/apply까지 연결 |
| 특수 로직 Detail | stage duration, animation slot, Counter/Groggy, collider, grabbed release와 일부 cue yaw는 typed 편집 가능 | 임의 key/value나 C++ 스크립트를 저장하지 않고 실제 Server enum/template만 Add Row로 노출 |
| F1 루트 크기 | `AlwaysAutoResize` 때문에 펼친 Resource/diagnostic 내용 전체만큼 커지고 수동 resize가 무효화됨 | 기본 `720x760`, viewport bounds, 내부 scroll, 새 saved-layout ID와 일반 resizable window로 복구 |
| Level 이동 | Tools와 Resource Files 사이에 이동 UI가 없고 KoukuSaydon은 Character Select 내부의 기존 Server 경로를 알아야만 진입 가능 | `Level Navigation`을 추가하되 typed command/Server admission만 사용하고 current/pending/reject 이유를 같은 패널에 표시 |
| Resource Files | 31,625개 raw 파일과 UI/Fonts 수천 행이 의미 있는 저작 데이터보다 먼저 화면을 점유 | canonical authoring source summary를 기본 노출하고 raw index는 기본 닫힘·검색·clipper·고정 높이 diagnostics로 격리 |
| Anim 기본 화면 | 기존 Target/Clip/Skill Timing 화면이 첫 창 전체를 점유해 Pattern Detail이 아래 fold 밖에 존재 | `Pattern Workbench`를 기본 첫 탭으로, `Animation Clips / Sequence Intake`를 별도 탭으로 분리하고 Valtan Target selector는 기본 접힘 |

### 기능 이름이 아니라 실제 실행 결과로 다시 세운 순서

이 작업은 패널, 버튼, 함수 또는 테스트 문자열이 존재한다는 이유로 완료 처리하지 않는다. 아래 Gate를
위에서부터 순서대로 닫는다. 앞 Gate가 실패하면 뒤 기능은 화면에 있어도 사용할 수 없는 상태로 기록한다.

| Gate | 사용자에게 보였던 실제 문제 | 현재 상태 | 완료 증거 |
|---|---|---|---|
| 0. 배포 정본 | 실행 중인 23:07 Client/Server 때문에 링크가 실패했지만 새 Data는 먼저 투영되어 old parser/new schema가 충돌 | 출력 잠금 preflight와 격리 빌드 추가. 표준 Debug 교체는 실행 프로세스 종료 전까지 미완료 | publisher 실행 전 exact PID/path 거부, 새 EXE timestamp/hash, 같은 revision Data receipt |
| 1. F1 창 | `AlwaysAutoResize`가 모든 펼친 내용을 한 창 크기로 만들고 edge resize를 무효화 | 일반 resizable window, 작은 default, viewport bound, 내부 scroll 코드/계약 테스트 반영. 사용자 화면 확인 전 미완료 | 사용자가 F1 창을 줄이고 늘린 관찰, 내용 overflow scroll |
| 2. Anim Workbench 창 | 1180x760이 최소 크기여서 줄일 수 없고 저장된 큰 layout을 계속 재사용 | 최소 320x200, 새 hidden layout ID, table clip이 하단 pane을 조기 종료하지 않도록 수정 중 | 최신 Client compile/link + 사용자 resize/scroll 확인 |
| 3. Preview capability | Valtan Arena에서 target을 만들지 않고 `Resolve_Model()==nullptr`이면 창 전체 return | exact `Valtan` Model View 1회 staging, 실패 시 data-only shell 유지 코드/32개 계약 테스트 반영 | preview READY/MISSING 상태 분리, model 없이 Outliner/Detail/Sequencer/Data Files 표시 |
| 4. Canonical join | WARP motion schema를 old Client가 거부해 master graph 전체가 사라짐 | current parser/publisher validator와 격리 Client compile/link는 PASS. 표준 EXE 적용은 미완료 | `Canonical Join: ADMITTED`, full pattern count, exact source revision |
| 5. Pattern 재생 UI | inventory가 없어서 Pattern 선택·Complete Play·Flow·Next가 모두 비활성 | Server/Client 계약 구현과 focused harness는 PASS, Gate 0/4 적용 전 사용자 화면은 미완료 | 실제 EXE에서 pattern 선택, Complete Play verdict, Flow slot 목록 |
| 6. Restart/Flow | Reload/Save/재생/벽 복구 의미가 섞여 사용자가 무엇이 reset되는지 알 수 없음 | Reload Saved Flow, Save+Publish, Refresh Arena, Restart Pattern, Restart Saved Flow, Next pending을 분리 구현 | Server occurrence와 arena/nav revision 보존·reset harness + 사용자 실행 |
| 7. Create New Pattern | Intake와 offline promotion script는 있으나 Workbench 버튼에서 Product transaction으로 이어지지 않음 | strict request/CAS/rollback backend와 temp fixture 10개 PASS, 실제 UI 연결 진행 중 | Workbench Validate/Apply → master/Boss inventory reload, 실제 임시 생성/재시작 확인 |
| 8. Persistent Detail | 일부 typed field는 있으나 model/canonical 실패가 패널 전체를 숨겨 실제 사용 불가 | data-only shell과 model capability guard 반영. field coverage 전수 대조 미완료 | 각 field의 source path, saved value, runtime consumer를 한 row에서 확인/저장 |
| 9. Effect/Sequencer | joined lane 코드는 있으나 사용자가 못 보고 anchor picker와 owner별 Save 범위도 불완전 | target snapshot/yaw와 실제 owner lane 일부 반영, typed picker/남은 owner adapter 미완료 | stage 선택 시 Animation/Effect/Sound/Camera/World rows와 owner deep-link/Save |
| 10. 패턴별 완성 | 여러 요구가 동시에 나열됐지만 각각의 Server/runtime/presentation closure가 다름 | WARP 이동 수직 슬라이스와 일부 Restart/피자 기반만 자동 검증. 나머지는 미완료 | 패턴별 JSON→Server→snapshot→Client presentation→Restart 수동 확인 |
| 11. 의미 기반 Resource/Level | raw filesystem dump가 authoring source처럼 보이고 Level 이동은 여러 화면에 흩어짐 | F1 semantic source summary·bounded raw diagnostics와 typed Level Navigation 구현 중 | 선택 Pattern의 exact source/deep-link와 각 Level의 Server-approved transition verdict |

Gate 0~4가 모두 닫히기 전에는 F1 화면에 버튼이 보인다는 사실을 기능 완료 증거로 사용하지 않는다.
Gate 5 이후도 각 명령의 Server verdict와 정본 revision을 확인하지 못하면 `UI ONLY` 또는 `NOT VERIFIED`로
남긴다.

## 구현 종료 판정

- `Reload Saved Flow`는 disk draft만 교체하고 어떤 Server 재생도 시작하지 않는다.
- `Save + Publish Flow`는 저장 SHA와 publisher/apply 상태를 구분하며, `COMMITTED` 또는
  `ALREADY_ACTIVE`로 exact revision이 확인되기 전 `Restart Saved Flow`가 비활성화된다.
- `Restart Pattern`은 같은 사용자가 소유한 active Pattern을 boss-only reset으로 교체할 수 있고 현재
  wall/floor/debris/collision/Nav revision을 보존한다.
- `Refresh Arena`는 FRESH preset 결과를 받은 뒤 모든 wall/floor active state를 Server snapshot으로 확인할
  수 있다. Refresh 성공을 Pattern/Flow 시작 성공으로 합성하지 않는다.
- `Restart Saved Flow (Fresh Arena)`는 저장 배열의 실제 첫 원소부터 시작하고 arena 전체 reset을 수행한다.
- `Next Pattern`은 현재 Pattern 종료 뒤 reset 없이 한 번만 승격하고 Flow 중이면 저장 문서는 바꾸지 않은 채
  현재 occurrence 뒤 남은 Flow만 대체한다.
- six-pizza cue는 Server snapshot의 exact pattern target entity pose를 사용한다. target이 사라지거나 resolve에
  실패하면 arena center로 조용히 fallback하지 않고 해당 cue occurrence만 격리한다.
- Workbench에서 Effect anchor row를 선택·수정·저장할 때 `Valtan.patterneffectcues.json`의 exact stable row가
  parse → validate → stage → CAS commit되며 Product runtime reload 실패 시 기존 cache를 유지한다.
- `Create New Pattern`은 저장된 Intake chain과 실제 WModel clip duration을 다시 검증해 gameplay,
  presentation, animation binding, `manualAuditions` owner를 전부 만들거나 아무것도 바꾸지 않는다.
- 새 Pattern은 기본적으로 `AUDITION_ONLY`이며 명시적 회전 승격 전에는 일반 전투 selector에 들어가지 않는다.
- F1 루트 창은 진입 시 화면 대부분을 덮지 않고, 테두리/모서리 drag로 줄이거나 늘릴 수 있다.
- Pattern Detail의 `Add Logic Row`는 Server가 실제 소비하는 typed template만 제공한다. 알 수 없는 kind, 지원되지
  않는 필드 조합, 중복 stable ID 또는 부분 생성은 거부하고 이전 Pattern/Product를 보존한다.
- Debug/Release Product, Core, 변경 domain validator와 focused native/Python harness가 통과한다.
- Client 화면, Effect 회전, Sound와 체감은 사용자가 직접 판정하기 전 manual PASS로 기록하지 않는다.

## G00. 실패를 먼저 고정하는 계약 테스트

수정 대상은 다음과 같다.

```text
Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
Tools/ValtanPipeline/test_valtan_boss_tool_contract.py
Tools/ValtanPipeline/test_action_presentation_workbench_contract.py
Client/Private/ValtanPatternAuditionServiceTests.cpp
Client/Private/ValtanPatternFlowServiceTests.cpp
Server/Private/ServerGameplayContractTests.cpp
```

먼저 다음 실패를 assertion으로 만든다.

- Reload 함수가 `Start_Flow` 또는 network send를 호출하면 실패
- Save보다 Publish가 먼저 호출되거나 저장 실패 뒤 Publish하면 실패
- 같은 owner active Pattern restart를 거부하면 실패
- 다른 owner, active Flow, pending Next를 Pattern restart가 교체하면 실패
- Pattern restart가 destruction/prop/collision/Nav revision을 바꾸면 실패
- Flow restart가 첫 배열 원소가 아닌 slot ID 숫자 정렬 결과를 사용하면 실패
- Next가 현재 pose, HP, arena state를 reset하거나 두 번 승격하면 실패
- player-target cue가 stale/missing target을 arena center로 fallback하면 실패
- F1 Resource Files가 검색 없이 수천 raw row를 기본 펼치거나 clipper 없는 무제한 child를 렌더하면 실패
- F1 Level Navigation이 `Change_Level`, `CNetworkManager` 또는 새 Kakul Lobby stage를 직접 사용하면 실패

## G00-01. F1 Level Navigation과 의미 기반 Resource 표면

`RenderDeveloperTools`의 순서는 `Tools → Level Navigation → Authoring Sources → Raw Index Diagnostics`다.
Level 버튼은 current/pending 상태에서 비활성화하고 결과를 같은 패널의 status에 보존한다. Lobby,
Character Select, Bern, Valtan은 현재 Level이 Lobby인지 여부에 따라 기존 `CLobbyCommandService`를 직접
staging하거나 token을 가진 Lobby load를 staging한다. KoukuSaydon은 `LOBBY_STAGE`를 확장하지 않고
Character Select의 `IWorldEntityCommandSink::Request_EnterKakulSaydonArena` typed wrapper만 호출한다.

`Authoring Sources`는 다음 도메인의 canonical source count/status와 owner 도구 이동만 표시한다.

```text
Boss / Pattern → Valtan gameplay, presentation, bindings, Flow
Character / Animation → authored binding과 saved Intake
Effect → EffectCatalog와 exact authored documents
Sound → pattern sound cue와 실제 Resources-relative variants
Gameplay / Rendering → typed Balance/World/Rendering JSON owners
```

3만 개 물리 파일 전체는 `Raw Index Diagnostics` 아래 기본 닫힘 상태로 둔다. 사용자가 search를 입력한 뒤에만
고정 높이 child와 `ImGuiListClipper`로 일치 행을 렌더한다. raw row 선택은 복사/publish를 하지 않으며, 실제
owner action이 없으면 read-only 이유를 표시한다.

## G01. Boss Tool의 명령 의미 분리

### 수정 파일

```text
Client/Public/BossTool.h
Client/Private/BossTool.cpp
.md/TEAM/보스툴.md
.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md
```

`Reload_FlowDocument`는 disk-only parse → validate → stage → commit만 수행한다. 기존 함수 안의
`Start_Flow(false)`를 제거한다. UI 명령은 다음처럼 분리한다.

```text
Reload Saved Flow
Save + Publish Flow
Restart Saved Flow (Fresh Arena)
Start Here (Fresh Arena)
Refresh Arena
Restart Pattern (Preserve Arena)
Next Pattern...
```

각 버튼 바로 아래에 Tool local 상태, publisher/apply 상태, Server playback 상태를 별도 줄로 표시한다.
`SAVED`, `PUBLISHING`, `APPLY_PENDING`, `COMMITTED`, `ALREADY_ACTIVE`, `FAILED`를 같은 성공 문구로 합치지
않는다.

## G02. same-owner Pattern Restart

### 수정 파일

```text
Client/Public/ValtanPatternAuditionService.h
Client/Private/ValtanPatternAuditionService.cpp
Server/Private/GameRoom.cpp
Server/Private/ServerGameplayContractTests.cpp
```

`CValtanPatternAuditionService::Restart`는 현재 snapshot이 같은 consumer와 boss placement를 소유할 때만 새
request sequence를 만든다. pending verdict, Next, Flow, 다른 consumer는 교체하지 않는다.

Server는 새 request를 검증하고 canonical boss replacement를 완전히 stage한 뒤에만 기존 same-session
stable-ID occurrence를 ABORTED 처리한다. commit 뒤 boss-owned attachment/combat object와 Pattern state만
초기화하고 destruction, prop, collision, navigation state와 revision은 유지한다. 실패 시 기존 occurrence와
arena를 그대로 둔다.

## G03. Refresh와 Flow Restart

`Refresh Arena`는 기존 `VALTAN_ARENA_PRESET::FRESH` typed transaction을 그대로 사용한다. Pattern/Flow 재생을
자동 제출하지 않으므로 timeout이나 거절 뒤 재생 성공처럼 보이지 않는다.

`Restart Saved Flow`와 `Start Here`는 현재 Product 계약대로 full arena reset을 수행한다. 첫 pattern 전에
boss/player를 canonical audition start에 배치하고 저장 Flow의 exact SHA를 pinned Server catalog와 비교한다.
현재 Flow가 같은 owner에게 active여도 새 request가 완전히 stage된 뒤 old lifecycle을 ABORTED 처리하고 새
epoch를 commit한다.

중앙에서 샘플처럼 보이는 문제는 버튼 이름으로 숨기지 않는다. Workbench Detail에 다음 read-only 시작 문맥을
표시한다.

```text
Replay reset: BOSS_ONLY 또는 FRESH_ARENA
Boss start: boss.valtan.center
Driver bait: 현재 Server audition bait policy
Pattern target: NONE / NEAREST / RANDOM_ALIVE / RANDOM_BEHIND / locked entity
Pattern motion anchor: stable anchor ID와 move-before-takeoff flag
```

향후 start pose를 조정할 때도 좌표를 Client packet에 넣지 않는다. stable start-anchor ID를 gameplay catalog가
소유하고 Server가 pinned revision에서 resolve하는 방식만 허용한다.

## G04. six-pizza의 Server-selected player Effect anchor

### 데이터와 런타임 흐름

```text
Server CValtanBrain LOCK_RANDOM_ALIVE_ON_START
→ boss snapshot patternTargetNetEntityId + locked facing
→ ClientReplication exact replicated entity lookup
→ CValtan pattern target pose snapshot
→ Valtan Pattern Effect cue anchor policy PATTERN_TARGET_SNAPSHOT
→ CEffectPlayback spawn pose
```

`PATTERN_TARGET_SNAPSHOT`은 cue spawn 시점의 exact replicated target position과 facing을 고정한다. 후속
`PATTERN_TARGET_FOLLOW`가 실제 요구될 때만 별도 정책으로 추가한다. target ID가 없거나 entity generation이
맞지 않으면 cue만 실패하고 Client가 random player를 다시 뽑지 않는다.

이번 slice의 six-pizza row는 world center와 target snapshot 중 선택할 수 있고 local Y yaw를 더해 sector를
회전한다. V1 element `groupId`는 다른 작업자의 group 저작 단위이며 anchor pivot으로 재해석하지 않는다.

## G05. Product Effect cue typed authoring

### 수정 후보

```text
Client/Public/ValtanPatternEffectCueDocument.h
Client/Private/ValtanPatternEffectCueDocument.cpp
Client/Public/Animation_Tool.h
Client/Private/Animation_Tool.cpp
Client/Public/Effect_Tool.h
Client/Private/Effect_Tool.cpp
Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1
```

현재 load-only cue document에 exact `cueId/occurrenceId/patternId/stageId/actionId`를 기준으로 Add/Edit/Remove
draft와 atomic Save를 추가한다. anchor policy, anchor slot, local position/rotation/scale, start offset,
duration/stop/repeat만 Effect invocation owner가 편집한다. Effect asset 내부 element/module/material/resource는
기존 Effect Tool owner가 계속 담당한다.

Save는 Encounter, Animation binding, Effect asset inventory와 source bytes를 snapshot하고 commit 직전에 CAS를
재검사한다. 새 row가 stage wall이나 linked clip wall을 벗어나거나 target policy가 해당 Pattern의 target
계약과 맞지 않으면 기존 source와 active runtime cache를 보존한다.

## G06. Anim Workbench Sequencer의 저장 계약

새 generic `sequence.json`에 Animation, Effect, Sound, Camera, Light, World 값을 복제하지 않는다. 현재 실제
Product consumer가 없기 때문에 그런 파일은 저장돼도 게임에서 읽히지 않는 두 번째 정본이 된다.

Workbench Sequencer는 다음 owner adapter를 같은 시간축에 배치한다.

| Lane | 저장 owner | 편집 상태 |
|---|---|---|
| Gameplay | `Valtan.gameplay.json` | stage/motion/counter/groggy typed Save |
| Animation | `Valtan.patternbindings.json` | slot Add/Edit/Remove typed Save |
| Effect invocation | `Valtan.patterneffectcues.json` | G05에서 Add/Edit/Remove typed Save |
| Effect content | `Data/Effects/Authored/*.effect.json` | Effect Tool deep-link와 기존 Save |
| Pattern Sound | `Valtan.patternsoundcues.json` | Add/Edit/Remove typed Save |
| Camera/Shake | 실제 camera/shake owner | 검증된 adapter가 있는 field만 edit, 나머지는 deep-link |
| Light/World/Combat Object | 실제 Product owner | read-only join 후 owner 도구로 이동 |

필요하면 `sequence workspace`는 선택 lane, zoom, playhead, 접힌 row 같은 UI session/bookmark만 저장한다.
Product timing, asset ID, anchor 또는 gameplay 값은 담지 않는다.

## G07. Animation Sequence Intake에서 Create New Pattern

현재 `Data/Valtan/Valtan.presentation.debug.json`은 Workbench가 저장하는 review chain이고,
`Data/Valtan/Valtan.animation-chain-promotions.json`과
`Tools/ValtanPipeline/promote_valtan_animation_chains.py`는 검토된 chain을 split gameplay/presentation의
`MANUAL_SERVER_AUDITION` Pattern으로 만드는 기존 검증 경로다. 새 UI는 이 두 경로를 복제하지 않고 다음
transaction으로 연결한다.

```text
실제 Valtan WModel clip / 저장 Intake chain 선택과 Preview
→ Create New Pattern
→ stable patternId + 한글 displayName + authoringPhase + target/aim policy 입력
→ source chain과 promotion row stage
→ 실제 model clip/duration + ID/owner/duplicate 검증
→ gameplay stages + presentation occurrences + manualAuditions 생성
→ Valtan split/Product 전체 projection stage
→ source CAS 확인
→ all-or-nothing commit
→ candidate publish/apply
→ joined Workbench와 Boss Tool inventory reload
```

기본 생성 규칙은 sequence occurrence 하나당 semantic `STEP_01..NN` 하나다. 각 stage는 우선 hit `NONE`,
motion `NONE`, sequential default edge를 가지며 실제 clip 길이 또는 작업자가 입력한 play wall을 duration으로
사용한다. 특별한 판정이나 이동을 추측해서 자동 생성하지 않는다. 생성 직후 선택된 Pattern Detail에서 다음
typed template을 추가·수정한다.

| Detail category | typed authoring unit |
|---|---|
| Timeline | stage duration, explicit blank/NONE stage, animation slot sourceStart/playMs/playRate/loop |
| Counter/Groggy | counterable enter/exit flag + `COUNTER_HIT` branch + same-pattern GROGGY stage/flag |
| Motion | supported motion kind, start delay, speed, distance, stable target/center anchor, yaw offset |
| Collider | circle/ring/cone/box/cross geometry, hit schedule, existing DamageProfile reference |
| Capture/Release | supported player response, attachment slot, release mode/speed/duration/yaw |
| Presentation | Effect/Sound/Camera typed occurrence link and exact owner deep-link |

`Custom Logic`은 raw JSON이나 임의 코드가 아니다. `Add Logic Row`가 제공하는 목록은 현재 Server parser와
fixed-tick consumer가 존재하는 action/branch template에서 자동 구성한다. 새로운 종류는 Data schema, Server
consumer, Client presentation, validator와 rollback harness가 함께 추가된 뒤에만 목록에 나타난다.

Create transaction은 기존 `promote_valtan_animation_chains.py`의 검증과 atomic commit을 재사용하되, 현재
manifest 전체 hash를 작업자가 직접 고치게 하지 않는다. Tool은 baseline bytes/SHA를 잡고 request를 stage하며,
commit 직전 source/manifest/gameplay/presentation이 모두 같은 baseline인지 CAS로 확인한다. 실패하면 debug
chain, manifest, split source, Product, receipt 중 어느 것도 부분 교체하지 않는다.

## G08. 다음 패턴 수직 슬라이스

Restart/Effect anchor가 닫힌 뒤 다음 순서로 구현한다.

1. `VALTAN_WARP`: stage별 0.5초 retarget delay, rush speed, distance, start/end portal anchor를 gameplay typed
   owner와 Server motion에서 소비하고 Workbench Detail에서 조정한다.
2. `VALTAN_STRUGGLING`: jump-to-center stable anchor, move-before-takeoff flag와 landing validation을 추가한다.
3. reference-only `VALTAN_TRIPLE_COUNTER`, `VALTAN_MAGIC_ORB_STAGGER_76`를 authoring master로 승격한 뒤
   Counter/Groggy와 stagger gauge Detail adapter를 연결한다.
4. stone spawn/destroy와 silence는 각각 Server combat object lifecycle, replicated debuff/input/HUD vertical
   slice가 필요하므로 데이터 row만 먼저 만들지 않는다.

이미 완료된 `VALTAN_HIGH_JUMP` 공백, Trash Counter/Groggy와 Pattern Sound, Catch Breath release yaw는 다시
구현하지 않고 회귀한다.

## G09. 자동 검증

```powershell
python -m unittest discover -s Tools/ValtanPipeline -p 'test_valtan_boss_tool*.py'
python -m unittest Tools.ValtanPipeline.test_action_presentation_workbench_contract
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Core
git diff --check
```

Effect anchor는 no-target, stale target, target despawn, exact target snapshot, local yaw와 cue Save CAS rollback을
focused native harness로 검증한다. Restart는 same-owner replacement, cross-owner reject, Flow conflict,
destruction/prop/collision/Nav preservation, first-slot restart와 duplicate request idempotency를 검증한다.

## G10. 사용자 수동 검증 가이드 산출물

RESULT에는 실제 F1 클릭 경로를 다음 작업 단위로 분리해 기록한다.

```text
Flow 편집 → Save + Publish → exact Server-active 확인 → Restart Saved Flow
Refresh Arena → Server FRESH 확인 → Restart Pattern
현재 Pattern 중 Restart Pattern → 첫 stage부터 재생 확인
현재 Pattern 중 Next Pattern 예약 → 현재 terminal 뒤 한 번 승격 확인
six-pizza → target player 이동 전/후 anchor와 sector yaw 확인
Effect cue Detail 수정 → Save → runtime reload → 같은 Pattern Restart 비교
```

에이전트는 Client/UI를 실행·조작하거나 시각·음향 PASS를 대신 기록하지 않는다.

## 2026-09-01 확정 변경: audition Flow와 Logic Flow 가시화

이 절은 위의 `Save + Publish` 및 active Flow 중 문서 편집 잠금보다 나중에 사용자가 확정한 계약이다.
Boss Tool의 저장 Flow는 Product 자동 전투 `scriptedSequence`를 암묵적으로 바꾸는 버튼이 아니라,
전체 ordered slot과 source revision을 `FLOW_START`에 복사해 제출하는 Debug audition 정본이다.
따라서 Pattern/Stage/Animation/Effect Product parity는 계속 fail-closed로 검증하되, audition Flow의
순서와 pursuit를 stale Product 배열과 비교해 canonical graph 전체를 폐기하지 않는다.

- `Boss Verification`의 Pattern 목록을 `All Patterns`와 `Current Patterns`로 나눈다.
- `Current Patterns`는 unsaved draft가 아니라 마지막 Save 성공으로 commit된 document baseline을
  순서와 중복까지 그대로 표시한다. Save 성공 프레임부터 최신 baseline을 표시한다.
- active run은 제출 당시 slot 사본을 유지한다. 그동안 다음 draft를 Add/Up/Down/Discard Selected/Save할
  수 있고, `Restart Flow`가 저장 파일을 다시 읽은 뒤 새 첫 slot부터 replacement request를 제출한다.
  Server verdict 전에는 이전 run이 계속 authoritative다.
- `Discard Selected`는 선택한 Flow slot 하나만 제거한다. 전체 unsaved draft 복원 명령과 합성하지 않는다.
- 세 번째 `Logic Flow` 탭은 1차 범위에서 읽기 전용이다. 상단에 현재/선택 Pattern을 표시하고,
  기존 `CActionCompositionGraphModel` projection에 Stage role, 실제 clip 이름, `COUNTER_HIT`, `TIMEOUT`,
  counterable flag/CounterProxy를 주석으로 합쳐 코드에 이미 연결된 흐름을 시각화한다.
- Logic Flow는 새로운 JSON, runtime VM, branch writer를 만들지 않는다. topology 저작은 기존 typed owner에
  남기며 이번 탭은 pan/zoom/fit 이외의 로직 상호작용을 제공하지 않는다.

회귀 검증은 saved Flow duplicate/reorder/pursuit 변경이 Sequencer cold admission을 0 patterns로 만들지
않는 경우, inline Product sequence drift는 계속 거부되는 경우, Save 직후 baseline 전환, active run 중
draft 편집과 replacement restart, malformed Counter 묶음의 경고 투영을 포함한다.

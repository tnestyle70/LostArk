# Animation Tool 담당자 인계와 Tool 경계

이 문서는 Animation 담당자가 오늘 바로 작업할 범위와 Character Preview, Effect Tool, Server gameplay
사이의 금지 경계를 고정한다. 세부 구현 전체 코드는 날짜별 PLAN에 두고, 이 문서는 담당 인터페이스만 소유한다.

## 1. 한 줄 계약

Animation Tool은 `어떤 animation asset의 어떤 clip에서 언제 cue가 발생하고 어느 anchor에 어떻게
붙는가`를 저작한다. Effect Tool은 `그 cue가 호출할 EffectAssetId 자체의 모양과 effect-local 수명`을
저작한다. Character Preview Panel은 두 Tool이 함께 보는 preview target과 매 frame anchor transform을
소유한다.

```text
Character Preview Panel
  preview target + model + root/weapon/bone anchor snapshot
          │
          ├─> Animation Tool: clip/playhead/event timing
          │                      │
          │                      └─> effectAssetId binding
          │
          └─> Effect Tool: emitter/module/trail/effect-local transform/lifetime
                                 │
                                 └─> future single Effect preview/runtime path
```

Effect asset의 원점과 emitter local transform은 재사용 가능한 asset 내부 값이다. character root,
weapon, bone, world point 중 무엇에 붙일지와 그 anchor 기준 offset, follow/detach, stop policy는 같은
EffectAssetId를 clip마다 다르게 사용할 수 있어야 하므로 Animation cue/binding 값이다.

## 2. 현재 코드와 목표 상태를 구분한다

`Render_TargetSelector`, `Select_PreviewAsset`, `Release_Preview`, `Refresh_PreviewLevel`과
`CPart_Body` preview 생성·제거는 `Client/{Public,Private}/CharacterPreviewPanel.{h,cpp}`의
`CCharacterPreviewPanel`로 이동했다. Animation Tool은 자기 document가 dirty인지만 패널에 알리고
preview 수명에는 관여하지 않는다.

패널이 아직 소유하지 않는 것은 preview camera/light/background, `mouseGroundPoint`, 다섯 class와
part 선택 UI, anchor slot 목록 열거다. 이 항목들을 채울 때도 preview 생성 경로를 각 Tool 안에
다시 만들지 않는다.

현재 `.animevents` Save는 destination을 직접 `w` mode로 열며, Reload는 Dirty 확인이 없다. Load는 임시
vector에 읽은 뒤 교체하지만 owner/count와 malformed row를 엄격히 거부하지 않는다. 오늘 Animation 담당자의
실제 수정 대상은 이 document safety와, 아직 닫힌 상태로 선언할 EffectAssetId binding 계약이다.

DimensionMaster로 rename된 `Data/Effects/Authored/.../Candidates`의 구형 authoring 문서 459개는 Effect Tool
재구축과 함께 삭제했다. 원본 추출 증거인 SourceCatalog/SourceExtracted와 Resources payload는 보존하지만
admitted effect 목록이나 제품 runtime cue에는 자동 연결하지 않는다.

### 2.1 2026-08-21 현재 Effect Tool과 Valtan 저작 계약

위의 G0 재구축 설명과 아래 `admitted Effect 0개` 문장은 당시 단계의 역사적 경계이며 현재 제품 계약이
아니다. 현재 Debug F1 `Effect Tool -> All Effects`는 direct-authored Player Product cue와 Valtan Product
cue를 실제 `EffectCatalog`에 join하고 같은 unified Effect editor를 연다.

```text
Player
  -> input slot / skill / stage / ordered clip cue
  -> unified Effect -> family -> editable element

Valtan
  -> phase / pattern / semantic stage / ordered clip occurrence / product cue
  -> unified Effect -> family -> editable element
```

두 owner 모두 Mesh, Sprite, Mesh Particle, Sprite Particle, Local Decal, Trail/Ribbon family tree와
`Open for Editing`, Play, Save를 공유한다. Save는 선택된 direct-authored 문서 하나만 원자 저장하고 같은
catalog revision의 prepared target을 교체한다. 실행 중 occurrence는 이전 document를 유지하고 다음 cue
spawn부터 새 document를 사용한다. 제품 교체가 실패하면 저장 파일을 이전 bytes로 되돌리고 이전 Product
pointer/cache를 유지한다. `Data/Effects/EffectCatalog.json`과 `Data/Effects/Authored/*.effect.json`만 제품
Effect 입력이며 schema, Catalog, source-family batch는 `Validate-EffectSources.ps1`로 검증한다. 복사본 생성,
Effect publish, 적용을 위한 Client 재시작은 없다. 다음 Client 실행도 같은 authored 파일을 직접 읽는다.
V1 Product가 실제 참조하는 DDS/WModel은 팀장 Drive의 `Client/Bin/Resources` 동일 상대 경로에 둔다.
Git은 authored source와 stable asset ID만 전달하며 Resource binary는 추적하지 않는다.
Animation Tool은 이 경로가 존재해도 Effect element를 직접 편집하지 않고 ordered clip과 cue timing/anchor를
소유한다. Valtan Server stage와 damage도 계속 Encounter/Server 권위이며 Effect Tool이 바꾸지 않는다.

## 3. Animation Tool이 소유하는 것

Animation 담당자는 다음을 계속 소유한다.

- animation asset과 clip 목록 표시
- Play/Pause/Step/Scrub/Loop와 clip local playhead
- clip chain, skill reference, notify reference의 읽기 전용 표시
- `.animevents`의 point/window marker 편집
- Effect Tool typed request로 stable `effectAssetId` point marker 생성
- `.animevents`의 parse → validate → stage → commit Save/Load/Reload
- `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`의 skillId → ordered model clip/BA stage 연결
- event document와 skill binding document의 독립 Dirty 상태, 실패 메시지와 기존 document 보존

Animation Tool의 저장 row는 다음 의미를 가진다.

```text
animationAssetId  파일 header owner. 어떤 class/preview asset의 timeline인지 식별한다.
clipName          해당 asset 안의 stable clip 이름이다.
startMs           clip 시작 기준 cue 시작 시각이다.
endMs             window cue의 종료 시각이다. point cue는 startMs와 같다.
eventKind         HIT/CANCEL/SUPERARMOR/INVULN/MOVE/SOUND/EFFECT/SHAKE다.
effectAssetId     authored EFFECT가 호출할 stable effect catalog ID다.
anchorSlotId      character root/weapon/bone/world 중 검증된 stable anchor ID다.
localTransform    anchor 기준 position/rotation/scale이다.
followPolicy      spawn 뒤 anchor를 계속 따를지 world transform을 snapshot할지 정한다.
stopPolicy        window 종료나 action 취소 때 emission/instance를 어떻게 끝낼지 정한다.
```

현재 `.animevents` v4는 `effectAssetId` point binding까지만 준비되어 있고 anchor/local transform,
follow/stop 직렬화는 아직 구현되지 않았다. 위 필드를 legacy payload 문자열에 임시로 넣지 않고 schema,
publisher, preview/runtime consumer를 함께 추가하는 후속 수직 슬라이스로 연다.

기존 EFFECT row의 `sPayload`는 원본 `.animnotify`에서 온 UE source cue 이름이거나 빈 문자열일 수 있다.
실측상 LanceMaster에는 `src=orig`가 없는 빈 legacy row도 한 건 있으므로 `bImported`만으로 runtime admission을
판정하면 안 된다. `bImported`는 재import provenance만 뜻한다. 새 schema는 EFFECT payload에
`SOURCE_REFERENCE`와 `EFFECT_ASSET_ID`를 명시적으로 구분한다. 저장 kind는 admission 결과의 영구
스냅샷이 아니다. `EFFECT_ASSET_ID`는 Save/Load/runtime마다 현재 catalog에서 다시 검증한다.
`Use Selected Effect`는 source row를 덮어쓰지 않고 별도 `EFFECT_ASSET_ID` binding을 만든다.

frame은 화면 표시값일 뿐 저장 정본이 아니다. 모델을 다른 tick rate로 다시 cook해도 marker의 실제 시간이
움직이지 않도록 millisecond를 저장한다.

## 4. Animation Tool이 소유하지 않는 것

Animation Tool에는 다음 기능을 넣지 않는다.

- Character, `CPart_Body`, weapon, effect runtime object의 새 생성 경로
- Effect emitter/module/material/texture/trail 수치 편집
- Effect를 직접 spawn하거나 Effect preview/runtime을 직접 update/render하는 코드
- socket, packet, Server damage, HP, cooldown 판정
- Character layer/tag/index를 추측해 target을 찾는 코드
- Effect Tool container 또는 파일을 직접 수정하는 코드
- `.skilltiming`, `.clipmap`, `.animnotify`, `.clipseq` 저장

Effect Tool에서 `Use Selected Effect`를 눌렀을 때 Animation Tool이 받는 입력은
`targetGeneration + animationAssetId + clipName + timeMs + effectAssetId` typed bind request 한 건뿐이다.
Animation Tool만 request를 소비해 해당 시점에 EFFECT marker를 한 번 만들고 document를 Dirty로 바꾼다.
빈 EFFECT marker를 먼저 만들지 않으며 source reference row는 불변이다.

## 5. Effect, collider, damage timing의 정확한 경계

Animation 화면에서 여러 timing을 한 timeline에 겹쳐 보는 것은 맞다. 그러나 현재 정본과 실행 권위는
다르다.

| timing | Tool에서 보이는 의미 | 제품 runtime 정답 |
|---|---|---|
| Effect spawn | animation clip의 presentation cue | Client Character presentation이 marker를 통과한 시점 |
| Trail start/end | animation에 붙는 presentation window | Client effect cue player와 anchor sampler |
| Collider active | hit shape가 활성일 authoring/reference window | Server collision/skill system |
| Damage apply | 판정 성공 뒤 damage를 적용할 action timing | Server 30 Hz room tick과 `Data/Balance` |
| Hitstop/Shake | 승인된 hit 결과의 presentation | Server hit result를 받은 Client presentation |

현재 `.animevents`를 저장하는 것만으로 Server collider와 damage timing은 바뀌지 않는다.
`Data/Balance/PlayerSkills.json`과 `DamageProfiles.json`이 Server 정본이다. Animation Tool의 HIT marker를
서버 정본으로 승격하려면 아래 수직 슬라이스가 별도로 필요하다.

```text
Animation HIT authoring marker
-> schema validation
-> gameplay balance publisher
-> Server runtime bootstrap
-> Server skill/collision tick
-> protocol/server harness
-> Client snapshot/event presentation
```

이 publisher와 harness가 생기기 전에는 Animation Tool의 HIT/collider 정보는 편집·비교용 authoring/reference
정보이며, damage가 연결됐다고 보고하지 않는다. Character/Animation 코드가 로컬 collider로 damage를 넣어
이 공백을 우회하지 않는다.

## 6. Burst와 Trail의 timing을 구분한다

- Burst/Impact: 한 시각에 시작하는 point EFFECT marker다. `startMs == endMs`다.
- Attached aura: 한 번 spawn한 뒤 Effect asset lifetime 또는 explicit stop 정책으로 끝난다.
- Weapon trail: 시작과 종료가 필요한 window binding이다. Animation Tool이 `startMs/endMs`를 소유하고,
  Effect Tool은 trail 폭, point lifetime, sampling 간격, texture/material을 소유한다.
- Beam/Tether: source와 end anchor 두 개가 필요하다. 단일 root offset으로 위장하지 않는다.

현재 EFFECT가 point event로만 저장되므로 weapon trail을 닫으려면 EFFECT window schema와 runtime
start/stop consumer를 먼저 추가해야 한다. trail lifetime을 임의의 Effect duration에 맞춰 point event처럼
저장하고 완료 처리하지 않는다.

## 7. Effect Tool이 소유하는 것

새 Effect Tool은 다음을 소유한다.

- stable `EffectAssetId`
- emitter/module/effect-local timeline
- texture/model/material과 Resources-relative asset ID
- blend, color, velocity, lifetime, SubUV, ribbon/trail 수치
- effect root와 emitter의 effect-local position/rotation/scale
- admitted effect catalog와 dependency/validation 상태
- authoring Save/Load/Reload와 runtime cook
- 하나의 검증된 Effect preview/runtime 경로

2026-08-04 재구축 G0의 실제 구현 범위는 ImGui의 `Mesh / Texture / Particle / Decal / Trail` 중 하나를
고르는 타입 selector뿐이다. 이 단계에는 EffectAssetId, 파일 format, Load/Save, catalog, preview/runtime
계약이 없다. 위 asset 책임은 새 schema와 소비자를 함께 검증하는 후속 G에서 한 계약씩 다시 연다.

Effect Tool은 Animation clip과 marker vector를 직접 편집하지 않는다. 후속 `Use Selected Effect`가 생겨도
선택한 `EffectAssetId`를 typed request로 제출할 뿐이며, Animation Tool에서 marker가 만들어지고 Save되기
전에는 binding이 영구 저장됐다고 표시하지 않는다.

### 7.1 Effect Tool V2 Group과 Valtan Action Workbench binding 경계

Effect Tool V2는 V2 leaf body와 재사용 가능한 Group body를 함께 소유한다.

```text
Data/Effects/V2/Authored/<effectId>.effectv2.json
  leaf의 mesh/texture/particle/decal/trail body와 effect-local 수명

Data/Effects/V2/Groups/<groupId>.effectv2group.json
  ordered leaf children
  + child-local start/duration/stop/offset/yaw/scale

Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
  CValtanActionWorkbench가 소유하는 exact Pattern occurrence
  + groupId 또는 direct leaf reference
  + stage/clip clock, start, anchor/follow/rotation/placement
```

- Composition에서 Group을 Pattern에 붙일 때 group children을 `Valtan.gameplay.json`이나
  `Valtan.presentation.json`에 펼쳐 복사하지 않는다. Effect Tool V2가 group body를 Save하고
  Composition은 `groupId` occurrence만 저장한다.
- Composition의 기본 palette는 `boss.valtan.*` Group 우선이다. direct leaf는
  `Advanced / Direct Leaves`에서만 같은 boss scope로 연다. 다른 owner의 leaf를 발탄 Pattern에
  연결하지 않는다.
- group binding이 펼치는 동일 leaf와 direct leaf의 expanded clock이 같으면 중복 재생이므로
  editor와 validator가 저장을 거부한다.
- 선택 animation box에 Group을 붙이면 그 box의 stable occurrence와 Stage-local start가 binding
  command의 baseline이다. Delete/Duplicate는 group body가 아니라 선택 exact binding row 하나만
  바꾸며 vector index를 identity로 쓰지 않는다.
- Composition에서 보이는 Group block 길이는 child max end의 display span이다. 이 표시는
  Pattern Stage duration이나 Group semantic duration을 자동으로 바꾸지 않는다.
- local Arena Clone은 저장 직후 Effect V2 catalog/runtime cache를 invalidation하고 현재 clock을
  restage해 저작 결과를 확인한다. Product Server Valtan은 publish/build와 Server restart,
  world re-entry가 봉인한 immutable presentation generation만 사용한다. `Reload Complete Play
  Inventory`는 이 Server generation을 갱신하지 않는다.

Animation Tool은 이 세 파일 중 Group body나 V2 binding을 편집하지 않는다. Animation/Composition이
공유하는 것은 선택 clip occurrence와 Stage-local clock뿐이며, Product 판정은 계속 Server Pattern
action과 fixed tick이 소유한다.

Boss Composition source는 이 owner들을 대체하지 않는다. `Data/Compositions/Bosses`의 manifest는
animation occurrence, V1 `effectAssetId`, V2 `groupId`/leaf reference, Sound와 Camera owner의 경로와
coverage만 stable ID로 묶는다. resolved Product에서 이 값들이 한 Stage의 `scope`, `clock`,
`stopPolicy`, `anchor`, `payload`로 함께 보이는 것은 파생 read model이며 정본 이동이 아니다.
Composition은 Effect 연결·시간·배치 reference를 다루고 Effect Tool은 asset/group body를,
Animation owner는 clip occurrence를, gameplay owner는 Stage/branch/hit를 계속 소유한다.

현재 Valtan manifest는 `SHADOW`이며 공용 `Action Workbench` 진입의 선행조건이 아니다.
Boss 선택은 Valtan/KoukuSaydon session을 같은 창에 연결한다. Valtan 상세 box 편집·Play·Save는
`CValtanActionWorkbench`/split writer, 쿠크는 독립 Composition session이 담당한다. Resources의 여섯 몸체
Animation inventory는 WModel metadata를 읽고 기존 CharacterPreviewPanel에서 플레이어 주변 모델을 생성한다.
clip 클릭은 Animation Tool의 창 열기 요청을 보내지 않는다. generated resolved Product를 재생하는 두 번째
runtime이나 모든 보스/owner를 저장하는 거대 writer를 추가하지 않는다.

## 8. Character Preview Panel이 소유하는 것

공용 Character Preview Panel은 다음을 소유한다.

- 다섯 class와 admitted part/model의 선택 UI
- preview Character/Body/Weapon의 Prototype → Clone → Layer 등록
- target 교체와 Level 전환 시 기존 preview 제거
- generation이 포함된 read-only preview target snapshot
- animation model과 root world transform
- stable anchor slot 목록과 매 frame anchor world transform
- preview camera/light/background와 mouse ground point

두 Tool에 제공할 최소 snapshot은 다음 의미를 가진다.

```text
targetGeneration   target 교체 때 증가하며 오래된 Tool request를 거부한다.
characterClassId   roster의 stable class ID다.
animationAssetId   Data/Animation owner를 찾는 stable asset ID다.
model              현재 살아 있는 preview model weak reference다.
rootTransform      character root의 현재 world transform이다.
anchors            stable anchorSlotId -> current world transform 검색 결과다.
mouseGroundPoint   현재 preview frame의 임시 world point다. asset에 좌표로 저장하지 않는다.
```

Preview Panel은 `.animevents`와 Effect asset을 저장하지 않는다. Tool별 선택 row, Dirty, event, emitter를
소유하지 않는다.

`targetGeneration`은 `Bind_Preview`, `Unbind_Preview`, target 교체, Level teardown 때 증가한다. Effect bind
request가 가진 generation과 소비 시점의 current generation이 다르면 같은 class/asset ID여도 stale로
거부한다.

## 9. Anchor와 위치 저장 규칙

Effect asset에는 실제 world coordinate, character/weapon anchor 또는 현재 마우스 좌표를 저장하지 않는다.
Animation cue/binding이 다음 정책을 저장한다.

| anchor kind | 저장 의미 | preview/runtime 입력 |
|---|---|---|
| `CHARACTER_ROOT` | character root를 따라간다 | current root transform |
| `WEAPON_SOCKET` | stable weapon anchor를 따라간다 | `anchorSlotId`의 current transform |
| `MODEL_BONE` | 검증된 skeleton anchor를 따라간다 | stable bone/socket ID의 current transform |
| `WORLD_POINT` | spawn 시점 world point에 남는다 | cue context world position |
| `MOUSE_GROUND` | spawn 시 커서 ground point를 사용한다 | picking 결과의 current world point |

Animation Tool은 EFFECT cue의
`anchorKind + anchorSlotId + localPosition + localRotation + localScale + followPolicy + stopPolicy`를
편집한다. Preview Panel은 그 anchor의 current world transform을 계산한다. Effect runtime은 cue의
attachment transform과 Effect asset의 effect-local transform을 합성한다.

`MOUSE_GROUND`는 “마우스 커서 anchor를 사용한다”는 정책만 저장한다. 사용자가 preview 중 가리킨 실제
좌표를 Effect asset에 저장하지 않는다. 제품 스킬에서는 입력 command와 Server 승인 계약이 허용한 ground
point를 cue context로 넘긴다.

weapon/bone은 raw bone index를 저장하지 않는다. `CharacterSpec` 또는 별도 anchor catalog가 검증한 stable
`anchorSlotId`만 저장한다. anchor가 사라지거나 target generation이 바뀌면 해당 effect의 kill/detach 정책에
따라 종료하며 stale pointer를 계속 사용하지 않는다.

## 10. 세 Tool의 호출 흐름

아래 흐름은 admitted fixture와 catalog resolver가 추가된 다음 Effect/Preview 단계의 최종 목표다.
현재 admitted Effect가 0개인 오늘의 Animation 안전성 검증에서는 `Use Selected Effect` 성공을 완료
증거로 삼지 않는다.

### 10.1 Burst Effect 저작

```text
Preview Panel에서 class/target 선택
-> Animation Tool에서 clip 선택, pause/scrub
-> Effect Tool에서 admitted EffectAssetId 선택·preview
-> Use Selected Effect typed request
-> Animation Tool이 anchor/offset/follow/stop을 가진 EFFECT_ASSET_ID point marker 한 건 생성, Dirty
-> atomic Save
-> staged Reload
-> 같은 asset/clip/ms/EffectAssetId/attachment binding 복원
```

### 10.2 Weapon Trail 저작

```text
Preview Panel이 weapon anchor를 매 frame 제공
-> Animation Tool이 trail start/end window 저작
-> Animation Tool이 WEAPON_SOCKET + stable anchorSlotId와 offset 선택
-> Effect Tool이 ribbon/trail width/lifetime/sampling/material 편집
-> 단일 Effect preview/runtime이 매 frame anchor를 sample
-> Animation window 종료 시 cue stop
```

이 두 번째 흐름은 EFFECT window schema와 runtime start/stop consumer가 구현된 뒤에만 완료다.

## 11. Animation 담당자의 오늘 작업

1. `Save_Events`를 sibling temp write → flush/close → strict reparse/validate → atomic replace로 바꾼다.
2. Save 성공 뒤에만 Dirty를 해제한다. 실패하면 기존 destination과 memory document를 유지한다.
3. Dirty Reload에 `Discard and Reload` 확인을 추가한다.
4. Load에서 magic/version/header owner/declared count/row/clip/time을 검증하고 staged vector 전체 성공 뒤 commit한다.
5. read-only playhead snapshot으로
   `targetGeneration/animationAssetId/clipName/timeMs/durationMs/paused`를 제공한다.
6. legacy v3 EFFECT는 기본 `SOURCE_REFERENCE`로 읽는다. typed Effect bind request와
   `EFFECT_ASSET_ID` kind는 계약을 선언하되, catalog resolver가 없는 현재 상태에서는 생성/Save를 거부하고
   기존 document를 보존한다. 성공 binding은 admitted fixture/resolver와 같은 후속 G에서 연다.
7. target 교체·unbind·Level teardown 때 generation을 증가시켜 이전 read-only snapshot을 무효화한다.
8. Dirty 상태에서 target/Level이 바뀌어도 `Sync_AssetName()`이 document를 초기화하지 않는다. 새 target은
   pending으로 보존하고 Save/Discard가 결정될 때까지 old document를 유지한다. Level transition request는
   target teardown 전에 전역 modal에서 Save/Discard/Cancel 중 하나가 끝날 때까지 pending으로 유지한다. 공용 Preview Panel 이동
   전에는 Dirty일 때 현재 TargetSelector 변경을 비활성화한다. 같은 Level의 외부 Character Select
   `Bind/Unbind`는 아직 사전 취소할 수 없다. 이 경우 old document를 사후 격리하고 새 model로 편집·Save하지
   않는다. 원래 class를 다시 선택해 같은 asset target을 복원한 뒤 Save하거나, conflict UI에서 명시적으로
   Discard한 뒤 새 target을 adopt한다. 모든 외부 target 교체의 pre-commit guard는 공용 Character Preview
   Panel 단계에서 하나로 통합한다.
9. TargetSelector/preview 생성 코드에는 신규 기능을 추가하지 않는다.
10. `.skilltiming/.clipmap/.animnotify/.clipseq`는 계속 read-only다.

## 12. Animation 담당자에게 그대로 전달할 문장

> Animation Tool의 Playback/Chain/Event/Reference/Clip 편집 책임은 유지해 주세요. 현재 Tool 내부의
> TargetSelector와 CPart_Body preview 생성·제거는 공용 Character Preview Panel로 이동할 예정이므로 그
> 부분에는 신규 기능을 추가하지 않습니다. 오늘 범위는 `.animevents` 원자 Save, Dirty Reload 확인,
> strict staged Load, target generation이 포함된 read-only playhead snapshot, admitted resolver가 연결된 다음
> stable EffectAssetId point marker를 만들 typed request shape와 현재의 closed admission gate입니다. Effect
> runtime, Character/GameObject 생성, Server collider/damage 판정은 Animation Tool에
> 넣지 않습니다. `.skilltiming/.clipmap/.animnotify/.clipseq`는 read-only를 유지합니다. 현재 HIT marker는
> Server balance publisher가 연결되기 전까지 damage 정본이 아닙니다.

## 13. 오늘의 종료 증거

```text
기존 네 class v3 .animevents strict staged Load
-> 모든 legacy EFFECT를 SOURCE_REFERENCE로 분류
-> non-admitted v4 EFFECT_ASSET_ID 입력 거부와 rollback
-> Dirty Reload Cancel/Discard 검증
-> malformed owner/count/row 실패에서 memory/destination 보존
-> target 교체/unbind/Level teardown 시 generation 증가와 이전 snapshot 무효화
-> Dirty target 변경 시 old document 보존
-> Dirty Level transition 시 target teardown 전 Save/Discard/Cancel
-> 같은 Level 외부 class 교체 시 새 model 편집 차단, 원래 class 복귀 Save 또는 명시적 Discard
```

자동 검증과 수동 검증을 구분한다.

- 자동: temp failure rollback, owner/count/malformed row 거부, non-admitted bind gate, Debug Client build,
  당시 animation binding/effect cue focused 계약 실행 결과, `git diff --check`. 광역 Client frontend
  계약 프로젝트는 현재 정본 regression에서 제거됐다.
- 수동: 다섯 class target, scrub 시각, Dirty target 전환 보존과 Level 전환 cleanup.
- 미완료로 남길 것: admitted Effect fixture/catalog resolver, 실제 Effect Tool `Use Selected Effect` 성공 경로,
  anchor-relative local transform 편집, Server collider/damage publisher, EFFECT window/trail start-stop,
  beam dual anchor, 새 Effect schema와 admitted fixture/catalog.

root/weapon anchor의 현재 world transform 조회는
`CAnimationTargetService::Resolve_AnchorTransform` / `Resolve_RootTransform`으로 열렸다. 없는 bone은 false를
반환하므로 누락된 anchor를 원점으로 위장하지 않는다. 아직 없는 것은 새 Effect runtime preview가 이 값을
소비하는 경로와 anchor에 상대적인 local transform 저작이다.

현재 admitted Effect는 0개이고 구형 candidate authoring 문서 459개도 삭제됐다. 따라서 오늘 Animation 문서
안전성 작업을 EffectAssetId 성공 Save나 preview로 검증하지 않는다. `EFFECT_ASSET_ID` 입력은 admission
resolver가 없거나 ID가 catalog에 없으면 실패하고 memory/destination을 유지해야 한다. 다음 Effect/Preview
설계 단계에서 새 schema와 의존성 검증을 통과한 fixture 1개, catalog resolver를 먼저 만든 뒤 실제
`Use Selected Effect` 성공 흐름을 연다.

이 경계가 닫히면 Animation 담당자는 Animation document에 집중하고, Effect 담당자는 Animation Tool 내부나
Character 생성 코드를 수정하지 않고 Effect와 anchor 동작을 독립적으로 발전시킬 수 있다.

## 14. Character와 Weapon asset 저장·연결 인계

이 절은 Animation 담당자가 "원본은 어디에 있고, Runtime은 무엇을 읽으며, Tool은 어느 객체를 편집하는가"를
한 번에 찾기 위한 경로 정본이다. 자동 검증 완료와 수동 화면 검증 대기는 아래에서 명시적으로 구분한다.

### 14.1 저장 위치를 수명별로 구분한다

| 수명 | 위치 | 저장하는 것 | Git |
|---|---|---|---|
| 설치 원본 | `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages` | 난독화된 원본 UPK | 금지 |
| 원본 DB/LookInfo 증거 | `C:/Users/user/Desktop/LostArk_Legacy_Quarantine_20260803/Resources/LostArk_SourceData/LPK` | PC/Item DB, Action/LookInfo `.loa` | 금지 |
| 추출·조리 작업공간 | `C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Character` | PSK/PSA, glTF, FBX, Blender, 조리 staging | 금지 |
| Client Runtime | `Client/Bin/Resources/Character` | `.wmodel`과 그 모델이 참조하는 texture | 팀장 Drive 물리 입력; Git 추적 금지 |
| Runtime 등록 정본 | `Data/Actors/CharacterCatalog.json` | stable class ID와 Resources-relative model asset ID | 허용 |
| Animation 저작 정본 | `Data/Animation/Authored/<AssetName>/<AssetName>.animevents` | 팀이 편집하는 animation event | 허용 |
| Animation 원본 참조 | `Data/Animation/Reference/<AssetName>` | 추출된 notify/clip/skill timing reference | 허용 |
| 팀 리소스 관리 | `Client/Bin/Resources` 물리 폴더 | 팀장이 전달한 runtime 리소스 | Drive only; Git index에 넣지 않음 |

`Client/Bin/Resources`에는 UPK, PSK, PSA, FBX, Blender 파일을 넣지 않는다. Runtime은 Assimp로 원본을
읽지 않고 `ModelAssetConverter`가 미리 조리한 `.wmodel`만 `CModel` 경로로 읽는다. 절대 경로나 drive 경로는
Catalog asset ID로 저장하지 않는다.

### 14.2 현재 다섯 Character asset

```text
Client/Bin/Resources/Character/
  LanceMaster/                 body + 5 equipment
  WP_WFLM_00L/                 현재 장착하는 긴 창
  WP_WFLM_00S/                 조리돼 있지만 현재 stance swap은 없음

  GunSlinger/                  body + 5 equipment
  WP_WGDH_02H/                 현재 양손에 같은 권총 model을 복제
  WP_WGDH_02S/                 조리돼 있지만 shotgun swap은 없음
  WP_WGDH_02L/                 조리돼 있지만 rifle swap은 없음

  Slayer/                      body + 5 equipment
  WP_WWBK_03/                  현재 장착하는 대검

  Artist/                      body + 5 equipment
  WP_WSDM_09/                  현재 장착하는 붓

  DimensionMaster/
    DimensionMaster_Character.wmodel
    DimensionMaster_DimensionCore.wmodel
    DimensionMaster_DimensionSummon.wmodel
    textures/
```

현재 다섯 class의 장착 무기는 모두 `animations=0`, `skeleton=no`인 정적 WModel이다. 무기 자체가 locomotion
clip을 재생하는 것이 아니라 Character body의 socket bone matrix를 따라간다. 차원술사 body/core/summon은
`Character/DimensionMaster`, 기본 무기 네 파츠는 `Character/WP_WSWP_M_06`에 분리한다.

### 14.3 현재 Character 생성 연결

```text
Data/Actors/CharacterCatalog.json
  bodyModel + equipmentModels + weaponModels[]
        |
        v
CActorCatalog
  parse -> validate -> staged character entry commit
        |
        v
CPlayableCharacterAssetService::Ensure_Prototypes
  Resources-relative asset ID resolve
  -> CModel::Create
  -> level prototype 등록
        |
        v
CHARACTER_SPEC
  body/equipment/weapon prototype tag와 body socket 선언
        |
        v
CCharacter::Ready_PartObjects
  Part_00_Body
  -> Part_10_Equip_*
  -> Part_90_Weapon_*
        |
        v
CPart_Equipment socket mode
  static weapon local matrix
  * body socket bone matrix
  * character world matrix
```

기존 class의 현재 장착 형태는 다음과 같다.

| class | Runtime model | Character socket | 현재 범위 |
|---|---|---|---|
| Lance Master | `WP_WFLM_00L.wmodel` | `b_weapon_rhand` | 긴 창 하나 |
| Gunslinger | `WP_WGDH_02H.wmodel` 두 instance | `b_wp_1`, `b_wp_2` | 양손 권총 |
| Slayer | `WP_WWBK_03.wmodel` | `b_weapon_rhand` | 대검 하나 |
| Artist | `WP_WSDM_09.wmodel` | `b_wp_1` | 붓 하나 |
| DimensionMaster | `WP_WSWP_M_06{L,S,P,E}.wmodel` | `b_wp_swm_m_1`, `_2`, `_3`, `_4_02` | 네 파츠, E socket yaw 180도 |

`CharacterCatalog`은 생성 가능한 asset을 소유하고, `CHARACTER_SPEC`은 해당 class가 몇 개의 part instance를
만들고 어느 socket에 붙이는지를 소유한다. Server에는 model path, prototype tag, socket 이름을 넣지 않는다.
Server와 remote Client는 `CHARACTER_CLASS_ID`만 공유하며 각 Client가 같은 Catalog와 Resources로 표현한다.

### 14.4 현재 Animation Tool 연결

Character Select가 Server snapshot class에 맞춰 replicated `CCharacter` presentation을 교체할 때
`CAnimationTargetService::Bind`를 호출한다. Animation Tool에서 `Scene Character`를 선택하면 다음 경로를 사용한다.

```text
CLevel_CharacterSelect replicated local CCharacter
  -> CAnimationTargetService::Bind(character)
  -> Resolve_Character()
  -> Resolve_Model() == character body CModel
  -> Resolve_AssetName() == CHARACTER_SPEC::pAssetName
  -> Data/Animation/{Authored,Reference}/<AssetName>
```

따라서 `Scene Character`의 timeline 대상은 body CModel이다. 장비와 무기는 별도 timeline target이 아니라
scene Character의 part로 함께 보이고 body socket을 따라간다. Animation 담당자는 weapon model을 다시
Clone하거나 layer에서 직접 찾지 않는다.

현재 차원술사의 별도 preview target은 `Client/Public/AnimationPreviewAssets.h`에 다음 세 개만 있다.

```text
dimensionmaster.character          DimensionMaster body 154 clips
dimensionmaster.dimension-core     preview-only core 1 clip
dimensionmaster.dimension-summon   preview-only summon 2 clips
```

Core와 Summon을 선택하면 Tool이 `CPart_Body` preview 하나를 scene Character 오른쪽에 만들고
`Bind_Preview(model, assetName)`으로 그 모델만 timeline target으로 바꾼다. 이 두 모델은 기본 장착 무기가
아니며 Character socket part로 해석하지 않는다.

### 14.5 차원술사 원본 무기 증거

원본 class와 기본 무기 연결은 다음과 같이 확인됐다.

```text
EFTable_PC.PC
  PrimaryKey      = 612
  Name            = DimensionMaster
  TownDefaultWeapon = 106600901

DEV/EFTable_Item.Item
  PrimaryKey = 106600901
  Model      = EFDLItem_WP_WSWP_M_06-2.WP_WSWP_M_06-2
```

원본 package와 LookInfo 경로는 다음과 같다.

```text
UPK
  C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/
  8V2N8G8V2CA2R06F04K1ECYE.upk

Item LookInfo
  .../LPK/data4/EFGame_Extra/ClientData/XmlData/LookInfo/Item/
  EFDLItem_WP_WSWP_M_06-2.WP_WSWP_M_06-2.loa

Character Action
  .../LPK/data3/EFGame_Extra/ClientData/XmlData/Action/DIMENSIONMASTER.loa
```

LookInfo가 선언하는 네 파츠와 body socket은 다음이 정답이다.

| source mesh | LookInfo part | body bone | 성격 |
|---|---|---|---|
| `WP_WSWP_M_06L_SK` | `WP_SWM_M_1` | `b_wp_swm_m_1` | 긴 blade part |
| `WP_WSWP_M_06S_SK` | `WP_SWM_M_2` | `b_wp_swm_m_2` | 짧은 blade part |
| `WP_WSWP_M_06P_SK` | `WP_SWM_M_3` | `b_wp_swm_m_3` | 조립형 paired part |
| `WP_WSWP_M_06E_SK` | `WP_SWM_M_4` | `b_wp_swm_m_4_02` | clock/core part, socket Yaw 180도 |

L/S/P는 root bone 하나이고 E는 8-bone clockwork skeleton, shared AnimSet과 SkelControl을 가진다. 원본
`DIMENSIONMASTER.loa`에는 `Mode_Battle`, `Mode_Normal`, `CEFActionNotify_ReAttachParts`가 있다. 따라서
원작 완전 재현은 정적 표시와 별개의 weapon mode/reattach 수직 슬라이스다.

### 14.6 차원술사 무기 현재 저장 구조

임시 추출 폴더에 의존하지 않도록 원본 추출 결과는 다음 외부 작업공간에 보존했다.

```text
C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Character/DimensionMaster/
  ActorX/WP_WSWP_M_06/
    MaterialInstanceConstant/
    SkeletalMesh3/
    Texture2D/
  GLTF/WP_WSWP_M_06/
    MaterialInstanceConstant/
    SkeletalMesh3/
    Texture2D/
```

Client Runtime에는 게임이 실제 사용하는 조리 결과만 둔다.

```text
Client/Bin/Resources/Character/WP_WSWP_M_06/
  WP_WSWP_M_06L.wmodel
  WP_WSWP_M_06S.wmodel
  WP_WSWP_M_06P.wmodel
  WP_WSWP_M_06E.wmodel
  textures/
    wp_wswp_m_06l_*
    wp_wswp_m_06s_*
    wp_wswp_m_06e_*
```

네 파츠는 기존 네 class와 같은 수준의 정적 bind pose WModel로 조리했다. 원본 glTF의 meter 단위는 body와
같은 centimeter 조립 계약에 맞게 converter에서 100배 변환했다. 각 WModel은 material v2의 diffuse, normal,
emissive, ORM 참조를 보존한다. E 내부 clockwork animation과 SkelControl은 이 완료 범위에 포함하지 않는다.

Git 데이터는 `CharacterCatalog` formatVersion 2와 `weaponModels` 배열로 전환됐다.

```json
{
  "archetypeId": "PLAYER_DIMENSIONMASTER",
  "networkClassId": "DIMENSIONMASTER",
  "assetId": "DimensionMaster",
  "bodyModel": "Character/DimensionMaster/DimensionMaster_Character.wmodel",
  "equipmentModels": [],
  "weaponModels": [
    "Character/WP_WSWP_M_06/WP_WSWP_M_06L.wmodel",
    "Character/WP_WSWP_M_06/WP_WSWP_M_06S.wmodel",
    "Character/WP_WSWP_M_06/WP_WSWP_M_06P.wmodel",
    "Character/WP_WSWP_M_06/WP_WSWP_M_06E.wmodel"
  ],
  "animationSetId": "ANIM_DIMENSIONMASTER",
  "runtimeStatus": "supported"
}
```

기존 네 class도 `weaponModels` 배열을 사용하되 각각 model definition 한 개를 저장한다. Gunslinger는 Catalog에
같은 권총 경로를 두 번 저장하지 않고 `CHARACTER_SPEC`에서 한 Prototype을 양손 socket에 두 번 Clone한다.

### 14.7 차원술사 무기 현재 Runtime 연결

```text
CharacterCatalog.weaponModels[4]
  -> CActorCatalog::CHARACTER_ACTOR_ENTRY::weaponModels
  -> CPlayableCharacterAssetService가 L/S/P/E 네 Prototype stage
  -> 전체 decode 성공 뒤 level Prototype 등록
  -> Spec_DimensionMaster::WEAPON_PART_SPEC[4]
  -> CCharacter::Ready_PartObjects가 네 CPart_Equipment 생성
  -> 각 part가 body bone matrix를 따라감
```

현재 part 계약은 다음과 같다.

```text
Part_90_Weapon_L -> Prototype_Component_Model_DimensionMaster_Weapon_L
                 -> b_wp_swm_m_1

Part_91_Weapon_S -> Prototype_Component_Model_DimensionMaster_Weapon_S
                 -> b_wp_swm_m_2

Part_92_Weapon_P -> Prototype_Component_Model_DimensionMaster_Weapon_P
                 -> b_wp_swm_m_3

Part_93_Weapon_E -> Prototype_Component_Model_DimensionMaster_Weapon_E
                 -> b_wp_swm_m_4_02 + socket-local Yaw 180 degrees
```

`WEAPON_PART_SPEC::fSocketYawDegrees`와 `PART_EQUIPMENT_DESC::fSocketYawDegrees`를 추가해 E의 원본 180도
회전을 socket-local 값으로 전달한다. `CPart_Equipment::Update`는 이 local 회전을 body socket bone matrix보다
먼저 적용한다. 모든 WModel은 첫 Prototype 등록 전에 decode한다. Prototype Manager 자체의 batch/rollback API는
아직 없으므로 등록 도중의 예외까지 원자 보장했다고 주장하지 않는다.

### 14.8 Animation 담당자가 확인할 것

무기 수직 슬라이스가 반영된 뒤 Animation 담당자는 다음만 확인한다.

1. Lobby -> Character Select -> DimensionMaster 선택 후 F1 -> Animation Tool -> `Scene Character`를 선택한다.
2. `pc_sp_m_00_sk_idle_battle_1`에서 L/S/P/E 네 파츠가 모두 보이는지 확인한다.
3. `pc_sp_m_00_sk_run_battle_1`에서 네 파츠가 body bone을 따라가고 원점에 남지 않는지 확인한다.
4. `pc_sp_m_00_sk_mode_battle`, `pc_sp_m_00_sk_mode_normal`을 scrub해 body weapon bone 이동을 관찰한다.
5. 첫 수직 슬라이스에는 원본 `ReAttachParts`가 없으므로 mode-normal 외형을 원작 완료로 판정하지 않는다.
6. Dimension Core와 Dimension Summon은 기존 별도 target으로 확인하며 기본 무기와 합치지 않는다.
7. Test/Bern/Valtan의 local/remote DimensionMaster가 Character Select와 같은 네 파츠를 사용하는지 확인한다.

Animation 담당자는 무기 WModel을 직접 load하거나 `CAnimationTargetService`에 L/S/P/E target을 임의로
추가하지 않는다. L/S/P는 animation target이 아니며 E도 첫 수직 슬라이스에서는 정적 bind pose다. E 내부
AnimSet을 실제로 살릴 때는 animated weapon part와 preview target을 함께 설계한 별도 변경으로 연다.

### 14.9 완료와 미완료를 구분한다

현재 자동 검증으로 닫힌 범위는 다음과 같다.

```text
원본 DB/LookInfo mapping 기록
-> L/S/P/E 네 WModel과 texture 존재
-> CharacterCatalog weaponModels[4]
-> 네 Prototype tag와 네 body socket attachment 코드
-> E socket-local Yaw 180도
-> ModelAssetConverter info 네 파일 정상
-> Client x64 Debug 전체 Rebuild 성공
```

다음 실행 증거가 추가되면 "차원술사 기본 무기 표시"를 완전히 완료 처리한다.

```text
-> 네 body socket 위치·회전 육안 확인
-> Character Select Scene Character 표시
-> Test/Bern/Valtan local/remote 표시
-> 팀장이 관리하는 `Client/Bin/Resources`에서 모델·texture 실물 확인
```

다음은 계속 미완료로 기록한다.

- E 내부 clockwork animation과 SkelControl
- `Mode_Normal <-> Mode_Battle` weapon reattach
- skill별 weapon visibility/재배치
- weapon trail과 hit window
- Server action과 weapon presentation mode 연결
- DimensionMaster skill/effect admission

Animation Tool은 위 미완료 기능을 로컬 clip 재생이나 임의 part toggle로 우회하지 않는다. Server action이
필요한 표현은 command -> Server approval -> replicated action -> Character presentation 계약이 생긴 뒤에만
제품 runtime에 연결한다.

## 15. Key/Skill Animation Binding 작업 절차

### 15.1 기존 데이터 구조에서 확장된 경계

이번 확장은 키, 스킬 수치, 애니메이션을 한 JSON에 합치지 않는다. 각 정본은 다음처럼 분리된다.

| 정본 | 소유 정보 | 수정 담당 |
|---|---|---|
| `Data/Balance/PlayerSkills.json` | class, `inputSlot`, `skillId`, `skillKind`, Server timing, resource, range, `comboStages` | Gameplay/Server |
| `Data/Balance/DamageProfiles.json` | Server damage rate와 판정 profile | Gameplay/Server |
| `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json` | `animationAssetId`, `characterClass`, skillId별 ordered model clip | Animation |
| `Data/Animation/Reference`와 `.skilltiming/.clipmap/.animnotify/.clipseq` | 원작 추출·비교·초기 유도 자료 | read-only |
| Server snapshot | 승인된 `action`, `skillId`, `actionStartTick`, `iComboStage` | Server runtime |

`skillbindings.json`의 저장 schema는 다음 형태다.

```json
{
  "schema": "lostark.animation-skill-bindings",
  "formatVersion": 3,
  "animationAssetId": "DimensionMaster",
  "characterClass": "DIMENSIONMASTER",
  "bindings": [
    { "skillId": 2050110, "clips": ["pc_sp_m_00_sk_..."] },
    {
      "skillId": 2050010,
      "clips": [
        [ { "clip": "pc_sp_m_00_sk_att_battle_1_01", "playMs": 3000, "playRate": 2.0 } ],
        [ "pc_sp_m_00_sk_att_battle_1_03" ],
        [ "pc_sp_m_00_sk_att_battle_1_04" ]
      ]
    }
  ]
}
```

문서는 현재 class의 `PlayerSkills.json` 정의를 정확히 한 번씩 전부 포함해야 한다. 알 수 없는/중복/누락
skillId, 다른 owner class, 현재 model에 없는 clip, COMBO의 `comboStages`와 다른 clip 수는 Save 전에
거부한다. `inputSlot`, `skillKind`, timing을 중복 저장하지 않으므로 두 정본이 갈라지지 않는다.

### 15.2 Animation 담당자의 실제 작업

1. Server와 Client를 실행해 Lobby에서 `Character Select`로 이동한다.
2. 작업할 class를 ImGui에서 선택하고 F1 → Animation Tool → `Scene Character`를 선택한다.
3. 기존 전체 clip 목록에서 확인할 clip을 고르고 Play/Pause/Scrub으로 동작을 확인한다.
4. `Key -> Skill Animation` 패널에서 원하는 key/skill row를 연다.
5. ACTIVE 스킬은 `Assign Current Clip`으로 현재 clip을 step에 넣는다. 필요한 경우 step을 추가하고 순서를
   바꾸거나 제거해 하나 이상의 ordered clip chain을 만든다.
6. LMB COMBO는 Server `comboStages` 수만큼 고정된 row에 현재 clip group을 각각 지정한다.
   차원술사 `2050010`은 `_01(3000ms source/2x)`, `_03`, `_04`의 automatic 3-stage다. BA 단계 수와
   automatic/manual timing 자체는 Animation Tool에서 추가·삭제하거나 변경하지 않는다.
7. Save를 누른다. Tool은 sibling temporary file에 쓴 뒤 flush, strict reparse/validate, destination replace를
   수행한다. 실패하면 기존 destination 문서를 유지한다.
8. 저장 성공 후 실행 중 Character는 새 mapping을 받는다. action 도중 reload한 chain은 즉시 포인터를
   바꾸지 않고 다음 action 경계에서 commit한다.
9. Server 연결 Character Select gameplay에서 실제 key 또는 LMB를 입력해 command → approval → snapshot →
   지정 clip 재생을 확인한다. Remote character도 같은 snapshot stage를 소비하는지 함께 확인한다.

문서가 없거나 잘못됐을 때는 `Create Repair Draft from Current Clip`을 사용한다. 이 기능은 현재 class의 모든
Server skill을 임시로 채운 완전한 draft를 만들고 Dirty 상태로 남긴다. 작업자는 각 row를 올바른 clip으로
교체한 뒤 Save해야 한다. 잘못된 문서를 조용히 정상값처럼 채택하거나 spawn 실패로 승격하지 않는다.

### 15.3 재수정과 안전 경계

- 애니메이션만 바꿀 때는 해당 row의 clip/순서만 수정하고 Save한다. key나 skillId를 바꾸지 않는다.
- key 편성, 새 스킬, combo 단계 수가 바뀌면 Gameplay 담당자가 먼저 `PlayerSkills.json`과 damage 계약을
  갱신하고 publisher/Server contract를 통과시킨다. 그 다음 Tool의 repair draft로 신규 row를 포함시켜
  animation을 배정한다.
- ACTIVE의 마지막 clip은 Server가 `NONE`을 내릴 때까지 마지막 pose를 유지한다. COMBO는 Client가 시간을
  세어 다음 BA로 넘어가지 않고 Server `iComboStage`가 바뀔 때 해당 단계로 직접 이동한다.
- presentation 문서 누락/오류/clip miss는 해당 action 표현만 한 번 보고하고 transform, HUD, 다른 player
  replication과 Character spawn은 계속 진행한다.
- Dimension Core/Summon과 reference preview는 playable Scene Character가 아니므로 key/skill binding Save
  대상이 아니다.
- Effect cue, trail, collider authoring은 이 문서와 별도 정본이다. 특히 collider/damage timing은 계속
  Server data/publisher 수직 슬라이스로만 변경한다.

### 15.4 현재 class별 row

```text
Lance Master    Q W E R A S T V ALT_V + LMB(4단)
Gunslinger      Q W E R A S D F T V ALT_V + LMB(3단)
Slayer          Q W E R A S D F V ALT_V + LMB(4단)
Artist          Q W E R A S T V Z ALT_V + LMB(4단)
DimensionMaster Q W E R A S D F T V ALT_V + LMB(3단 automatic)
```

DimensionMaster의 LMB `2050010`은 한 번의 command로 세 Server stage가 자동 진행되고 Animation Tool은
snapshot `iComboStage`에 따라 위 세 clip group을 재생한다. Product Effect suffix는 stage ordinal이
아니다. 고정 cue는 `_01 -> ba2`, `_03 -> ba3`, `_04 -> ba1`이며 세 cue 모두
`root/follow/action_facing/natural`을 사용한다. 기존 `ba4` authored payload는 삭제·이름 변경하지 않지만
현재 3-stage 제품 chain에는 연결하지 않는다. `ALT_V`는 `PlayerSkills.json`의 `2050540`과 skill binding의
`pc_sp_m_00_sk_sk_super_timewave`를 사용한다. Tool 화면은 위 목록을 하드코딩하지 않고 `PlayerSkills.json`을 정렬해
그리므로 이후 합법적으로 추가되는 `Z`, `SPACE`, `RMB` 등의 slot도 숨기지 않는다.

## 16. Character Select Server Arena 검증 흐름 (2026-08-05)

Character Select는 Lobby가 Server 승인을 받은 뒤에만 같은 visual map을 연다. offline Preview와
`Preview / Server Play` mode 선택은 없다. Animation 담당자는 Server Arena에서 class thumbnail을 선택하고,
Server 승인 snapshot으로 `Scene Character`가 교체된 뒤 clip mapping과 실제 skill presentation을 확인한다.
Character Select Level 자체는 connect/send/entry approval을 반복하지 않는다.

```text
Lobby Character Select -> Server approval -> 같은 map Server Arena 진입
   -> class thumbnail 선택 -> Server class-change approval/snapshot -> Scene Character 교체
   -> F1 Animation Tool -> key/skill row 편집 -> Save
   -> Q/W/E/R/A/S/D/F/T/V 또는 LMB 입력
   -> Server approval/snapshot -> 저장한 ACTIVE/COMBO clip 재생 확인
```

- 저장 정본은 `Data/Animation/Authored/<Class>/<Class>.skillbindings.json`이다.
- ImGui가 일반 keyboard를 capture해도 gameplay key 검증은 가능하다. 단, InputText 편집 중에는 gameplay
  command를 보내지 않으며 편집 종료 시 누르고 있던 key도 새 press로 오인하지 않는다.
- F6는 follow/free camera를 전환한다. free camera에서는 gameplay command를 제출하지 않으며 follow로
  돌아온 뒤 새 key press부터 다시 제출한다.
- `Summon Valtan (Lazy)`는 animation 저작 기능이 아니라 Server-authoritative 검증 target 생성 명령이다.
  첫 요청에서 presentation asset을 준비하며, 중복 요청은 기존 Server entity를 재사용한다.
- Valtan collider, damage timing, effect cue를 조정할 때도 animation binding JSON에 판정 수치를 넣지 않는다.
  각 정본 데이터와 publisher를 통해 별도 수직 슬라이스로 변경한다.

## 17. 쿠크·세이튼 기획 액션과 로컬 Pattern 저작

### 17.1 이름과 순서 정본

쿠크·세이튼 Animation Tool 목록은 WModel clip index나 파일명 정렬로 만들지 않는다. 다음 네
`actionreference.json`의 `actions[]` 순서와 `displayName`을 그대로 사용한다.

```text
Data/Animation/Reference/KoukuSaydon/MN_RPCT_05.actionreference.json
Data/Animation/Reference/KoukuSaydon/MN_RPCT_06.actionreference.json
Data/Animation/Reference/KoukuSaydon/MN_RPCT_07.actionreference.json
Data/Animation/Reference/KoukuSaydon/MN_RPCZ_00.actionreference.json
```

현재 네 프로필은 기획 액션 349개, stage 4,072개, 실제 WModel slot 3,692개를 보유한다.
`MN_RPCT_07`은 독립된 기획 액션 목록이지만 preview body는 `MN_RPCT_05`를 공유한다. 같은
`displayName`이 중복될 수 있으므로 목록과 저장 join에서는 `sourceActionId`, `stageId`, `slotId`를
항상 함께 유지한다. `HOLDOUT` 액션도 원본 순서로 표시하지만 선택 검토만 가능하고 전체 액션
preview와 Pattern 생성·저장은 거부한다.

### 17.2 Animation Tool 작업 흐름

1. F1 → Animation Tool에서 `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCT_07`, `MN_RPCZ_00` 중 프로필을 연다.
2. `Planner Actions`에서 기획 이름 또는 `sourceActionId`로 검색하고 액션을 선택한다.
3. `Selected Action Clips`에서 모든 stage/slot occurrence를 원본 순서로 확인한다. 행을 선택하면
   해당 실제 WModel clip을 단일 preview한다.
4. `Preview Action`은 선택 액션의 유효 occurrence를 source timing 순서로 연속 재생한다.
5. `Create Pattern`은 같은 occurrence를 별도 `REFERENCE_ONLY` 패턴 draft로 복사한다.
6. `Created Local Patterns`에서 패턴을 선택해 전체 재생하거나 Pattern을 Duplicate/Delete한다.
7. 선택 Pattern의 clip은 단일 window preview, Duplicate/Delete, Move Up/Down을 지원한다. 마지막
   clip 하나는 삭제하지 않고 Pattern 자체를 삭제한다.
8. `Save Patterns`로 현재 프로필의 `*.patternbindings.json`을 저장한다.

preview는 현재 animation target generation에 고정된다. 다른 target/profile로 바꾸거나 Tool을
닫으면 원래 body를 idle, 1배속, loop 상태로 복원한다. 각 occurrence는 `sourceStartMs`, `playMs`,
`playRate`와 `EXACT`, `HOLD_LAST_POSE`, `LOOP_TO_WINDOW` 중 하나의 명시적 end policy를 사용한다.

### 17.3 저장과 권위 경계

로컬 Pattern 정본은 다음 경로다.

```text
Data/Animation/Reference/KoukuSaydon/<Profile>.actionreference.json
schema: lostark.kouku-saydon-animation-action-reference
Data/Animation/Authored/KoukuSaydon/<Profile>.actionbindings.json
schema: lostark.kouku-saydon-animation-action-bindings
Data/Animation/Authored/KoukuSaydon/<Profile>.patternbindings.json
schema: lostark.kouku-saydon-animation-pattern-bindings
formatVersion: 1
authority: REFERENCE_ONLY
```

Pattern ID는 `kakulsaydon.<Profile>.pattern.<N>`, clip occurrence ID는
`<PatternId>.clip.<N>`이며 monotonic ordinal로 발급하고 삭제 뒤 재사용하지 않는다. Load는
`parse -> validate -> stage -> commit`, Save는 `validate -> sibling temp durable write -> strict
reparse/exact compare -> reference revision 재확인 -> atomic replace`를 따른다. 잘못된 profile,
reference revision, HOLDOUT source, 중복 ID, source tuple 불일치, model에 없는 clip, 잘못된 timing/end
policy가 있으면 현재 admitted draft와 destination을 유지한다.

이 문서는 쿠크·세이튼 Server 전투 Pattern이나 Product gameplay/presentation 정본이 아니다.
Animation Tool의 `Create Pattern`은 로컬 동작 조합과 검토만 제공하며 Server 실행, damage, collider,
Effect cue를 만들거나 publish하지 않는다. 제품 Pattern 승격은 별도 Data → Shared → Server → Client
수직 슬라이스와 publisher/harness가 생긴 뒤에만 진행한다.

`Data/Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json`도 이 경계를 바꾸지 않는
`REFERENCE_ONLY` manifest다. 네 profile의 reference/action/pattern-binding owner를
`sourceActionId + stageId + slotId + referenceRevision` identity와 coverage로 검증할 뿐 boss/encounter
ID, gameplay Pattern, collider, Effect 또는 Sound를 생성하지 않는다.
`Data/Compositions/Sequences/KoukuSaydonArena.sequencer.json`은
현재 authored world/camera track inspection만 제공하며 unified Save/Play나 Server scene runtime이 아니다.


### 17.4 Composition의 모델별 Pattern과 Sequence 저장

Composition의 `KoukuSaydon` 아래 Pattern은 `Kouku (MN_RPCZ_00)`,
`Saydon (MN_RPCT_05)`, `Large Saydon (MN_RPCT_06)` 모델별로 나눈다.
`MN_RPCT_07` 액션은 실제 몸체가 같은 Saydon Pattern에 넣으며 occurrence의 원래
`profileId`는 `MN_RPCT_07`로 보존한다. 다른 몸체의 Pattern에는 붙이지 않는다.

Resources의 기획 Action을 Sequence에 붙이면 원본 slot의 clip, source timing과 순서를
복사한다. 모델에 Pattern이 없으면 첫 Append가 같은 모델의 DRAFT Pattern을 만든다.
Stage/animation box는 클릭, Ctrl+클릭, 빈 공간 드래그로 선택하고 `Delete Selected` 또는
Delete로 제거한다. Sequence의 `Save`는 삭제를 포함한 현재 Composition 문서를 저장한다.
입력 또는 저장 검증이 실패하면 기존 Pattern과 파일을 보존한다.

정본은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`이다. formatVersion 2의
각 Pattern은 물리 모델 ID인 `actorProfileId`를 명시한다. version 1은 읽을 때 occurrence의
모델로 owner를 유도하며, 빈 기존 Gate 1 Pattern은 `MN_RPCZ_00`으로 옮긴다. 서로 다른
모델이 섞였거나 모르는 프로필이 있는 기존 Pattern은 오류 항목으로 격리하고 원문을 보존한다.
저장은 version 2로 수행한다. 별도의 `*.patternbindings.json` 로컬 Pattern 저장과는 다른
Composition 저작 문서다.

`sourceActionId = 0`도 유효한 기획 액션이다. 물리 RAW clip은 `sourceActionId = 0`과
`sourceStageId = RAW`의 조합으로 구별한다. 참조 파일의 누락이나 최신 여부는 Composition
Save를 막지 않으며 저장된 source ID와 revision 문자열을 보존한다.

DRAFT Pattern은 원본의 긴 기획 Action을 담을 수 있도록 최대 1,024 Stage를 허용하며
Pattern 전체 600초 제한은 유지한다. PRODUCT는 기존 64 Stage 제한을 유지한다.

`PRODUCT` 투영과 Server 재생은 현재 Gate 1 몸체 `MN_RPCZ_00` Pattern만 지원한다.
Saydon과 Large Saydon의 DRAFT 저장이 Gate 1 boss를 다른 모델로 바꾸지는 않는다.
기존 publisher의 runtime timing 제한과 검증 후 명시적 배포 절차를 유지한다.

대형 이름 Action의 로컬 preview는 요청 배율 100배를 사용하고 일반 동작/정지 시 기준 크기로
복원한다. 원본 Unreal Actor 배율로 검증된 값은 아니다. Large Saydon의 오른손 `b_wp_1`에는
기존 `Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel`을 부착한다.
무기 자체 skeleton과 대응 clip을 source seconds로 동기화하고, 대응이 없으면 bind pose를 쓴다.
필요한 물리 폴더는 `Client/Bin/Resources/Character/KoukuSaton/WP_MN_RPCT_06`이며
모델과 인접 texture는 팀 Drive Resources 입력으로 전달한다. preview는 Server boss를 바꾸지 않는다.

# Animation Tool 담당자 인계와 Tool 경계

이 문서는 Animation 담당자가 오늘 바로 작업할 범위와 Character Preview, Effect Tool, Server gameplay
사이의 금지 경계를 고정한다. 세부 구현 전체 코드는 날짜별 PLAN에 두고, 이 문서는 담당 인터페이스만 소유한다.

## 1. 한 줄 계약

Animation Tool은 `어떤 animation asset의 어떤 clip에서 언제 cue가 발생하는가`를 저작한다.
Effect Tool은 `그 cue가 호출할 EffectAssetId의 모양과 부착 방식`을 저작한다. Character Preview Panel은
두 Tool이 함께 보는 preview target과 매 frame anchor transform을 소유한다.

```text
Character Preview Panel
  preview target + model + root/weapon/bone anchor snapshot
          │
          ├─> Animation Tool: clip/playhead/event timing
          │                      │
          │                      └─> effectAssetId binding
          │
          └─> Effect Tool: emitter/module/trail/local transform/anchor policy
                                 │
                                 └─> same CEffect_Runtime preview
```

## 2. 현재 코드와 목표 상태를 구분한다

현재 `CAnimation_Tool` 안에는 `Render_TargetSelector`, `Select_PreviewAsset`, `Release_Preview`,
`Refresh_PreviewLevel`과 `CPart_Body` preview 생성·제거가 들어 있다. 이것은 아직 공용 Character Preview
Panel로 이동하지 않은 현재 상태다. 이동이 끝날 때까지 Animation 담당자는 이 코드에 신규 preview 기능을
추가하지 않고 보존만 한다.

현재 `.animevents` Save는 destination을 직접 `w` mode로 열며, Reload는 Dirty 확인이 없다. Load는 임시
vector에 읽은 뒤 교체하지만 owner/count와 malformed row를 엄격히 거부하지 않는다. 오늘 Animation 담당자의
실제 수정 대상은 이 document safety와, 아직 닫힌 상태로 선언할 EffectAssetId binding 계약이다.

현재 `Data/Effects/Authored/DimensionMaster/Candidates`는 `candidate_only`다. 이 후보는 admitted effect 선택
목록이나 제품 runtime cue에 자동 연결하지 않는다.

## 3. Animation Tool이 소유하는 것

Animation 담당자는 다음을 계속 소유한다.

- animation asset과 clip 목록 표시
- Play/Pause/Step/Scrub/Loop와 clip local playhead
- clip chain, skill reference, notify reference의 읽기 전용 표시
- `.animevents`의 point/window marker 편집
- Effect Tool typed request로 stable `effectAssetId` point marker 생성
- `.animevents`의 parse → validate → stage → commit Save/Load/Reload
- Dirty 상태, 선택 event, 실패 메시지와 기존 document 보존

Animation Tool의 저장 row는 다음 의미를 가진다.

```text
animationAssetId  파일 header owner. 어떤 class/preview asset의 timeline인지 식별한다.
clipName          해당 asset 안의 stable clip 이름이다.
startMs           clip 시작 기준 cue 시작 시각이다.
endMs             window cue의 종료 시각이다. point cue는 startMs와 같다.
eventKind         HIT/CANCEL/SUPERARMOR/INVULN/MOVE/SOUND/EFFECT/SHAKE다.
effectAssetId     authored EFFECT가 호출할 stable effect catalog ID다.
```

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
- Effect를 직접 spawn하거나 `CEffect_Runtime`을 직접 update/render하는 코드
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
- anchor policy와 anchor-relative local position/rotation/scale
- admitted effect catalog와 dependency/validation 상태
- authoring Save/Load/Reload와 runtime cook
- 동일 `CEffect_Runtime`을 통한 preview

Effect Tool은 Animation clip과 marker vector를 직접 편집하지 않는다. `Use Selected Effect`는 선택한
`EffectAssetId`를 typed request로 제출할 뿐이다. Animation Tool에서 marker가 만들어지고 Save되기 전에는
binding이 영구 저장됐다고 표시하지 않는다.

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

Effect asset에는 실제 world coordinate나 현재 마우스 좌표를 저장하지 않고 다음 정책을 저장한다.

| anchor kind | 저장 의미 | preview/runtime 입력 |
|---|---|---|
| `CHARACTER_ROOT` | character root를 따라간다 | current root transform |
| `WEAPON_SOCKET` | stable weapon anchor를 따라간다 | `anchorSlotId`의 current transform |
| `MODEL_BONE` | 검증된 skeleton anchor를 따라간다 | stable bone/socket ID의 current transform |
| `WORLD_POINT` | spawn 시점 world point에 남는다 | cue context world position |
| `MOUSE_GROUND` | spawn 시 커서 ground point를 사용한다 | picking 결과의 current world point |

Effect Tool은 `anchorKind + anchorSlotId + localPosition + localRotation + localScale`을 편집한다. Preview
Panel은 그 anchor의 current world transform을 계산한다. Effect runtime은 둘을 합성한다.

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
-> Animation Tool이 EFFECT_ASSET_ID point marker 한 건 생성, Dirty
-> atomic Save
-> staged Reload
-> 같은 asset/clip/ms/EffectAssetId 복원
```

### 10.2 Weapon Trail 저작

```text
Preview Panel이 weapon anchor를 매 frame 제공
-> Animation Tool이 trail start/end window 저작
-> Effect Tool이 WEAPON_SOCKET + stable anchorSlotId 선택
-> ribbon/trail width/lifetime/sampling/material 편집
-> CEffect_Runtime preview가 매 frame anchor를 sample
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
  ProjectAudit, `git diff --check`.
- 수동: 다섯 class target, scrub 시각, Dirty target 전환 보존과 Level 전환 cleanup.
- 미완료로 남길 것: admitted Effect fixture/catalog resolver, 실제 Effect Tool `Use Selected Effect` 성공 경로,
  root/weapon anchor preview, Server collider/damage publisher, EFFECT window/trail start-stop, beam dual anchor,
  candidate-only DimensionMaster effect admission.

현재 admitted Effect는 0개이며 459개는 전부 DimensionMaster `candidate_only`다. 따라서 오늘 Animation 문서
안전성 작업을 EffectAssetId 성공 Save나 preview로 검증하지 않는다. `EFFECT_ASSET_ID` 입력은 admission
resolver가 없거나 ID가 catalog에 없으면 실패하고 memory/destination을 유지해야 한다. 다음 Effect/Preview
설계 단계에서 의존성 검증을 통과한 fixture 1개와 catalog resolver를 먼저 만든 뒤 실제
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
| Client Runtime | `Client/Bin/Resources/Character` | `.wmodel`과 그 모델이 참조하는 texture | Resources payload는 금지 |
| Runtime 등록 정본 | `Data/Actors/CharacterCatalog.json` | stable class ID와 Resources-relative model asset ID | 허용 |
| Animation 저작 정본 | `Data/Animation/Authored/<AssetName>/<AssetName>.animevents` | 팀이 편집하는 animation event | 허용 |
| Animation 원본 참조 | `Data/Animation/Reference/<AssetName>` | 추출된 notify/clip/skill timing reference | 허용 |
| 팀 리소스 배포 | immutable Resources ZIP + `Data/AssetPacks.lock.json` | 다른 PC의 Hydrate/Verify 입력 | lock/manifest만 허용 |

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

Character Select가 preview `CCharacter`를 교체할 때 `CAnimationTargetService::Bind`를 호출한다. Animation
Tool에서 `Scene Character`를 선택하면 다음 경로를 사용한다.

```text
CLevel_CharacterSelect preview CCharacter
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
-> immutable Resources pack Hydrate/Verify
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

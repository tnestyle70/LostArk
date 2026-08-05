# Animation Tool과 Effect Tool 연결 계획 — 쉬운 설명

## 1. 이 문서의 결론

우리가 만들려는 작업 흐름은 아래 한 줄이다.

~~~text
캐릭터를 고른다
→ 애니메이션을 재생하고 원하는 시각을 찾는다
→ 그 시각에 재생할 이펙트를 고른다
→ 무기, 캐릭터 중심, 바닥 중 어디에서 나올지 정한다
→ 저장하고 다시 열어도 같은 결과가 나온다
~~~

이 작업을 한 Tool에 전부 넣지 않는다.

- **Character Preview Panel**은 어떤 캐릭터와 무기를 보고 있는지 관리한다.
- **Animation Tool**은 애니메이션의 몇 ms에 무엇이 발생하는지 관리한다.
- **Effect Tool**은 이펙트의 모양, effect-local 수명, 재질과 파티클을 관리한다.
- **Animation Tool**의 Effect cue는 부착 anchor, offset, follow/stop policy를 관리한다.
- **Server**는 실제 충돌과 데미지를 판정한다.

Animation Tool이 Effect를 직접 만들거나, Effect Tool이 Animation 파일을 직접 수정하지 않는다.

---

## 2. 지금 실제로 되는 것

### 2.1 Character Select

현재 Character Select에서는 다음 다섯 클래스를 선택할 수 있다.

- Lance Master
- Gunslinger
- Slayer
- Artist
- DimensionMaster

캐릭터를 바꾸면 Animation Tool의 **Scene Character** 대상도 선택한 캐릭터로 바뀐다.

### 2.2 Animation Tool

현재 Animation Tool에서 가능한 작업은 다음과 같다.

- 현재 Scene Character 선택
- 차원술사 전용 preview model 선택
- animation clip 목록 확인
- Play, Pause, Step, Scrub, Loop
- clip chain과 reference 확인
- HIT, CANCEL, SUPERARMOR, INVULN, MOVE 이벤트 생성
- SOUND, EFFECT 이벤트 생성
- 기존 게임 notify 가져오기
- .animevents Save
- .animevents Reload
- 선택 이벤트 삭제와 시간 수정

기존 Animation Tool의 선택·재생·Effect 이벤트·저장 기능은 유지한다.

이번 수정으로 추가된 안전장치는 다음과 같다.

- Save는 기존 파일을 먼저 지우지 않고 임시 파일을 완성한 뒤 교체한다.
- Reload 전에 편집 중인 내용이 있으면 확인한다.
- 잘못된 파일을 읽어도 현재 화면의 이벤트를 먼저 지우지 않는다.
- v3 파일을 읽기만 했다는 이유로 Dirty가 되지 않는다.
- 실제 편집이 생겼을 때만 Dirty가 된다.
- 캐릭터가 바뀌면 오래된 target을 구분할 수 있도록 generation이 증가한다.

### 2.3 차원술사 Animation 대상

Animation Tool의 Target 목록에는 다음 항목이 있다.

| 항목 | 용도 |
|---|---|
| Scene Character | Character Select에서 현재 선택한 실제 캐릭터 |
| DimensionMaster Character | 차원술사 본체, 약 154개 clip |
| Dimension Core | sk_super_instance core model |
| Dimension Summon | 소환체 model, 2개 clip |

차원술사 본체, Core, Summon은 같은 Animation Tool에서 확인할 수 있다.

다만 현재 차원술사는 다음 범위까지만 닫혀 있다.

- Character Select
- 본체 model
- IDLE/RUN
- Animation Tool preview
- 빈 .animevents container와 reference container

차원술사 Q/W 스킬과 실제 Effect binding은 아직 완료되지 않았다.

### 2.4 Effect Tool 재구축 G1

기존 `Effect_Tool.h/.cpp`의 emitter/module 편집, CPU canvas, world preview, HDR readback,
source catalog UI는 제거했다. UI만 소비하던 `Effect_ResourceCatalog.h/.cpp`도 삭제했다.
이어 구형 `CEffect_Runtime`, `Effect_Types`, `Effect_AssetIO`, `Effect_ParticleSimulator`, Effect shader,
resource intake pipeline과 `Data/Effects/Authored`의 459개 `.effect` 문서도 삭제했다.

새 G1 Tool은 ImGui에서 `Mesh / Sprite / Particle / Decal / Trail` 중 하나를 고르고, stable ID와 표시 이름을
검증해 메모리 `EffectDocument` 하나를 생성·폐기한다. 선택한 Element 종류는 session 상태일 뿐이며 G2의
`Add Element` 전에는 Document를 변경하지 않는다. 파일 저장, GPU preview, Animation binding은 아직 없다.

DimensionMaster로 rename된 구형 `.effect` 후보 459개도 재구축 삭제 의도에 따라 모두 제거했다. 원본 조사
증거인 `SourceCatalog`/`SourceExtracted`와 `Client/Bin/Resources/Effect` payload는 보존하지만, 새 schema와
dependency 검증을 통과한 admitted Effect는 아직 0개다.

---

## 3. 아직 구현되지 않은 것

다음 항목은 문서로 경계만 정했으며 아직 실제 연결은 끝나지 않았다.

- Animation Tool 밖의 공용 Character Preview Panel
- Effect Tool의 Load/Edit/Save/Cook/Preview UI
- admitted Effect catalog와 resolver
- Use Selected Effect 버튼
- Animation Tool과 Effect Tool 사이의 실제 binding request
- root, weapon, bone, mouse-ground anchor 편집
- weapon trail start/stop
- animation EFFECT marker를 단일 Effect runtime으로 보내는 제품 consumer
- Animation HIT marker를 Server collision/damage로 publish하는 경로

따라서 현재 상태를 “Animation과 Effect가 완전히 연결됐다”고 말하면 안 된다.

현재 완료된 것은 Animation 쪽 저장 안전성, Effect Tool이 읽을 수 있는 playhead 정보 경계와
Effect type selection-only G0다.

---

## 4. 세 부분을 왜 나누는가

### 4.1 Character Preview Panel

Preview Panel은 화면에 보이는 대상을 소유한다.

- 어떤 class인가
- 어떤 body model인가
- 어떤 weapon인가
- 현재 root transform은 무엇인가
- weapon anchor가 현재 어디에 있는가
- 마우스로 찍은 바닥 위치가 어디인가
- target이 교체됐는가

Preview Panel이 캐릭터를 만들고 제거한다.

Animation Tool과 Effect Tool은 캐릭터를 각각 따로 생성하지 않는다.

### 4.2 Animation Tool

Animation Tool은 “언제”를 소유한다.

예를 들면 다음과 같다.

~~~text
sk_skill_attack_01
  180ms  EFFECT
  240ms  HIT 시작
  300ms  HIT 종료
  420ms  SOUND
~~~

Animation Tool은 clip과 시간을 저장한다.

- animationAssetId
- clipName
- startMs
- endMs
- eventKind
- payload

Animation Tool은 particle 개수, texture, material, trail 폭을 편집하지 않는다.

### 4.3 Effect Tool

Effect Tool은 “무엇을, 어떤 모양과 내부 수명으로 보여줄지”를 소유한다.

- EffectAssetId
- emitter
- module
- texture
- material
- color
- lifetime
- velocity
- SubUV
- ribbon/trail
- local position, rotation, scale
- effect root와 emitter의 effect-local transform

Effect Tool은 animation clip 목록이나 .animevents vector를 직접 수정하지 않는다.

### 4.4 Server

Server는 실제 게임 결과를 소유한다.

- collider 활성 시점
- 명중 여부
- 대상 선택
- damage
- boss HP
- player HP
- skill cooldown

Animation Tool의 HIT marker는 현재 비교와 저작을 위한 정보다.

HIT marker를 저장했다고 Server damage timing이 자동으로 바뀌는 것은 아니다.

---

## 5. Animation Tool 화면을 어떻게 사용하면 되는가

### Target

현재 Animation Tool이 재생할 model을 고른다.

- Scene Character: Character Select에서 선택한 캐릭터
- DimensionMaster Character/Core/Summon: 별도 차원술사 preview

### Playback

선택한 animation을 재생하거나 멈추고 원하는 위치로 이동한다.

- Play/Pause
- 한 frame 이동
- timeline scrub
- Loop

### Chain

한 스킬이 여러 animation clip으로 이어질 때 순서를 보여준다.

이 정보는 reference이며 Animation Tool이 원본 chain 파일을 수정하지 않는다.

### Events

현재 clip의 특정 시각에 이벤트를 추가한다.

- Hit: 공격 판정 참고 구간
- Cancel: 입력 취소 가능 구간
- SuperArmor: 경직 면역 구간
- Invuln: 무적 구간
- Move: 이동 구간
- Sound: 사운드 재생 시각
- Effect: 이펙트 재생 시각

현재 Effect 버튼은 기존 방식대로 사용할 수 있다.

지금 입력하는 payload는 기존 source cue/reference다.
새 Effect Tool의 stable EffectAssetId를 연결하는 기능은 admitted catalog가 생긴 뒤 추가한다.

### Import original

추출된 원본 notify를 현재 clip에 가져온다.

가져온 row는 src=orig로 표시한다. 다시 import하면 이전 import row만 교체하고 사용자가 직접 만든 row는 유지한다.

### Save

현재 .animevents를 저장한다.

새 Save 흐름은 다음과 같다.

~~~text
현재 이벤트 검사
→ 같은 폴더에 임시 파일 작성
→ 임시 파일 flush
→ 임시 파일을 다시 읽어 같은 내용인지 검사
→ 성공했을 때만 기존 파일 교체
~~~

중간에 실패하면 기존 파일을 유지한다.

### Reload

디스크의 .animevents를 다시 읽는다.

편집 중인 내용이 없으면 바로 읽는다.
편집 중이면 Discard and Reload 또는 Cancel을 고른다.

파일이 잘못됐으면 현재 화면의 이벤트를 유지한다.

---

## 6. Animation 데이터는 무엇을 저장하는가

### 6.1 저장 위치

~~~text
Data/Animation/Authored/<AnimationAssetId>/<AnimationAssetId>.animevents
~~~

예:

~~~text
Data/Animation/Authored/Artist/Artist.animevents
Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents
~~~

### 6.2 기존 v3

기존 파일은 v3이며 animation 시간을 millisecond로 저장한다.

~~~text
LOSTARK_ANIM_EVENTS 3 "Artist" 2
"skill_attack_01" HIT startms=240 endms=300 rep=1 repms=0 ...
"skill_attack_01" EFFECT startms=180 payload="original_source_cue" src=orig
~~~

v3 파일은 그대로 읽을 수 있다.

v3를 읽었다는 이유만으로 Dirty가 되지 않는다. 실제로 이벤트를 편집하고 Save할 때 v4로 저장한다.

### 6.3 v4에서 추가된 구분

기존 source cue와 새 EffectAssetId를 구분하기 위해 effectref를 추가한다.

기존 source reference:

~~~text
"skill_attack_01" EFFECT startms=180 payload="original_source_cue" effectref=source src=orig
~~~

향후 admitted EffectAssetId binding:

~~~text
"skill_attack_01" EFFECT startms=180 payload="Effect/Artist/skill_attack_01" effectref=asset-id
~~~

두 값은 의미가 다르다.

| 값 | 의미 |
|---|---|
| effectref=source | 원본 게임 데이터에서 추출한 cue 이름 또는 기존 수동 reference |
| effectref=asset-id | Effect catalog가 승인한 실제 EffectAssetId |

현재는 admitted Effect가 없으므로 asset-id row를 Load/Save에서 거부한다.

이것은 기존 Effect 이벤트를 막기 위한 것이 아니다. 잘못된 candidate를 제품 Effect처럼 연결하지 않기 위한 임시 gate다.

---

## 7. Effect 데이터는 무엇을 저장하는가

### 7.1 현재 포맷

현재 G0에는 Effect authoring 파일 포맷이 없다. 아래 정보는 타입 selector 다음 단계에서 새 schema를
정의할 때 함께 결정하고, 과거 `.effect v5`를 암묵적으로 복원하지 않는다.

향후 Effect asset이 소유할 정보는 다음과 같다.

- emitter 종류
- emitter delay와 duration
- particle spawn
- texture
- mesh
- material
- lifetime
- size
- color
- velocity
- SubUV
- beam
- ribbon/trail

Animation timing은 .effect 파일에 넣지 않는다.

### 7.2 Animation cue에 다음 단계에서 추가할 부착 정보

다음 Animation/Preview 단계에서는 같은 EffectAssetId가 cue마다 어느 anchor를 따라갈지 저장할 계약이 필요하다.

형태는 아래 의미를 가져야 한다.

~~~text
ATTACH
  kind=WEAPON_SOCKET
  slot="weapon.main"
  localPosition=0,0,0
  localRotation=0,0,0
  localScale=1,1,1
~~~

지원할 anchor 의미는 다음과 같다.

| anchor | 의미 |
|---|---|
| CHARACTER_ROOT | 캐릭터 중심을 따라간다 |
| WEAPON_SOCKET | 현재 weapon anchor를 따라간다 |
| MODEL_BONE | 검증된 skeleton anchor를 따라간다 |
| WORLD_POINT | 생성 시점의 월드 위치에 남는다 |
| MOUSE_GROUND | 스킬 사용 시 선택한 바닥 위치를 사용한다 |

MOUSE_GROUND에는 Tool에서 찍은 실제 좌표를 저장하지 않는다.
“실행할 때 마우스 바닥 위치를 사용한다”는 정책만 저장한다.

raw bone index나 C++ pointer도 저장하지 않는다. stable anchorSlotId를 저장한다.

---

## 8. Animation과 Effect가 실제로 연결되는 흐름

### 8.1 현재 가능한 흐름

~~~text
Character Select에서 class 선택
→ Animation Tool에서 Scene Character 선택
→ clip 선택
→ play/pause/scrub
→ HIT/SOUND/EFFECT source marker 작성
→ Save
→ Reload
~~~

이 흐름은 현재 사용할 수 있다.

### 8.2 다음 단계의 목표 흐름

~~~text
Character Preview Panel에서 캐릭터 선택
→ Animation Tool에서 clip과 시각 선택
→ Effect Tool에서 admitted EffectAssetId 선택
→ Effect Tool에서 preview
→ Use Selected Effect
→ Animation Tool이 해당 시각에 asset-id marker 한 건 생성
→ Animation Tool Save
→ Reload 후 같은 clip/time/effectAssetId 복원
~~~

Effect Tool은 Animation 파일을 직접 열지 않는다.

Effect Tool이 보내는 값은 아래 정도면 충분하다.

~~~text
targetGeneration
animationAssetId
clipName
timeMs
effectAssetId
~~~

Animation Tool만 이 요청을 받아 .animevents를 수정한다.

---

## 9. Burst, Trail, Collider, Damage는 서로 다르다

### Burst Effect

한 시각에 한 번 생성한다.

~~~text
180ms EFFECT
~~~

Animation Tool에서는 point event다.

### Weapon Trail

시작과 종료가 필요하다.

~~~text
180ms TRAIL START
420ms TRAIL END
~~~

Animation Tool은 시작/종료 시간을 저장한다.
Effect Tool은 trail 폭, lifetime, sampling, texture를 저장한다.
Preview Panel은 매 frame weapon anchor 위치를 제공한다.

현재 EFFECT는 point event이므로 Trail window와 start/stop runtime은 후속 구현이다.

### Collider

Animation Tool에서 HIT window를 볼 수는 있다.

하지만 실제 collider 활성화는 Server skill/collision system이 결정해야 한다.

### Damage

Animation Tool에서 damage 숫자를 계산하지 않는다.

실제 정본은 다음 데이터와 Server다.

~~~text
Data/Balance/PlayerSkills.json
Data/Balance/DamageProfiles.json
Server 30Hz gameplay tick
~~~

Animation HIT marker를 Server에 반영하려면 publisher와 Server harness가 별도로 필요하다.

---

## 10. 실제 코드에서 추가된 최소 경계

### Animation 쪽 구현

| 파일 | 현재 역할 |
|---|---|
| Client/Public/Animation_Tool.h | event kind, source/asset-id 구분, Save/Load 상태 |
| Client/Private/Animation_Tool.cpp | 기존 UI 유지, staged Load, atomic Save, Dirty Reload |
| Client/Public/AnimationTargetService.h | 현재 target과 generation 조회 |
| Client/Private/AnimationTargetService.cpp | target이 교체될 때 generation 증가 |
| Client/Public/AnimationAuthoringBridge.h | Effect/Preview가 읽을 playhead snapshot |
| Client/Private/AnimationAuthoringBridge.cpp | asset, clip, time, duration, pause 상태 계산 |
| Client/Default/Client.vcxproj | 새 H/CPP 빌드 등록 |
| Client/Default/Client.vcxproj.filters | Visual Studio Animation Tool 필터 등록 |

읽기 전용 playhead snapshot은 다음 값을 제공한다.

~~~text
targetGeneration
animationAssetId
clipName
timeMs
durationMs
isPaused
~~~

Effect Tool이 CModel 내부를 직접 뒤지지 않도록 만든 경계다.

### 이번 변경에서 건드리지 않은 것

- Effect_Tool.h/.cpp
- Effect_Runtime.h/.cpp
- Effect emitter/module 구조
- 새로운 Character Preview Panel
- Server damage 코드
- Balance JSON

Preview와 Effect 구현은 이 최소 Animation 경계 위에서 다음 단계로 진행한다.

---

## 11. 앞으로 구현할 순서

### 1단계 — Animation 기존 기능 유지

- 다섯 class 선택
- 차원술사 세 preview target
- clip 목록
- playback
- event 생성
- Save/Reload

현재 여기까지 동작한다.

### 2단계 — 공용 Character Preview Panel

- Animation Tool 안의 target 생성 코드를 공용 panel로 이동
- body와 weapon을 한 곳에서 생성
- root/weapon/bone transform 제공
- target 제거와 level 전환 cleanup
- generation snapshot 제공

이동이 완료되기 전에는 기존 Animation preview 코드를 먼저 삭제하지 않는다.

### 3단계 — admitted Effect 하나 만들기

- 보존한 원본 추출 증거에서 조사 대상 하나 선택
- 새 authoring schema와 stable ID 규칙 정의
- texture/material/model dependency 검사
- 새 단일 runtime preview 검사
- stable EffectAssetId 부여
- admitted catalog에 등록

이 한 개가 있어야 실제 binding 성공 경로를 검증할 수 있다.

### 4단계 — 새 Effect Tool

- 캐릭터와 animation playhead 표시
- Effect 선택
- emitter/module 편집
- Animation cue의 root/weapon/world/mouse anchor 선택
- local transform 편집
- 새 단일 Effect runtime preview

### 5단계 — Use Selected Effect

- Effect Tool이 typed request 제출
- Animation Tool이 marker 한 건 생성
- Animation Tool이 Dirty가 됨
- Save/Reload parity 검증

### 6단계 — Trail과 Server timing

- EFFECT window
- trail start/stop
- weapon anchor sampling
- HIT publisher
- Server collision/damage harness

---

## 12. 완료 기준

### Animation 최소 경계 완료

- 기존 class에서 clip과 이벤트가 다시 보인다.
- 차원술사 Character/Core/Summon을 선택할 수 있다.
- 기존 Effect 버튼을 사용할 수 있다.
- v3 파일을 정상적으로 읽는다.
- v3 로드만으로 Dirty가 되지 않는다.
- Save 실패 시 기존 파일을 보존한다.
- Dirty Reload는 확인을 요구한다.
- Debug/Release Client가 빌드된다.

### 전체 Animation ↔ Effect 연결 완료

- Preview Panel에서 캐릭터와 weapon을 선택한다.
- Animation Tool에서 clip과 시간을 고른다.
- Effect Tool에서 admitted Effect를 preview한다.
- Animation cue에서 root/weapon/mouse anchor를 고른다.
- Use Selected Effect로 marker를 만든다.
- Save 후 Reload해도 같은 값이 복원된다.
- runtime에서 같은 animation 시각과 anchor에서 Effect가 나온다.
- target이나 level을 바꿔도 고아 preview가 남지 않는다.

---

## 13. 현재 검증 상태

확인된 항목:

- Client x64 Debug 빌드 성공
- Client x64 Release 빌드 성공
- Debug Client 시작 smoke 성공
- 기존 Animation 선택·clip·event 화면 수동 확인
- 차원술사 Character/Core/Summon Target 등록
- 기존 Effect 버튼 복구

ProjectAudit는 Animation 코드가 아니라 현재 Resource pack lock과 실제 Resources inventory 불일치 때문에 실패한다.

~~~text
asset-lock.inventory
actual files=10179
actual bytes=5405091440
~~~

이 Resource pack 문제는 Animation/Effect Tool 경계와 별도로 정리해야 한다.

---

## 14. 담당자에게 짧게 전달할 내용

### Animation 담당자

> 기존 Playback, Chain, Events, Reference, Clip 기능을 유지합니다. .animevents는 Animation Tool만
> 수정합니다. 기존 source EFFECT는 계속 편집할 수 있습니다. 새 stable EffectAssetId binding은 admitted
> catalog가 생긴 다음 typed request로 받습니다. Effect runtime과 Character 생성 코드는 Animation Tool에
> 추가하지 않습니다.

### Effect 담당자

> G0에서는 Effect 타입 하나만 선택합니다. 다음 단계부터 새 schema로 emitter/module을 한 계약씩 열고,
> 하나의 preview/runtime 경로를 함께 만듭니다. Animation clip이나 .animevents를 직접 수정하지 않으며,
> 향후 admitted EffectAssetId 선택 요청만 Animation 쪽으로 보냅니다. anchor/offset/follow/stop 설정은
> Animation cue가 소유합니다.

### Preview 담당자

> Character, Body, Weapon의 생성·제거와 root/weapon/bone transform을 소유합니다. Animation과 Effect Tool에
> read-only target snapshot을 제공합니다. Tool마다 별도 캐릭터를 만들지 않습니다.

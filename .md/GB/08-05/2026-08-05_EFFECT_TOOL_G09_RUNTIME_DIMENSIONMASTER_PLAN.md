# Effect Tool G09 Effect admission·runtime·DimensionMaster 스킬 계획

## G09-00. 이 문서의 기준

- 이전 단계: [G08 Particle·Trail·AfterImage와 실제 렌더링](2026-08-05_EFFECT_TOOL_G08_PARTICLE_TRAIL_RENDERING_PLAN.md)
- 코드 설명 형식 정본: [G06 Shader와 남은 반영 가이드](2026-08-05_EFFECT_TOOL_G06_SHADER_REMAINING_GUIDE.md)
- Animation 경계: [Animation Tool 담당자 인계와 Tool 경계](../../TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md)
- 데이터 경계: [통합 데이터 관리 구조](../../TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md)
- 현재 clip 정본: `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
- 현재 skill 정본: `Data/Balance/PlayerSkills.json`

G09의 본질은 Tool에서 잘 보이는 Effect를 제품에서 새로 구현하는 것이 아니다. G07~G08에서 레퍼런스
화면과 같은 제작 흐름으로 완성한 `CEffectObject + CEffect_Playback + CEffect_DocumentRenderer`를
admitted `EffectAssetId`로 찾아 Server가 승인한 Character action presentation에서 spawn하는 것이다.
따라서 이 문서는 단순 runtime 연결 계획이 아니라 G07~G09 전체 Effect Tool의 최종 합격 지점이다.

```text
Data/Effects/Authored v5
→ EffectCatalog admission
→ Publish-Effects.ps1
→ Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json
→ CEffectCatalog parse/validate/stage/commit
→ 승인된 Character action / Animation EFFECT cue
→ CEffectPresentationService
→ 기존 CEffectObject
```

## G09-01. 종료 증거

G09 완료는 다음이 모두 실제로 연결된 상태다.

1. Effect Tool에서 저장한 v5 문서 중 catalog의 `ADMITTED` entry만 publish된다.
2. 잘못된 version/ID/path/resource/hash/중복이 하나라도 있으면 기존 runtime catalog를 보존한다.
3. Animation Tool에서 source reference와 admitted EffectAssetId가 명확히 구분된다.
4. local 입력 즉시가 아니라 Server snapshot의 새 action edge에서 Effect가 시작된다.
5. local/remote DimensionMaster 모두 같은 skillId, combo stage, animation cue로 Effect를 본다.
6. Q/W/E/R/A/S/D/F/T/V/LMB 11개 binding이 비어 있지 않고 runtime에서 실제 표시된다.
7. effect 문서나 cue가 깨져도 Character spawn, 이동, Server gameplay는 유지되고 해당 연출만 격리된다.
8. 아래 레퍼런스 완성 계약을 F1 Tool에서 처음부터 끝까지 수행한 같은 asset이 gameplay에서도 같은 결과로 나온다.

### G07~G09 레퍼런스 완성 계약

네 장의 레퍼런스 이미지는 개별 아이디어가 아니라 남은 G 전체의 제품 목표다. 항목별 owner는 다음처럼
나누되 G09 수동 smoke에서 한 흐름으로 다시 검증한다.

| 레퍼런스 동작 | 닫는 G | 최종 결과 |
|---|---|---|
| `Effect Tool / Model View / Effect Detail / All Effects / Data Files` | G07 | 다섯 창이 같은 활성 Document와 session을 공유한다. |
| `Mesh / Texture / Particle / Decal / Trail`, `CreateEffect` | G07, G08 | `Texture`는 `SPRITE`; 선택 종류의 실제 layer가 Document stack에 추가된다. |
| Base/Noise/Mask/Emissive/Dissolve slot, category, thumbnail, KeyFrames | G07 | thumbnail click 한 번으로 stable asset ID와 월드 결과가 즉시 바뀐다. |
| Reset, Update Textures, Update Meshes, Delete, Clear All, Time Reset All | G07, G08 | Document 편집과 preview reset을 구분하고 같은 catalog/playback owner를 재사용한다. |
| class/model 선택, 실제 clip 목록, play/stop/frame | G07 | 현재 다섯 roster와 실제 `CModel` animation만 표시한다. |
| weapon 선택, Player/Weapon/Clear pivot, mouse world position | G07 | `CHARACTER_SPEC` socket, player root, `CGameInstance::Picking`을 사용한다. |
| Lerp Position/Rotation/Revolution/Scaling/Velocity/ColorOffset | G08 | v5의 시작값·끝값을 deterministic lifetime 평가로 재생하고 seek한다. |
| Color Clip/Mul, Bloom, Distortion, Radial, UV, sequence/loop/tile | G06, G08 | SceneHDR/Distortion과 기존 Bloom/Deferred composite까지 실제로 소비한다. |
| Lifetime/Start Delay/AfterImage/Dissolve/Billboard/Pass | G06, G08 | typed 필드와 profile로 저장·재로드하며 arbitrary pass string을 쓰지 않는다. |
| Particle와 `Trail Vertex -/+` | G08 | instancing Particle과 dynamic ribbon Trail이 월드에서 실제 생성된다. |
| Decal thumbnail click 후 ground/object 표면 표시 | G08 | depth reconstruction 기반 projected Decal로 월드 표면에 고정된다. |
| Save/Load한 합성 Effect를 Animation cue에 연결 | G09 | anchor/local transform/follow/stop과 admitted EffectAssetId가 함께 저장된다. |
| DimensionMaster 스킬에서 생성·종료 | G09 | Server-approved action 뒤 같은 Effect asset을 runtime에서 spawn/cleanup한다. |

레퍼런스의 예전 class/weapon 문자열 자체는 복제 대상이 아니다. 현재 프로젝트 roster와
`CHARACTER_SPEC`가 데이터 정본이며, 복제 대상은 `선택 → animation → pivot → effect 확인` 동작이다.

## G09-02. 권위와 소유권

| 값 | owner | 소비자 |
|---|---|---|
| damage/hit/action timing | Server + `Data/Balance` | Server room/snapshot |
| `skillId`, `comboStage`, `actionStartTick` | Server snapshot | `CCharacter::Apply_NetworkAction` |
| `effectId` | `PlayerSkills.json`의 Client presentation field | catalog/cue validation과 fallback spawn |
| Effect 모양·local lifetime | Effect v5 Document | `CEffect_Playback` |
| clip 내 cue time | `.animevents` admitted EFFECT row | `CCharacter` presentation cue cursor |
| root/bone/local offset/follow/stop | Animation EFFECT cue | `CEffectPresentationService` |
| 현재 bone world matrix | 실제 `CCharacter`와 `CModel` | attachment resolve |

`CPlayerController`, `Logic_DimensionMaster`, UI는 Effect를 spawn하지 않는다. Server는 EffectAssetId,
resource path, animation clip을 알지 않는다. Client는 snapshot으로 확인한 action edge에서만 presentation을 시작한다.

## G09-03. Authoring catalog

### 새 파일 `Data/Effects/EffectCatalog.json`

이 파일은 publish 허용 목록만 소유한다. source candidate, Prototype tag, pointer, vector index를 넣지 않는다.

```json
{
  "schema": "lostark.effect-catalog",
  "formatVersion": 1,
  "effects": [
    { "effectAssetId": "effect.dimensionmaster.skill.2050010", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050110", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050150", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050190", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050200", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050210", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050220", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050240", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050500", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050510", "admission": "ADMITTED" },
    { "effectAssetId": "effect.dimensionmaster.skill.2050540", "admission": "ADMITTED" }
  ]
}
```

각 ID는 정확히 다음 파일을 가리킨다.

```text
Data/Effects/Authored/<EffectAssetId>.effect.json
```

459개 legacy `.effect/.weffect` 후보와 `dimensionist_admission.json`의 `candidate_only`는 이 catalog에
자동 추가하지 않는다. G09의 11개 v5 문서는 Effect Tool에서 resource와 결과를 직접 확인한 별도 정본이다.

## G09-04. DimensionMaster 11개 binding

`PlayerSkills.json`의 현재 빈 `effectId`를 다음과 같이 채운다.

| slot | skillId | display name | action/clip 정본 | effectId |
|---|---:|---|---|---|
| LMB | 2050010 | 기본 공격 | `att_battle_1_01..04` | `effect.dimensionmaster.skill.2050010` |
| Q | 2050110 | 예고 | `sk_dimensionalbreak_01` | `effect.dimensionmaster.skill.2050110` |
| W | 2050150 | 공간 베기 | `sk_riftslash_01..02` | `effect.dimensionmaster.skill.2050150` |
| E | 2050220 | 일점 관통 | `sk_momentaryrift` | `effect.dimensionmaster.skill.2050220` |
| R | 2050190 | 진공 | `sk_voidtrigger` | `effect.dimensionmaster.skill.2050190` |
| A | 2050240 | 경계 돌파 | `sk_telekinesisthrust_01,04` | `effect.dimensionmaster.skill.2050240` |
| S | 2050210 | 분광 | `sk_willowrend` | `effect.dimensionmaster.skill.2050210` |
| D | 2050200 | 공간 절단 | `sk_tearingsword_01..02` | `effect.dimensionmaster.skill.2050200` |
| F | 2050500 | 업의 경계 | `sk_dimensionprison` | `effect.dimensionmaster.skill.2050500` |
| T | 2050510 | 일념 | `sk_dimensionthrust_01..02` | `effect.dimensionmaster.skill.2050510` |
| V | 2050540 | 무간의 옥 | `sk_super_timewave` | `effect.dimensionmaster.skill.2050540` |

DimensionMaster에는 `ALT_V` binding을 만들지 않는다. V의 Effect는 G09에서 보이지만 원본 약 3.7초
Matinee camera/cut/fullscreen post-process까지 동일하다는 뜻은 아니다.

## G09-05. Effect publisher

### 새 파일 `Tools/EffectPipeline/Publish-Effects.ps1`

한 줄 책임: admitted source Document와 resource dependency를 전부 검증한 뒤 runtime catalog 하나를 원자적으로 교체한다.

### 입력과 출력

```text
입력  Data/Effects/EffectCatalog.json
입력  Data/Effects/Authored/<EffectAssetId>.effect.json
입력  Client/Bin/Resources/<Resources-relative asset ID>
출력  Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json
```

runtime catalog entry는 다음을 포함한다.

```json
{
  "effectAssetId": "effect.dimensionmaster.skill.2050110",
  "authoringFormatVersion": 5,
  "contentSha256": "64-lowercase-hex",
  "dependencies": [
    {
      "assetId": "Effect/DimensionMaster/Textures/example.dds",
      "sha256": "64-lowercase-hex"
    }
  ],
  "document": {}
}
```

`document`에는 validated v5 Document 전체가 들어간다. runtime은 authoring 경로를 다시 열지 않는다.

### publisher 흐름

```text
EffectCatalog exact schema parse
→ stable EffectAssetId 중복 검사
→ ID에서 authoring filename 계산
→ v5 Document exact parse와 filename ID 일치 검사
→ Element ID/kind/detail/budget 검사
→ 모든 resource slot의 Resources-relative path 검사
→ 실제 file 존재와 SHA-256 계산
→ effect/document/dependency를 local runtime object에 stage
→ 전체 JSON을 temporary file에 UTF-8 BOM 없이 write
→ temporary file 재parse와 entry/hash count 확인
→ 기존 runtime을 rollback 이름으로 이동
→ temporary를 destination으로 promote
→ 성공 시 rollback 삭제
→ 실패 시 destination 복원
```

`Validate` mode는 output을 바꾸지 않는다. `Publish` mode만 destination을 교체한다.

### 새 파일 `Tools/EffectPipeline/Test-EffectPipeline.ps1`

정상 1개, wrong catalog/document version, unsafe path, drive path, unknown kind, duplicate Effect/Element ID,
missing resource, hash mismatch, 중간 promote 실패를 temp fixture로 검증한다. 각 실패 뒤 기존 runtime bytes가
동일한지 확인한다.

## G09-06. `Effect_Catalog.h/.cpp`

### 이 파일이 존재하는 이유

제품 runtime은 `Data/Effects/Authored`와 Effect Tool을 읽지 않는다. publisher가 승인한 runtime catalog만
읽고 stable EffectAssetId로 immutable Document를 찾는다.

### 새 public 계약

```text
class CEffectCatalog final

Load(std::string& outStatus)
  EffectCatalog.runtime.json을 parse/validate/stage/commit한다.

Find(const std::string& effectAssetId)
  admitted immutable Document를 찾고 없으면 nullptr를 반환한다.

Contains(const std::string& effectAssetId)
  Animation Tool admission UI와 publisher smoke가 같은 ID 집합을 확인한다.

Get_EffectAssetIds()
  Debug Tool의 admitted Effect 목록을 정렬된 read-only view로 제공한다.

Clear()
  Client shutdown에서 committed catalog를 해제한다.
```

### 멤버와 불변식

| 멤버 | 의미 |
|---|---|
| `s_Effects` | EffectAssetId → immutable Document owner map |
| `s_RuntimeRevision` | 성공한 catalog load마다 증가하는 generation; 실패 시 증가하지 않는다. |
| `s_Status` | 마지막 load 결과 |

- `Load` 실패 시 이전 `s_Effects`를 유지한다.
- runtime path는 executable module 기준 `../Bin/DataFiles/Effect`와 installed `Bin/DataFiles/Effect` 후보만 해석한다.
- catalog 안의 절대 경로를 읽지 않는다.
- missing/corrupt catalog는 Client 전체 시작을 막지 않고 Effect presentation만 unavailable로 만든다.

## G09-07. Animation EFFECT cue v5

현재 `.animevents`의 `effectref=source`는 추출 근거일 뿐 제품 EffectAssetId가 아니다.
G09는 admitted row를 다음 형식으로 저장한다.

```text
"clip_name" EFFECT startms=200 payload="effect.dimensionmaster.skill.2050110" effectref=asset anchor="root" follow=follow stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
```

window stop이 필요하면 `endms`와 `stop=cue_end`를 함께 쓴다.

### enum class

```text
EFFECT_FOLLOW_POLICY
  FOLLOW    매 frame current root/bone transform을 다시 읽는다.
  SNAPSHOT  spawn 순간 world transform을 유지한다.
  END       잘못된 parse와 초기화 누락을 검출한다.

EFFECT_STOP_POLICY
  NATURAL   Effect Document lifetime이 끝날 때까지 재생한다.
  CUE_END   endms에서 emission/render를 중단한다.
  END       sentinel이다.
```

`anchor="root"`는 Character root다. 다른 문자열은 raw bone index가 아니라 `CModel::Has_Bone`으로 검증하는
stable bone 이름이다. local position은 meter, rotation은 degree, scale은 양수 배율이다.

v3 source row는 source reference로 유지한다. v4 admitted row는 root/follow/natural/identity default로 읽고,
다음 Save에서 v5로 쓴다. source row를 admitted row로 자동 변환하지 않는다.

## G09-08. 새 파일 `AnimationEffectCueDocument.h/.cpp`

제품 `CCharacter`가 Debug 전용 `CAnimation_Tool` 내부 struct를 include하지 않게 하는 작은 runtime reader다.

### `ANIMATION_EFFECT_CUE`

| 필드 | 의미 |
|---|---|
| `strClipName` | 실제 `CModel` animation name |
| `iStartMs`, `iEndMs` | clip local millisecond; point cue는 같은 값 |
| `strEffectAssetId` | `CEffectCatalog`에서 찾아야 하는 admitted ID |
| `strAnchorSlotId` | `root` 또는 검증된 bone 이름 |
| `LocalTransform` | anchor 뒤에 곱하는 position/rotation/scale |
| `eFollowPolicy` | follow/snapshot |
| `eStopPolicy` | natural/cue_end |

### 함수 흐름

```text
Load(animationAssetId, availableClips, outDocument, outStatus)
→ Data/Animation/Authored/<Asset>/<Asset>.animevents read
→ header/version/owner parse
→ EFFECT + effectref=asset row만 runtime cue로 stage
→ clip 존재, ms 범위, EffectAssetId catalog 존재 검사
→ 같은 clip/start/effect/anchor 중복 거부
→ 전체 성공 시 outDocument 교체
```

source reference, SOUND/HIT/CANCEL 같은 다른 row는 runtime Effect cue vector에 넣지 않는다.

## G09-09. `Effect_PresentationService.h/.cpp`

### 이 파일이 소유하는 것

Effect 문서나 Character animation을 소유하지 않는다. 현재 level의 `Layer_Effect`에 spawn한 Effect instance와
attachment lifetime만 소유한다.

### `EFFECT_SPAWN_DESC`

```text
effectAssetId
weak Character owner
anchorSlotId
local transform
follow policy
stop policy
optional cue end time
actionStartTick
```

`actionStartTick`은 저장 ID가 아니라 같은 replicated action edge의 중복 spawn을 막는 session token이다.

### 함수

| 함수 | 한 줄 책임 |
|---|---|
| `Spawn` | catalog lookup, anchor resolve, EffectObject clone을 stage한 뒤 active instance에 commit |
| `Update` | follow attachment와 cue end/natural finish를 갱신하고 끝난 object를 layer에서 제거 |
| `Stop_Owner` | Character despawn/disconnect 때 그 owner의 Effect를 제거 |
| `Clear_Level` | level 전환 전 현재 level의 Effect를 모두 제거 |
| `Get_Status` | 마지막 격리된 presentation 실패 이유를 Debug Tool에 제공 |

### `Spawn` 흐름

```text
Server-approved Character action 또는 crossed animation cue
→ EffectAssetId를 CEffectCatalog::Find
→ Character weak owner와 current level 확인
→ root 또는 bone world transform resolve
→ CEffectObject desc와 Document를 local stage
→ Layer_Effect clone 성공
→ root/local transform과 playback reset
→ active instance vector에 commit
```

어느 단계든 실패하면 새 object만 제거하고 기존 Effect와 Character action을 유지한다.

## G09-10. `CCharacter` 연결

### 추가 상태

| 멤버 | 의미 |
|---|---|
| `m_EffectCueDocument` | 현재 animation asset의 admitted cue snapshot |
| `m_iPreviousEffectCueTimeMs` | 현재 clip에서 직전 frame까지 소비한 시간 |
| `m_strEffectCueClip` | cue cursor가 속한 clip name |
| `m_iEffectCueTargetGeneration` | clip/action reset을 구분하는 session generation |

### 추가 함수

| 함수 | 한 줄 책임 |
|---|---|
| `Load_EffectCues` | Character spawn 시 available clip과 catalog로 cue document를 stage한다. |
| `Reset_EffectCueCursor` | 새 action/clip/combo stage에서 cursor를 0으로 되돌린다. |
| `Update_EffectCues` | 직전 ms와 현재 ms 사이를 통과한 cue를 정확히 한 번 spawn한다. |
| `Resolve_EffectAnchor` | 이 Character의 root/bone world matrix를 layer 검색 없이 계산한다. |

### 승인 action 흐름

```text
CClientReplication snapshot
→ CCharacter::Apply_NetworkAction(skillId, actionStartTick, comboStage)
→ 같은 actionStartTick이면 기존 early return으로 중복 차단
→ Play_Skill / Advance_ComboStage
→ cue cursor reset
→ 매 Character Update에서 current animation track ms 계산
→ (previousMs, currentMs]에 있는 admitted cue를 Spawn
```

현재 clip에 admitted cue가 하나도 없지만 `PlayerSkills.effectId`가 admitted 상태면 action start에
Character root/follow/natural/identity로 한 번 fallback spawn한다. cue가 하나라도 있으면 fallback은 사용하지 않아
중복 Effect를 만들지 않는다. catalog/cue 실패는 animation과 snapshot 적용을 실패시키지 않는다.

## G09-11. Animation Tool 연결

기존 `EFFECT_REFERENCE_KIND::SOURCE_REFERENCE`와 `EFFECT_ASSET_ID`는 유지한다.

### 변경 함수

```text
Render_HitEvents
  EFFECT row에서 source/admitted badge, EffectAssetId, anchor, follow, stop, local transform을 표시한다.

Use_Selected_Effect
  CEffectCatalog에 존재하는 ID만 별도 EFFECT_ASSET_ID row로 추가한다.

Validate_Events
  asset row의 ID/clip/time/anchor/local scale/follow/stop을 검증한다.

Save_Events
  admitted row가 하나라도 있으면 LOSTARK_ANIM_EVENTS 5로 atomic save한다.

Load_Events
  v3 source, v4 admitted default, v5 full cue를 parse → validate → stage → commit한다.
```

Effect Tool의 `Use Selected Effect` request는 다음 typed snapshot을 Animation Tool에 전달한다.

```text
targetGeneration
animationAssetId
clipName
timeMs
effectAssetId
pivotKind          PLAYER_ROOT / WEAPON_SOCKET / MODEL_BONE
anchorSlotId       root 또는 검증된 stable socket/bone 이름
localTransform     position meter, rotation degree, positive scale
followPolicy       FOLLOW / SNAPSHOT
stopPolicy         NATURAL / ACTION_END / ANIMATION_END
```

WORLD mouse-pick pivot은 Tool preview에는 쓸 수 있지만 Character animation cue에는 그대로 저장하지 않는다.
runtime Character cue로 넘기려면 player/weapon/bone 중 하나를 명시해야 한다.
Animation Tool만 marker를 만들고 Dirty 상태를 소유한다. Effect Tool이 `.animevents`를 직접 쓰지 않는다.

## G09-12. `PlayerSkillCatalog`와 balance receipt

`PLAYER_SKILL_DEFINITION`에 `std::string strEffectId`를 추가하고 `PlayerSkills.json.effectId`를 읽는다.
빈 값은 다른 class의 아직 미완성 presentation을 위해 허용하지만, 값이 있으면 stable ID와
`CEffectCatalog::Contains`를 Client presentation load에서 검사한다.

DimensionMaster 11개 `effectId` 변경 뒤 다음 순서를 지킨다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

receipt의 해당 field는 `PROJECT_TUNED`다. Server bootstrap은 EffectAssetId를 소비하지 않으며 damage/timing
authority도 바뀌지 않는다. 그래도 balance 정본을 수정했으므로 publisher 검증과 Server 재시작 증거를 남긴다.

## G09-13. 추가·수정 파일

### 새 코드/도구/데이터

```text
Client/Public/Effect_Catalog.h
Client/Private/Effect_Catalog.cpp
Client/Public/Effect_PresentationService.h
Client/Private/Effect_PresentationService.cpp
Client/Public/AnimationEffectCueDocument.h
Client/Private/AnimationEffectCueDocument.cpp
Tools/EffectPipeline/Publish-Effects.ps1
Tools/EffectPipeline/Test-EffectPipeline.ps1
Data/Effects/EffectCatalog.json
Data/Effects/Authored/effect.dimensionmaster.skill.<11 ids>.effect.json
```

### 수정 파일

```text
Client/Public/Animation_Tool.h
Client/Private/Animation_Tool.cpp
Client/Public/Character.h
Client/Private/Character.cpp
Client/Public/PlayerSkillCatalog.h
Client/Private/PlayerSkillCatalog.cpp
Client/Public/MainApp.h
Client/Private/MainApp.cpp
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Data/Balance/PlayerSkills.json
Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json
Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents
Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

`CEffectObject`, playback, renderer, HLSL은 G07~G08 파일을 그대로 제품 runtime에서 사용한다.
`EffectRuntimeObject`, `RuntimeParticleManager` 같은 두 번째 경로를 추가하지 않는다.

## G09-14. project/filter 등록

새 Client H/CPP 여섯 개를 기존 Effect/Animation Header와 Source 필터에 등록한다.
`Data/Effects/EffectCatalog.json`과 11개 authored JSON은 `None` item으로 `96.DataFiles\Effects`에만 노출한다.
생성물 `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`과 Resources payload는 project source item이나
Git commit에 넣지 않는다.

## G09-15. 구현 순서

```text
G09-1  EffectCatalog authoring schema와 Publish/Test pipeline
G09-2  runtime EffectCatalog parse/validate/stage/commit
G09-3  EffectPresentationService와 기존 EffectObject lifecycle
G09-4  Animation EFFECT cue v5 reader/writer/admission UI
G09-5  Character approved action cue cursor와 fallback effectId
G09-6  11개 DimensionMaster v5 Effect 저작과 catalog admission
G09-7  PlayerSkills 11 effectId + provenance + gameplay publish
G09-8  local/remote/combo/disconnect/level transition runtime smoke
```

11개 Effect 품질 작업은 Q부터 순서대로 하지 않고 다음 작은 대표 세트로 렌더 기능을 먼저 증명한 뒤
나머지 ID를 채운다.

```text
기본 hit/burst → 2050110 Q
trail/weapon follow → 2050150 W
long-range/world effect → 2050220 E
particle+distortion → 2050190 R
multi-layer/afterimage → 2050240 A
나머지 S/D/F/T → 같은 admitted 경로
V → Effect 본체만, cinematic parity는 별도
LMB 4 stage → 각 실제 clip의 cue cursor와 comboStage 재시작 확인
```

## G09-16. 자동 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Test-EffectPipeline.ps1
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
msbuild Engine/Default/Engine.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Shared/Default/Shared.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
Server/Bin/Debug/Server.exe --contract-test
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

Effect G09는 packet을 바꾸지 않지만 기존 Server-approved action 경계를 우회하지 않았다는 회귀 증거로
protocol harness와 Server contract test를 같이 실행한다.

## G09-17. 수동 runtime smoke

```text
Client/Default에서 Debug Client 실행
→ Lobby / Test / Development / F1
→ G07~G09 레퍼런스 완성 계약을 새 Document에서 순서대로 수행
→ thumbnail click, world Decal, character animation, Player/Weapon pivot, Particle/Trail을 한 합성 Effect로 저장
→ Discard/Load 뒤 같은 월드 모양과 timeline 결과 확인
→ Animation Tool Use Selected Effect에서 anchor/local transform/follow/stop marker 저장
→ Server 실행
→ Client/Default에서 Debug Client 실행
→ Lobby / Character Select / DimensionMaster 선택 / Server Play
→ Q W E R A S D F T V를 각각 1회 사용
→ 각 action이 Server snapshot에서 승인된 뒤 해당 Effect 시작
→ LMB 4 stage에서 각 새 actionStartTick당 Effect 정확히 1회
→ remote Client에서도 같은 approved action Effect 표시
→ 잘못된 effectId 하나로 fixture 실행 시 그 Effect만 누락되고 Character/Server gameplay 유지
→ cue anchor를 없는 bone으로 바꾸면 load 거부, 기존 catalog/cue 유지
→ action 중 disconnect와 level 전환 시 Layer_Effect object 0개
→ F1 Effect Tool에서 같은 asset을 열었을 때 gameplay와 같은 모양/lifetime
```

## G09-18. G09 완료와 후속 경계

G09 완료는 네 장의 레퍼런스가 보여 준 제작 흐름으로 합성 Effect를 만들고, 저장·재로드·Animation cue를
거쳐 DimensionMaster 11개 입력의 admitted composite Effect가 실제 runtime에서 나온다는 뜻이다.
이 화면과 동작을 완성하기 위한 별도 G10은 두지 않는다. 과거 문서의 distortion G10 표기는 G08의
SceneHDR/Distortion/Deferred composite에 흡수한다. 다음은 그 이후의 별도 원본 자산 호환·품질 단계다.

- 459개 legacy Cascade 후보 전체 1:1 module parity
- 미해결 외부 module 65,626개와 material 17개 자동 admission
- VectorField, CameraOffset, 현재 projected Decal을 넘어선 전체 UE3 Decal module parity, arbitrary material expression
- fullscreen RGB split/zoom blur/desaturation/transition
- `STANDARD_SKILLCAM_DIMENSIONMASTER`의 약 3.7초 camera/cut/Matinee sequencer
- hit collider나 damage timing을 Client Effect에 종속시키는 변경

이 경계가 남아도 11개 skill Effect runtime 연결은 완료로 판정할 수 있지만, 원본 Lost Ark의 모든
후처리·카메라·Cascade graph까지 완벽 복제했다고 보고하지 않는다.

# All Effects 발탄 Runtime Product·서버 권위·독립 Effect·정적 월드/표면 확장 구현 계획

## 0. 작업 결론

이번 작업의 기준은 `Pattern마다 빈 aggregate Effect 하나만 보이게 한다`가 아니다. 그 화면은 실제로
publish되어 Server pattern이 소비하던 Product Effect를 숨겨 사용자가 기존 Effect가 삭제된 것으로
오해하게 만들었다.

2026-08-26 추가 요청은 여기서 한 단계 더 나아간다. 기존 `Independent Effect`가 서버 Pattern/Combat
Object가 소유하는 동적 Effect 두 개만 보여 주는 제한을 유지하되, 같은 All Effects 화면에서 Area가
소유하는 정적 월드 Effect와 기존 Map/Deploy 표면 presentation도 열고 실제 소비 값을 튜닝할 수 있게
확장한다. 여기서 `정적`은 픽셀이 멈춘다는 뜻이 아니라 **월드 배치와 수명이 Character/Boss attach가
아닌 Area/placement에 소속된다**는 뜻이다. 하늘 layer의 UV·revolution·opacity timeline은 계속 움직일
수 있다.

실측한 정본은 그대로 남아 있다.

```text
Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
  cue 51개
  unique effectAssetId 48개

Data/Effects/EffectCatalog.json
  effect.valtan.* 58개 이상
  위 48개 cue asset과 effect.valtan.sky-axe.active의 authored 문서가 모두 존재
  각 문서의 effectAssetId가 catalog와 일치하고 drawable Element가 하나 이상 존재
```

따라서 `All Effects -> Valtan`은 다음 세 경계를 동시에 보여 주어야 한다.

```text
INDEPENDENT EFFECT
  Pattern-owned (2)
    INNER / OUTER 도넛
    플레이어 추적 도끼
  Area / Static Presentation (2)
    발탄 바닥 균열 Emissive
    발탄 100줄 붉은 Vortex Sky

PATTERN
  [Play Server]
  Runtime Product Effects
    [Open Editor] [Play Effect 또는 Play Effect + Animation]
  Unpublished Pattern Draft
    sidecar가 있을 때만 [Open Draft]
```

- `Play Server`는 선택 Pattern 전체를 기존 Server fixed-tick 경로로 실행한다.
- `Runtime Product Effects`는 그 Pattern이 현재 실제로 소비하는 published cue와 combat-object visual이다.
- `Open Editor`와 로컬 `Play`는 실제 authored Effect 문서를 열고 Model View 저작 검증을 제공한다.
- `Unpublished Pattern Draft`는 `New Effect`가 만든 sidecar 초안일 뿐 Server Play 입력이 아니다.
- 바닥 균열은 새 decal/복제 mesh를 만들지 않고 기존 `CDeployPropObject`의 실제 deferred emissive
  overlay 값을 같은 Effect Detail UI에서 편집한다.
- 붉은 하늘은 기존 6개 Map proxy와 hardcoded shader policy를 하나의 direct-authored Effect와 stable
  Area placement로 이관하고, `VALTAN_FOUR_PILLARS_105` Server stage clock으로 sample한다.

## 1. 데이터와 재생 권위

### 1.1 Pattern의 실제 Runtime Product Effect

Pattern row는 `VALTAN_PATTERN_VIEW::Stages`를 기준으로 다음 두 소스를 합친다.

```text
Stage.ProductCues
  CLIP_BOUND Product cue
  STAGE_CLOCK Product cue

Stage.CombatObjectEffects
  Server combat-object가 생성하는 시각 Effect
```

각 row는 stable `effectAssetId`로 `EffectCatalog.json`의 `authoringPath`를 resolve한다. 사용자가
`Open Editor`를 누르면 그 exact authored Effect 문서를 연다. 같은 Effect가 여러 Pattern/Stage에서
소비될 수 있으므로 UI row의 중복 occurrence와 물리 asset 개수를 혼동하지 않는다.

clip-bound Product row의 `Play Effect + Animation`은 실제 clip occurrence와 presentation binding을
조인해 Model View animation과 Effect를 같은 authoring timeline으로 재생한다. `Open Editor`는 같은
binding을 준비하되 animation을 time 0에서 pause해 단순 열기와 재생을 구분한다.

`STAGE_CLOCK` row는 clip에 종속되지 않으므로 정적인 Valtan Model View root에서 `Play Effect`만 제공한다.
combat-object row의 로컬 재생은 owner stage의 전체 animation timeline과 Effect 시작 offset을 사용하지만,
추적 위치·wave 생성·hit 판정은 계속 Server 권위다.

### 1.2 Pattern `Play Server`

`Play Server`의 호출 경로는 새로 만들지 않는다.

```text
Effect Tool Play Server
-> CValtanPatternAuditionService::Submit(patternId)
-> CNetworkManager::Send_ValtanPatternAuditionById
-> C2S PLAY_PATTERN_ID
-> CValtanBrain Server fixed tick
-> replicated pattern/action/stage/startTick
-> Client Valtan presentation binding
-> published Product cue / combat-object presentation
```

버튼은 `VALTAN_ARENA`, Server 연결, 활성 `boss.valtan.center`, idle audition 조건에서만 활성화한다.
요청 성공은 재생 완료가 아니라 queued 상태이며, 완료·거부·중단은 기존 audition snapshot을 따른다.

Character Select 또는 Effect Tool 로컬 clock을 Server 합격 근거로 사용하지 않는다. 다만 사용자가
Effect 모양과 연결 animation을 빠르게 편집·확인하는 로컬 authoring 기능은 유지한다.

### 1.3 Independent Effect

`Data/Valtan/Valtan.presentation.json`의 `independentEffects`에는 owner가 명확한 기존 두 asset만 둔다.
이 배열은 Server pattern library이고 정적 Area item의 catalog가 아니다.

| independentEffectId | runtime owner | 로컬 버튼 | Server 버튼 |
|---|---|---|---|
| `valtan.independent-effect.donut-in-out` | `VALTAN_FIST_IN_OUT / INNER / SERVER_PATTERN_STAGE` | `Open Editor`, `Play Effect` | `Play Server Owner` |
| `valtan.independent-effect.target-axe` | `VALTAN_HIGH_JUMP / AIRBORNE / SERVER_COMBAT_OBJECT` | `Open Editor`, `Play Effect + Owner Animation` | `Play Server Owner` |

같은 asset이 Pattern의 `Runtime Product Effects`에서도 보일 수 있다. root row는 독립 저작 진입점이고,
Pattern 하위 row는 실제 소비 occurrence이므로 이 중복은 의도된 projection이다.

`Play Server Owner`는 UI에 core/manual Pattern으로 노출되지 않은 owner라도 exact stable ID로 기존 Server
audition을 제출할 수 있어야 한다. 도끼의 로컬 authoring preview는 `AIRBORNE` owner stage의 시작 offset을
사용하고, 실제 플레이어 추적과 hit는 `Play Server Owner`로 확인한다.

All Effects UI는 이 두 row와 별도로 Valtan Area의 `.mapeffects.json`을 projection한다.

| independentEffectId | presentation kind | 실제 owner | 로컬 버튼 | Server 버튼 |
|---|---|---|---|---|
| `valtan.independent-effect.floor-crack-emissive` | `DEPLOY_SURFACE_OVERLAY` | `VALTAN_FLOOR_BRICK_A/B`의 stable Deploy placement 4개, `INTACT` 상태 | `Open Surface Detail`, `Preview Current Area` | 없음 |
| `valtan.independent-effect.red-vortex-sky` | `EFFECT_DOCUMENT` | Valtan Area world placement + `VALTAN_FOUR_PILLARS_105` window | `Open Editor`, `Preview at Placement`, `Play/Loop/Stop` | `Play Server Activation` |

Pattern-owned exact count `2`를 `4`로 바꾸거나 `VALTAN_INDEPENDENT_EFFECT_VIEW` parser에 client static
ownership string을 억지로 추가하지 않는다. UI root만 `Pattern-owned`와 `Area / Static Presentation` 두
source를 합성한다. 그래야 하늘과 바닥이 Server pattern master에 가짜 owner를 만들지 않는다.

## 2. FIST 도넛의 현재 계약

`VALTAN_FIST_IN_OUT`을 과거 4-stage animation sequence로 복원하지 않는다. 현재 사용자가 확정한 계약은
animation-free 독립 도넛이다.

```text
VALTAN_FIST_IN_OUT
  stage: INNER 하나
  duration: 2600ms
  animation.mode: NONE
  Product cue timingBasis: STAGE_CLOCK
  trigger: stage +0ms
  gameplay: RING 8~16, hit +1600ms
```

Product cue에는 `clipOccurrenceId`가 없어야 한다. 기존 네 action binding은
`playbackMode: NONE` tombstone으로 남기며 로컬 animation sequence로 다시 승격하지 않는다.

- `Open Editor` / `Play Effect`: 정적 Model View root에서 도넛 Effect만 검증한다.
- `Play Server Owner`: Server stage clock, hit timing, snapshot/natural/once 정책을 포함한 실제 owner Pattern을
  검증한다.

## 3. Pattern 저작 초안 sidecar의 위치

`Data/Effects/ValtanPatternAuthoringEffects.json`은 published runtime binding이 아니라 저작 초안 소유권
문서다.

```text
patternId
effectAssetId
authoringPath = Effects/Authored/<effectAssetId>.effect.json
state = DRAFT_ATTACHED
```

`New Effect`는 선택 Pattern에 빈 v13 Effect와 sidecar binding을 2문서 CAS transaction으로 만든다.
이 초안은 `Unpublished Pattern Draft` 아래에만 표시한다. 생성만으로 다음 정본을 수정하거나 Server
재생에 끼워 넣지 않는다.

- `Valtan.patterneffectcues.json`
- `Valtan.patternbindings.json`
- `EffectCatalog.json`
- gameplay/presentation Product

따라서 published Product row와 draft row는 같은 `Effect`라는 이름으로 합치거나 대체하지 않는다.

## 4. All Effects projection

visible Pattern inventory는 기존 요구대로 core 6개와 `ManualAuditions` 20개다.

```text
CORE SERVER PATTERNS (6)
  VALTAN_WHIRLWIND
  VALTAN_FOUR_SLASH
  VALTAN_HIGH_JUMP
  VALTAN_DASH_CHARGE
  VALTAN_FLOOR_WIPE_130
  VALTAN_ARENA_BREAK_109

ANIMATOR PATTERNS (20)
  CValtanPatternTree::ManualAuditions authored order
```

검색은 Pattern 이름뿐 아니라 Stage, clip occurrence, Product cue, Stage effect, combat-object effect까지
조회한다. `VALTAN_FIST_IN_OUT` Pattern row는 독립 도넛 root와 중복되므로 숨기지만 owner resolution과
Server audition 대상에서는 제거하지 않는다.

Pattern row를 열면 `Runtime Product Effects`를 먼저 표시한다. sidecar binding이 있을 때만 그 아래에
`Unpublished Pattern Draft`를 별도 표시한다. `(not created)` 한 줄로 실제 runtime Effect를 대체하지 않는다.

## 5. PR #232 한국어 P2 Pattern 통합

PR #232의 선행 gameplay/client 수정도 commit별로 대조한다. held-LMB repeat, Character Select 고정 cooldown
override 제거, 네 class max HP/provenance는 현재 WIP를 보존하며 적용한다. Animation Tool no-blend 수정은 현재
master sampler가 같은 목적을 더 뒤의 구조로 대체했는지 확인하고, 대체되었다면 옛 hunk를 강제로 되살리지
않는다.

PR #232의 animation-chain promotion 정본을 현재 split gameplay/presentation과 생성 Product에 반영한다.
20개 manual Pattern 중 15개는 아래 stable ID와 한국어 표시 이름으로 승격되고, PR에서 이름이 정리되지
않은 5개(`four`, `rush`, `front-back-front`, `twohand`, `whirlwind`)는 기존 ID/표시 이름을 유지한다.

| stable pattern ID | 표시 이름 |
|---|---|
| `VALTAN_SIX_PIZZA_106` | 중앙이동 후 6방향 공격 후 피자 패턴 |
| `VALTAN_ATTACK_WHIRLWIND` | 점프찍기 후 휠윈드 |
| `VALTAN_CHARGE` | 모아치기 |
| `VALTAN_ROAR_CHARGE` | 사자후 후 위로 모아치기 |
| `VALTAN_THREE` | 3연속 내려치기 |
| `VALTAN_TERRAIN_DESTRUCTION` | 2페이즈 지형파괴 패턴 |
| `VALTAN_WARP` | 워프 패턴 |
| `VALTAN_TRASH` | 버러지 패턴 |
| `VALTAN_TRASH_CATCH_SUCCESS` | 버러지 패턴 잡기 성공 |
| `VALTAN_TRASH_CATCH_FAIL` | 버러지 패턴 잡기 실패 |
| `VALTAN_TRASH_CATCH_IF` | 버러지 패턴 잡기 분기 |
| `VALTAN_CATCH_BREATH` | 잡아채서 불어 날리기 |
| `VALTAN_COUNTER` | 카운터 쳐야 하는 내려치기 |
| `VALTAN_CHARGE_2` | 모아치기 2 |
| `VALTAN_STRUGGLING` | 3페이즈 전 발악패턴 |

stable ID 변경은 promotion manifest만 바꾸고 끝내지 않는다. split source의 manual audition,
scripted sequence 참조, presentation, Encounter, authored pattern binding과 promotion receipt를 같은 publisher
단위에서 갱신한다. All Effects는 `ManualAuditions`의 현재 stable ID를 resolve하므로 한국어 P2 Pattern을
별도 하드코딩 목록으로 복제하지 않는다.

## 6. 구현 파일과 책임

### `Client/Public/Effect_Tool.h`

- Pattern runtime row와 independent owner preview helper의 계약을 선언한다.
- local authoring preview와 Server audition 상태를 서로 다른 상태로 유지한다.
- pending document load가 standalone/Product preview intent와 owner timeline을 보존한다.

### `Client/Private/Effect_Tool.cpp`

- `Render_ValtanPatternNode()`가 published Product cue와 combat-object visual을 aggregate해 실제 row로 그린다.
- `Render_ValtanIndependentEffectNode()`가 `STAGE_CLOCK`, clip Product, combat-object owner를 분리한다.
- `Open Editor`는 animation을 준비한 뒤 time 0에서 pause한다.
- `Play Effect (+ Animation)`은 동일 binding을 재생하고, `Play Server Owner`는 실제 owner patternId를
  audition service에 제출한다.
- sidecar는 `Unpublished Pattern Draft`로만 그린다.

### 발탄 데이터와 publisher

- `Data/Valtan/Valtan.animation-chain-promotions.json`
- `Data/Valtan/Valtan.presentation.debug.json`
- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- publisher가 생성하는 Encounter/pattern binding/receipt closure

PR #232의 한국어 stable ID와 99-stage promotion closure를 위 정본 경로로 반영한다.

## 7. 자동 검증

```powershell
python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
python -B Tools/ValtanPipeline/promote_valtan_animation_chains.py --repo-root . --mode Validate
python -B Tools/ValtanPipeline/test_valtan_animation_chain_promotion.py
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 `
  -Mode ValidateV2 -RepositoryRoot .
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 `
  -Mode Validate -RepositoryRoot .
python -B Tools/ValtanActionExtractor/build_valtan_rootmotion.py --repo-root . --check

MSBuild Client/Default/Client.vcxproj `
  /t:ClCompile `
  /p:SelectedFiles=..\Private\Effect_Tool.cpp `
  /p:Configuration=Debug `
  /p:Platform=x64

MSBuild Client/Default/Client.vcxproj /m /p:Configuration=Debug /p:Platform=x64
MSBuild Server/Default/Server.vcxproj /m /p:Configuration=Debug /p:Platform=x64
MSBuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /m `
  /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
Server/Bin/Debug/Server.exe --contract-test
git diff --check
```

검증은 다음을 구분해 기록한다.

- authoritative cue 51개와 unique asset 48개가 유지되는가
- 모든 runtime asset이 catalog/authored drawable document로 resolve되는가
- FIST가 `NONE + STAGE_CLOCK`인가
- Pattern runtime row와 independent owner 버튼이 존재하는가
- 한국어 promotion이 20 Pattern, 99 occurrence/stage, 7 projected artifact closure를 이루는가
- V2 Product projection, balance runtime set, RootMotion과 sound/server consumer가 새 stable ID를 소비하는가
- focused compile과 전체 Client build가 실제로 통과했는가
- 사용자가 Valtan Arena에서 시각 결과를 확인했는가

## 8. 사용자 수동 검증

1. 사용자가 `Server + Client` Debug profile을 실행한다.
2. Valtan Arena에 입장하고 `boss.valtan.center`를 활성화한다.
3. `F1 -> Effect Tool -> All Effects -> Valtan`을 연다.
4. Pattern의 `Play Server`로 실제 Server fixed-tick 전체 Pattern을 확인한다.
5. Pattern을 펼쳐 `Runtime Product Effects`의 `Open Editor`와 `Play Effect + Animation`을 확인한다.
6. 도넛은 `Play Effect`로 Effect-only, `Play Server Owner`로 실제 stage clock/hit을 구분해 확인한다.
7. 도끼는 `Play Effect + Owner Animation`으로 AIRBORNE 연결 시점을 보고, `Play Server Owner`로 실제
   추적·wave·hit을 확인한다.
8. animator 목록에 한국어 P2 이름과 `2페이즈 지형파괴 패턴`이 표시되는지 확인한다.

에이전트는 Client를 자율 실행하거나 visual PASS를 기록하지 않는다.

## 9. 완료 조건

- 기존 51 cue/48 asset inventory를 삭제하거나 aggregate draft로 가리지 않는다.
- Pattern 아래 실제 published Product/combat-object row를 열고 편집·로컬 재생할 수 있다.
- Pattern `Play Server`와 independent `Play Server Owner`는 기존 Server authority를 사용한다.
- FIST는 `NONE + STAGE_CLOCK` 계약을 유지한다.
- sidecar draft는 unpublished authoring 문서로만 표시된다.
- PR #232의 한국어 P2 stable ID와 생성 closure가 publisher validation을 통과한다.
- focused harness, compile, 관련 publisher 검증과 `git diff --check` 결과를 RESULT에 실제 실행 기준으로 남긴다.
- 최종 시각 합격은 사용자의 Valtan Arena EXE 검증 뒤에만 기록한다.

## 10. 첨부 화면과 현재 자산 실측

### 10.1 1번 화면: 바닥 균열

1번 화면의 청록색/초록색 균열은 `CEffectObject`가 아니다. 현재 실제 소비자는 다음 경로다.

```text
VALTAN_FLOOR_BRICK_A/B
-> stable Deploy placement
-> CDeployPropObject
-> material index 1 emissive mask
-> RENDERGROUP::DEFERRED_OVERLAY
-> Shader_VtxMeshBinary pass 18
```

- source/runtime Deploy catalog의 현재 `emissiveIntensity`는 A/B 모두 `1.5`다.
- emissive texture는
  `Map/BG_RAD_VALTAN_A/.../bg_rad_valtan_crack_floor01_em_reconstruction.png`다.
- 네 실제 바닥 placement ID는
  `7000000000000000002`, `7000000000000000003`,
  `7000000000000000006`, `7000000000000000007`이다.
- 각 객체는 `STATIC + INTACT`일 때만 overlay를 제출하고 Server destruction이 `DESPAWNED`를 확정하면
  base mesh와 함께 사라진다.

따라서 이를 일반 decal이나 독립 mesh Effect로 복제하면 z-fighting, 이중 emissive, 붕괴 상태 불일치가
생긴다. 이번 확장은 **기존 실제 surface owner를 유지한 채** intensity/color/mask tuning packet을
Effect Tool의 static detail editor로 노출한다.

### 10.2 2번과 3번 화면: 현재 하늘과 목표 Vortex

2번은 현재 `sky_mirror_sm` 계열의 파란 하늘만 보이는 상태다. 3번은 HP multiplier가 `x104`인 100줄
구간이므로 현재 Product 기준 owner는 `VALTAN_FOUR_PILLARS_105`다. 109 문서 이름이나 과거 실험 ID를
새 activation owner로 사용하지 않는다.

기존 source audit 기준 원본 구성도 한 장의 sky texture가 아니다. `sky_mirror_sm + lv_sky_0161_d` 위에
5-emitter `par_d_spacehole_03`과 12-emitter `par_d_hugechaosgate_01`을 겹치는 구조다. 현재 프로젝트의
6개 proxy는 그 source를 video-match용으로 축약 복원한 것이며 원 UE3 17-emitter exact 복원이라고
과장하지 않는다.

현재 붉은 하늘은 Effect document가 아니라 다음 6개 hidden Map proxy와 hardcoded 정책의 조합이다.

| stable layer | 현재 상태 | 핵심 texture | profile |
|---|---|---|---|
| `VALTAN_PHASE_CHAOS_CLOUD` | visible | `fx_k_cloudtilie_01.png` | `RED_CLOUD_DISC` |
| `VALTAN_PHASE_CHAOS_ELECTRIC` | hidden | `fx_k_electile_02.png` | `NONE` |
| `VALTAN_PHASE_CHAOS_RING` | visible | `fx_d_shockwave_001_ycl.png` | `RED_RING` |
| `VALTAN_PHASE_SPACEHOLE_CLOUD` | visible | `fx_d_cloud_031.png` | `RED_CLOUD_DISC` |
| `VALTAN_PHASE_SPACEHOLE_CORE` | visible | `fx_d_atypical_019.png` | `DARK_APERTURE` |
| `VALTAN_PHASE_SPACEHOLE_STREAK` | hidden | `fx_a_cloud_017.png` | `NONE` |

`CValtanCinematicCameraController::Get_SkyLayerPolicies()`가 scale `2.75/2.45/2.25/1.90`, opacity,
회전 배수와 visibility를 C++에 고정하고, `CLevel_ValtanArena`가 exact 6개 `CMapAssetObject`를 찾아
`Shader_VtxMeshBinary.hlsl`의 세 procedural vortex profile을 매 frame 적용한다. 이 때문에 사용자는
Effect Details에서 layer별 값을 편집할 수 없다.

### 10.3 `fx_a_atypical_013_cl.dds` 판정

`fx_a_atypical_013_cl.dds`는 목표 하늘 완성본이 아니다.

- Valtan/Warlord 복사본은 같은 SHA-256
  `0F85EC20993380D65D1A494E23DA12B70598048DE481C7F372D7787468B3C4C3`다.
- 128x128 DXT1 grayscale cloud/orb mask이며 cubemap이 아니다.
- `effect.valtan.front-back-front.windup`의 60개 element 중 한 mesh-particle plane에서 noise/mask와
  조합되는 resource일 뿐, 현재 red sky Product asset이나 placement가 아니다.

따라서 이 DDS를 이름만 보고 하늘 전체 texture로 연결하지 않는다. Effect Detail에서 후보 resource로
비교할 수는 있지만, canonical red vortex는 위 6-layer source와 radial material 동작을 기준으로 만든다.

## 11. 정본 데이터 설계

### 11.1 Area static projection

새 source/runtime pair는 point light와 같은 Map publisher 경계를 사용한다.

```text
source
  Data/Maps/Authoring/LV_LUT_HEARTRB_ED/
    LV_LUT_HEARTRB_ED.mapeffects.json

runtime
  Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapeffects.json

MapCatalog
  sourceEffects / effects exact pair
```

schema는 `lostark.map-effect-presentation`, `formatVersion: 1`, exact `areaId`와 stable row를 가진 tagged
union으로 만든다. 한 row가 서로 다른 kind의 field를 섞으면 거부한다.

```json
{
  "schema": "lostark.map-effect-presentation",
  "formatVersion": 1,
  "areaId": "LV_LUT_HEARTRB_ED",
  "presentations": [
    {
      "independentEffectId": "valtan.independent-effect.floor-crack-emissive",
      "presentationKind": "DEPLOY_SURFACE_OVERLAY",
      "ownerPlacementIds": [
        "7000000000000000002",
        "7000000000000000003",
        "7000000000000000006",
        "7000000000000000007"
      ],
      "visibleStates": ["INTACT"],
      "materialIndex": 1,
      "emissiveIntensity": 1.5,
      "emissiveColor": [1, 1, 1, 1],
      "maskPower": 1
    },
    {
      "independentEffectId": "valtan.independent-effect.red-vortex-sky",
      "presentationKind": "EFFECT_DOCUMENT",
      "placementId": "valtan.sky.red-vortex",
      "effectAssetId": "effect.valtan.environment.red-vortex-sky",
      "position": [156.03, 74, -122.06],
      "rotationQuaternion": [-1, 0, 0, 0],
      "scale": [1, 1, 1],
      "orientationPolicy": "CAMERA_FACING_WORLD",
      "activationPolicy": "SERVER_PATTERN_WINDOW",
      "activationSetId": "sky.valtan.four-pillars-105",
      "playbackPolicy": "SERVER_CLOCK_SAMPLE"
    }
  ]
}
```

위 JSON은 field 책임을 보여 주는 schema 예시다. 구현 시 quaternion convention과 scale 범위는 현재
Engine Transform convention을 실측해 validator와 serializer가 같은 순서로 고정한다.

- `DEPLOY_SURFACE_OVERLAY`는 기존 owner placement와 visible state를 따라간다. surface ray나 복제
  transform을 저장하지 않는다.
- 64-bit Deploy placement ID는 JSON number/double로 읽어 정밀도를 잃지 않도록 canonical decimal
  string으로 저장하고 strict uint64 parser로 변환한다.
- `EFFECT_DOCUMENT`는 재사용 가능한 모양을 `effectAssetId`로 참조하고, instance transform과 activation만
  Area 문서가 소유한다.
- `independentEffectId`와 `placementId`는 각각 문서 안에서 unique stable ID다. vector index, pointer,
  Prototype tag는 저장 ID가 아니다.
- Valtan처럼 MapCatalog가 pair를 선언한 Area는 source/runtime 한쪽만 없거나 semantic parity가 깨지면
  fail-closed한다. pair를 선언하지 않은 다른 Area는 기존처럼 static presentation 없음으로 처리한다.

### 11.2 바닥 Surface Detail의 정본

Deploy catalog는 model path, static/anim kind, prototype, overlay 지원 여부만 소유한다. A/B의 실제 색과
강도는 `.mapeffects.json`의 `DEPLOY_SURFACE_OVERLAY` row로 이동한다.

```text
Deploy catalog
  어떤 model/material이 deferred overlay를 지원하는가

mapeffects row
  어느 stable placement owner에 어떤 intensity/color/mask 값을 적용하는가
```

`CDeployPropRuntime`은 object를 만들기 전에 catalog + placement + surface presentation을 모두 stage하고,
row가 참조한 네 placement가 A/B static asset이며 material 1 emissive를 실제로 가지는지 검증한다. 그 뒤
최종 descriptor를 한 번만 commit한다. 편집 실패 때 live object 일부만 바뀌는 setter loop를 금지한다.

### 11.3 붉은 Vortex Effect 정의

새 direct-authored Product는 다음 identity를 사용한다.

```text
effectAssetId
  effect.valtan.environment.red-vortex-sky

authoringPath
  Effects/Authored/effect.valtan.environment.red-vortex-sky.effect.json

physical source
  Data/Effects/Authored/effect.valtan.environment.red-vortex-sky.effect.json
```

Effect document에는 여섯 source layer를 stable element ID로 모두 둔다. 현재 보이는 네 개는 enabled,
electric/streak 두 개는 disabled로 시작해 사용자가 Effect Detail에서 직접 비교할 수 있게 한다. element는
다음 값을 소유한다.

- mesh/sprite resource와 texture slots
- render order, alpha/additive blend, depth-read, two-sided state
- local position/rotation/scale
- color tint, opacity, emissive/intensity
- UV scale/offset/scroll
- revolution/rotation speed
- radial profile의 inner radius, outer radius, edge softness, aperture strength
- enabled와 document timeline curve

현재 Map 전용 `PRESENTATION_VORTEX_PROFILE` enum을 Effect 문서에 그대로 복사하지 않는다. radial mask 수학을
공유 HLSL include로 추출하고 Effect mesh/sprite material에 generic authored radial fields를 추가한다. 기존
Map shader와 새 Effect renderer가 전환 기간에 같은 함수와 기준값을 소비해 비교 가능하게 한 뒤, Product
switch가 끝나면 Map-only profile binding을 제거한다.

현재 Effect codec은 runtime resource를 `Effect/` 아래로 제한한다. 그러므로 `Map/ValtanPhase` ID를 authored
Effect JSON에 그대로 넣어 validation을 우회하지 않는다. 필요한 6개 WModel/texture 최소 closure만
`Client/Bin/Resources/Effect/Valtan/Environment/RedVortex/...`로 승격하고 Resources-relative ID로 참조한다.
팀 인계에는 이 상대 ID와 실제 물리 폴더, Git dependency closure 포함 여부를 기록한다.

### 11.4 100줄 Server activation 정본

현재 `ValtanCinematicCamera.json`의 sky window는 정확히 다음 네 stage다.

```text
VALTAN_FOUR_PILLARS_105
  TAKEOFF
  YELLOW_ZONE
  TARGET_CONE
  RECOVERY
```

이 문서는 앞으로 visibility/opacity/aperture/rotation 같은 pixel 값을 소유하지 않는다. 각 row는
`activationSetId`, `placementId`, `patternId`, `stageId`, active window, authored timeline offset만 가진다.
실제 visual curve는 Effect document로 이동한다.

```text
Server snapshot pattern/stage/actionStartTick
-> activation window resolve
-> stable placement handle acquire/reuse
-> stage local age + authored timeline offset
-> CEffectObject sample/seek
-> stage replacement/interrupt/death/disconnect/level clear exact stop
```

이렇게 하면 late join도 현재 Server age를 같은 document sample로 변환하고, Tool preview와 Product가 동일한
visual timeline을 소비한다. 109 Pattern 재사용은 이번 요청 범위가 아니며 별도 정본 row 없이 추측해
추가하지 않는다.

## 12. Effect Tool UX와 authoring transaction

### 12.1 All Effects tree

목표 UI는 다음과 같다.

```text
All Effects
  Valtan
    INDEPENDENT EFFECT
      Pattern-owned (2)
        INNER / OUTER 도넛
        플레이어 추적 도끼
      Area / Static Presentation (2)
        발탄 바닥 균열 Emissive [DEPLOY SURFACE]
        발탄 100줄 붉은 Vortex Sky [WORLD EFFECT]
```

기존 `VALTAN_ALL_EFFECTS_INDEPENDENT_EFFECT_IDS` exact 2 검증은 Pattern-owned 검증으로 이름과 의미를
명확히 한다. static row를 C++ array에 추가하지 않고 staged `.mapeffects.json`에서 data-driven으로 읽는다.
검색은 independent ID, display name, effect asset ID, placement ID, owner placement ID, activation pattern/stage를
모두 찾는다.

### 12.2 바닥 row

`Open Surface Detail`은 공통 Effect Detail window 안에 실제 지원 field만 표시한다.

- Emissive Intensity
- Emissive Color
- Mask Power
- Preview Enabled
- `Save`, `Reload`, `Reset to Saved`

particle count, emitter loop 같은 존재하지 않는 field를 가짜로 보여 주지 않는다. preview는 현재 Valtan
Level의 네 stable Deploy object에 staged packet을 적용하되, 하나라도 owner/state/material validation이
실패하면 네 개 모두 이전 packet을 유지한다. Server destruction state는 Tool이 변경하지 않는다.

### 12.3 하늘 row

`Open Editor`는 일반 v13 Effect Details를 그대로 연다. 별도 `World Placement` section에서 다음 instance
값을 편집한다.

- stable placement ID와 Area ID: read-only identity
- position / quaternion / scale
- camera-facing orientation
- activation set와 playback policy
- `Preview at Placement`, `Play`, `Loop`, `Pause`, `Restart`, `Stop`
- `Save Effect Definition`, `Save Placement`, `Reload`

기존 `STANDALONE_EFFECT` preview intent는 paused Valtan boss `PLAYER_ROOT`를 강제하므로 하늘에 재사용하지
않는다. 새 `STATIC_AREA_PLACEMENT` intent/context가 pending load, dirty-session confirmation, reload, restart,
preview filter와 save hot reload를 끝까지 보존한다. preview root는 mapeffects의 world transform이며 boss
model이나 scene player가 없어도 동작한다.

### 12.4 `New Static World Effect`

정적 Effect 생성은 빈 파일을 즉시 Product에 노출하지 않는다.

1. Tool이 `independentEffectId`, `effectAssetId`, `placementId`, Area와 policy 입력을 stage한다.
2. v13 authored draft와 placement candidate를 메모리에서 만든다.
3. 사용자가 resource/template을 선택해 drawable document가 되기 전에는 `Unpublished Static Draft`로만
   표시한다.
4. `Register in Area`가 document parse/drawable, catalog collision, resource, placement transform,
   activation reference, GPU preparation을 전부 검증한다.
5. authored JSON + `EffectCatalog.json` row + source `.mapeffects.json`을 one transaction으로 commit하고
   Map publisher가 runtime pair를 교체한다.
6. 어느 단계든 실패하면 새 catalog/placement가 보이지 않고 기존 active preview와 Product target을 유지한다.

이번 red vortex는 기존 6-layer source가 있으므로 구현 단계에서 drawable direct-authored document로 먼저
구성한 뒤 위 registration 경로를 통과시킨다.

## 13. Product runtime 설계

### 13.1 level-owned Effect handle

현재 `CEffectPresentationService::Spawn_WorldRoot`는 non-null Valtan boss weak owner와 finite natural lifetime을
전제로 한다. static placement에 boss를 fake owner로 넘기거나 매우 큰 duration을 넣지 않는다.

새 typed entry point는 다음 책임만 가진다.

```text
Spawn_LevelPlacement
  levelIndex
  stable placementId
  effectAssetId
  root world
  orientation policy
  playback policy
  initial sample time
```

- duplicate key는 `(levelIndex, placementId)`로 거부한다.
- prepared Product target과 scene budget admission을 기존 service에서 그대로 사용한다.
- `SERVER_CLOCK_SAMPLE`은 매 frame respawn하지 않고 같은 handle을 seek/update한다.
- 일반 static loop 지원은 document duration modulo sample 또는 explicit reset으로 구현하며 fake huge lifetime을
  쓰지 않는다.
- `Clear_Level`은 level-owned pending/active handle을 exact-once stop하고 GPU/shared resource ownership을
  기존 정책대로 해제한다.

### 13.2 Map effect runtime

`CMapEffectPresentationRuntime`은 다음 순서로 동작한다.

```text
parse source/runtime document
-> validate area/schema/unique IDs/path/finite transform/tagged union
-> join EffectCatalog or Deploy owner placements
-> prewarm every EFFECT_DOCUMENT asset
-> stage surface packets and level Effect descriptors
-> create all staged resources/objects
-> commit runtime document + handles + surface packet set
```

중간 Effect clone, surface packet, shader resource, activation reference 중 하나라도 실패하면 모두 release하고
이전 hot-reload state를 유지한다. 최초 Level load라면 Valtan activation 자체를 실패시켜 부분 화면으로
진입하지 않는다.

`Level_Loading`의 Product priority queue에는 mapeffects의 unique `effectAssetId`를 포함한다. Level activation
뒤 disk를 다시 읽거나 first cue edge에서 GPU resource를 준비하지 않는다.

### 13.3 바닥 runtime은 기존 owner를 유지

바닥 row는 새 `CEffectObject`를 spawn하지 않는다. staged surface packet이 최종 `CDeployPropObject::DESC`에
합쳐지고 기존 pass 18 draw가 이를 소비한다. `CDeployPropObject::Should_RenderDeferredEmissiveOverlay()`의
`STATIC + INTACT` 조건과 `CDeployPropRuntime::Set_States()` rollback은 유지한다. 따라서 84/30줄 붕괴,
collision, navigation, Server mutation 계약은 바뀌지 않는다.

### 13.4 기존 6-proxy sky path의 종료

새 Effect가 준비되기 전에는 기존 sky를 삭제하지 않는다. 전환은 다음 gate를 통과한다.

1. 새 document 6-layer resource/material/timeline이 harness에서 drawable이다.
2. Tool placement preview가 submitted/committed draw를 만든다.
3. 105 네 stage와 late join sample이 new handle을 사용한다.
4. 동일 occurrence에서 old Map proxy와 new Effect가 동시에 보이지 않는 source contract를 통과한다.
5. 그 뒤 `Ready_ValtanSkyPresentation`, `Apply_ValtanSkyPresentation`, hardcoded layer policy와 Map-only profile
   binding을 제거하거나 non-Product reference로 격리한다.

원본 evidence 파일과 six-layer stable identity는 provenance로 남길 수 있지만 Product draw owner는 하나만
존재해야 한다.

## 14. 저장·실패·rollback 계약

### 14.1 Effect definition Save

기존 direct-authored CAS + `Reload_SelectedProductEffect`를 재사용한다. 현재 hot reload admission count가 boss
cue 위주이므로 runtime consumer count를 다음 source의 union으로 일반화한다.

- clip/stage Product cue
- combat-object visual
- Area `EFFECT_DOCUMENT` placement

catalog revision, selected prepared target, active occurrence와 unrelated prepared target은 실패 시 그대로
유지한다. red vortex가 Area consumer라는 이유로 `Count_ProductCueMappings == 0`에 막히면 안 된다.

### 14.2 Placement/Surface Save

- source `.mapeffects.json`의 baseline canonical text/hash와 current disk를 비교한다.
- candidate 전체 문서를 다시 parse/validate/stage한다.
- source replace와 runtime publish를 기존 Map file-set transaction 안에서 수행한다.
- publish 중간 실패 injection 시 source/runtime와 현재 live runtime 모두 이전 byte/hash를 유지한다.
- surface preview의 네 owner 중 하나만 갱신되는 partial commit을 금지한다.

### 14.3 실패 상태 표시

Tool은 다음 상태를 서로 구분해 표시한다.

- document parse/drawable 실패
- resource/prewarm 실패
- placement/owner/activation validation 실패
- scene budget admission 실패
- CAS conflict
- runtime hot reload rollback 성공/실패
- Server activation queued/accepted/rejected/interrupted

로컬 `Preview at Placement` 성공을 Server Product 재생 성공이나 visual PASS로 표시하지 않는다.

## 15. G별 구현 순서

### G01. 실측 고정과 regression fixture

- 1번 바닥의 exact Deploy asset/placement/material/pass 18 계약을 fixture로 고정한다.
- 3번 source의 6 layer, 4 visible/2 hidden, render order, texture identity와 current hardcoded 값을 receipt로
  기록한다.
- `fx_a_atypical_013_cl.dds`는 candidate evidence일 뿐 sky identity가 아님을 test/document에 고정한다.
- 현 Product activation이 109가 아닌 `VALTAN_FOUR_PILLARS_105` 네 stage임을 고정한다.

### G02. `.mapeffects.json` schema와 publisher

- `CMapEffectDocument` parser/validator/serializer를 추가한다.
- MapCatalog `sourceEffects/effects` pair와 `Publish-MapAuthoring.ps1` transaction에 포함한다.
- duplicate, wrong version/area, unknown field, absolute/drive/`..` path, non-finite transform, invalid quaternion,
  wrong tagged-union field, missing source/runtime pair를 fail-closed한다.
- 새 C++ 파일은 `Client.vcxproj`와 `.filters`에 물리 폴더 기준으로 등록한다.

### G03. 바닥 static surface authoring

- A/B 실제 intensity/color/mask 값을 mapeffects surface row로 이동한다.
- Deploy runtime이 네 stable owner에 한 staged packet을 적용하도록 한다.
- Effect Tool의 typed Surface Detail UI와 atomic preview/save/reload를 연결한다.
- duplicate draw/decal을 추가하지 않고 붕괴 state와 pass 18을 유지한다.

### G04. red vortex resource/material Effect

- 최소 6-layer asset closure를 `Resources/Effect/Valtan/Environment/RedVortex`로 준비한다.
- shared radial HLSL과 generic authored material fields를 Effect renderer에 추가한다.
- 6개 stable element와 4-stage 누적 timeline을 가진 v13 direct-authored Effect를 생성한다.
- EffectCatalog 등록 전 source identity, drawable, adapter, blend/depth, draw submission을 검증한다.

### G05. level placement lifetime와 prewarm

- `Spawn_LevelPlacement`와 level-owned handle/loop/sample/clear를 기존 presentation service에 추가한다.
- `CMapEffectPresentationRuntime`이 parse -> validate -> stage -> commit과 rollback을 소유한다.
- Level Loading priority prewarm, scene budget, duplicate placement admission과 teardown을 연결한다.

### G06. 105 Server window 연결

- cinematic sky row에서 pixel curve/asset pair를 제거하고 stable activation set + placement + timeline offset만
  남긴다.
- authoritative pattern/stage/action age를 document sample로 변환한다.
- stage switch, late join, rewind/replacement, interrupt, death, disconnect, level clear를 exact handle lifecycle로
  검증한다.

### G07. All Effects static projection와 생성 workflow

- 기존 Pattern-owned exact 2와 Area/static data rows를 합성한다.
- surface와 world Effect가 같은 root에서 서로 맞는 detail editor/button만 제공한다.
- `STATIC_AREA_PLACEMENT` preview intent와 world placement panel을 pending/dirty/restart/save 경로에 연결한다.
- `New Static World Effect -> draft -> Register in Area` transaction을 구현한다.

### G08. 기존 sky proxy Product path retirement

- 새 Effect consumer가 실제 Product owner가 된 뒤 old six-proxy mutation을 끈다.
- Map/Effect double draw, old hardcoded policy 잔존, retired asset ID 소비를 source/harness로 거부한다.
- reference/provenance와 Product runtime input을 문서에서 명확히 구분한다.

### G09. 문서·빌드·사용자 검증 인계

- `AREA_DATA_LAYER_GUIDE.md`에 mapeffects optional pair와 두 tagged kind를 추가한다.
- public Effect/Area 계약이 바뀐 부분만 TEAM handbook/CLAUDE에 반영한다.
- 실제 실행한 validator/harness/build와 미실행 visual smoke를 RESULT에 분리한다.
- 사용자가 직접 누를 정확한 Effect Tool 경로와 105 Pattern 재생 절차를 인계한다.

## 16. 예상 파일과 책임

| 구분 | 파일 | 책임 |
|---|---|---|
| 신규 | `Client/Public/MapEffectDocument.h` | strict Area static presentation record/tagged union |
| 신규 | `Client/Private/MapEffectDocument.cpp` | JSON parse/validate/serialize, source/runtime parity |
| 신규 | `Client/Public/MapEffectPresentationRuntime.h` | staged runtime와 level-owned handle 공개 계약 |
| 신규 | `Client/Private/MapEffectPresentationRuntime.cpp` | Effect/Deploy join, prewarm, commit/rollback/clear |
| 수정 | `Client/Default/Client.vcxproj`, `.filters` | 위 신규 C++ 물리 폴더 등록 |
| 수정 | `Client/Public/Effect_Tool.h` | static item/preview context/placement draft state |
| 수정 | `Client/Private/Effect_Tool.cpp` | All Effects 합성, surface/world detail, creation/save UX |
| 수정 | `Client/Public/Effect_PresentationService.h` | level-placement descriptor/handle entry point |
| 수정 | `Client/Private/Effect_PresentationService.cpp` | prewarm/admission/sample/loop/teardown/rollback |
| 수정 | `Client/Private/Effect_DocumentCodec.cpp` 및 관련 schema | authored radial material field validation |
| 수정 | `Client/Private/Effect_DocumentRenderer.cpp` | generic radial material packet과 actual draw |
| 신규/수정 | Effect shader shared `.hlsli`, Effect mesh/sprite shader | radial aperture/ring/cloud 공통 수학 |
| 수정 | `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl` | 전환 중 shared math 소비, 이후 Map-only profile 정리 |
| 수정 | `Client/Public/DeployPropObject.h`, `Client/Private/DeployPropObject.cpp` | staged surface color/intensity/mask 실제 소비 |
| 수정 | `Client/Private/DeployPropRuntime.cpp` | stable owner join과 atomic surface packet commit |
| 수정 | `Client/Private/Level_Loading.cpp` | mapeffects Product target priority prewarm |
| 수정 | `Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp` | runtime owner 연결, old sky path retirement |
| 수정 | `Data/Maps/MapCatalog.json` | Valtan sourceEffects/effects exact pair |
| 신규 | `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json` | static source 정본 |
| 생성 | `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapeffects.json` | publisher runtime 산출물 |
| 수정 | `Tools/MapPipeline/Publish-MapAuthoring.ps1` | static source/runtime atomic publish |
| 신규 | `Data/Effects/Authored/effect.valtan.environment.red-vortex-sky.effect.json` | 6-layer visual/timeline 정본 |
| 수정 | `Data/Effects/EffectCatalog.json` | red vortex Product admission |
| 수정 | `Data/Encounters/Valtan/ValtanCinematicCamera.json` | 105 activation window + timeline offset only |
| 수정 | `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` | optional layer와 publisher/runtime 소비 계약 |

실제 구현 전에 현재 dirty worktree의 같은 파일 변경을 다시 비교하고 다른 팀원 수정 줄을 덮어쓰지 않는다.

## 17. 자동 검증 추가분

### 17.1 schema/publisher

```powershell
python -B Tools/MapPipeline/test_map_effect_presentation_contract.py
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId LV_LUT_HEARTRB_ED -Mode Validate
```

focused test는 정상 문서 외에 다음 실패를 반드시 만든다.

- wrong schema/version/area
- duplicate independent/placement ID
- unknown EffectCatalog asset, missing authored file, non-drawable Effect
- absolute/drive-qualified/`..` resource path
- NaN/Inf, zero/negative scale, invalid quaternion
- missing Deploy owner, wrong asset/material/state, duplicate surface owner
- invalid activation set/pattern/stage/timeline offset
- optional pair 한쪽 누락과 source/runtime semantic mismatch
- `FailureAfterPromote`에서 source/runtime byte/hash rollback

### 17.2 Effect/runtime harness

```powershell
python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
python -B Tools/EffectPipeline/test_valtan_static_independent_effect_contract.py

MSBuild Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj `
  /m /p:Configuration=Debug /p:Platform=x64
Tools/EffectRenderContractHarness/Bin/Debug/EffectRenderContractHarness.exe

MSBuild Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj `
  /m /p:Configuration=Debug /p:Platform=x64
Tools/ActionPresentationTimelineHarness/Bin/Debug/ActionPresentationTimelineHarness.exe
```

검증 receipt는 최소 다음을 증명한다.

- Pattern-owned independent exact 2는 유지되고 static rows는 별도 data projection이다.
- red vortex 6 element, 4 enabled/2 disabled, stable render order와 resource가 resolve된다.
- fixed world position은 움직이지 않지만 authored UV/revolution sample은 진행한다.
- 105 네 stage의 initial sample/late join이 deterministic하다.
- stage switch가 handle을 중복 spawn하지 않고 interrupt/death/disconnect/level clear가 exact stop한다.
- scene budget reject나 prepare 실패가 기존 prepared target/active occurrence를 보존한다.
- floor 네 owner가 INTact일 때만 같은 packet을 소비하고 DESPAWNED 뒤 overlay draw가 0이다.
- old Map proxy와 new Effect가 같은 Product occurrence에 동시에 submit되지 않는다.
- submitted draw와 committed draw가 모두 0보다 크다. 이 수치는 pixel fidelity 합격을 뜻하지 않는다.

### 17.3 build/regression

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Release
git diff --check
```

Engine public header나 shared shader include 위치를 변경하면 Engine Debug/Release, `UpdateLib.bat` 두 구성,
Client Debug/Release까지 실행한다. Release에서 제외된 Development smoke를 PASS로 대체하지 않는다.

## 18. 사용자 수동 시각 검증 절차

에이전트는 Client/UI를 자율 실행하거나 screenshot을 만들지 않는다. build와 runtime 준비가 끝난 뒤 사용자가
직접 다음을 확인한다.

### 18.1 바닥 균열

1. `Server + Client` Debug profile로 Valtan에 진입한다.
2. `F1 -> Effect Tool -> All Effects -> Valtan -> Independent Effect -> Area / Static Presentation`을 연다.
3. `발탄 바닥 균열 Emissive`에서 intensity/color/mask를 바꾸고 네 sector가 함께 preview되는지 본다.
4. Save 후 Level 재진입에서도 값이 유지되는지 본다.
5. 84/30줄 붕괴 뒤 사라진 sector의 균열도 함께 사라지는지 확인한다.

### 18.2 붉은 Vortex Sky

1. `발탄 100줄 붉은 Vortex Sky`에서 `Open Editor`와 `Preview at Placement`를 누른다.
2. 여섯 element를 solo/toggle하며 cloud, ring, aperture, hidden electric/streak를 구분한다.
3. scale, opacity, burgundy color, radial edge, UV/rotation을 조절해 3번 reference와 대조한다.
4. `Play Server Activation`으로 실제 `VALTAN_FOUR_PILLARS_105`를 요청한다.
5. TAKEOFF -> YELLOW_ZONE -> TARGET_CONE -> RECOVERY에서 끊김/중복/순간 재시작 없이 이어지는지 본다.
6. Pattern 종료·중단·사망·disconnect 뒤 vortex가 정확히 정리되는지 본다.
7. 2번의 일반 하늘과 3번의 100줄 하늘 전환 시 old proxy와 new Effect가 겹쳐 두 배로 밝아지지 않는지 본다.

사용자의 서면 관찰 전에는 first pixel, eye smoke, reference match 또는 visual PASS를 RESULT에 완료로 기록하지
않는다.

## 19. 정적 Independent 확장의 완료 조건

- 기존 Server-owned independent 두 row와 그 parser/publisher 권위가 그대로 유지된다.
- All Effects가 hardcoded third row가 아니라 `.mapeffects.json`의 Area/static rows를 data-driven으로 표시한다.
- 바닥 균열은 실제 Deploy surface owner와 destruction state를 유지하면서 Effect Detail에서 실제 runtime
  intensity/color/mask를 저장·재로드할 수 있다.
- red vortex는 단일 DDS가 아니라 6-layer direct-authored Effect로 생성되고 EffectCatalog와 stable Area
  placement에 등록된다.
- Tool preview와 Product가 같은 authored Effect/material/timeline을 소비한다.
- 105 Server stage는 activation/sample/stop 권위만 소유하고 pixel tuning 값은 Effect document가 소유한다.
- boss fake owner, huge lifetime, per-frame respawn, duplicated Map/Effect draw가 없다.
- source/runtime publish와 hot reload가 parse -> validate -> stage -> commit 및 rollback을 만족한다.
- 관련 publisher, focused harness, Debug/Release regression과 `git diff --check`를 실제로 통과한 결과만 RESULT에
  기록한다.
- 최종 시각 품질은 사용자가 Valtan EXE에서 확인한 뒤에만 완료된다.

## 20. 선택 Effect 삭제 계약

All Effects의 삭제 단위는 화면 행의 소유권에 따라 분리한다.

- `Create Effect` 오른쪽의 `Delete Effect`는 Pattern row가 아니라 그 아래에서 명시적으로 선택한 Effect row에만
  작동한다. 확인 modal 전후에 stable Pattern/Effect/cue identity를 다시 검사한다.
- `[PRODUCT]`는 파일 삭제가 아니라 선택 Pattern의 exact cue 집합 unlink다. 정본 split presentation을 CAS로
  바꾸고 `ValidateV2 -> PublishV2`한 뒤에도 shared Effect asset/catalog와 다른 Pattern 소비는 유지한다.
- `DRAFT_ATTACHED`는 deterministic aggregate ID/path, sidecar baseline, authored document canonical과 Product
  미등록을 확인한 뒤 sidecar row와 exact 파일을 삭제한다. 파일 삭제 실패 시 sidecar를 CAS rollback한다.
- Product 미등록 판정은 화면 cache가 아니라 write/delete를 차단한 complete Product source read set 아래 fresh
  Effect catalog와 Valtan Product graph를 parse해 수행한다. lock/parse/drift 실패는 보존으로 닫는다.
- unsaved Effect/occurrence/detail/Area draft가 있으면 삭제를 막는다. refresh나 target replacement가 발생한 frame의
  임시 vector pointer는 modal에 보존하지 않는다.
- Product unlink publisher는 async non-killing child로 실행한다. timeout은 경고만 하고 Tool/Client 종료도 child를
  terminate하지 않는다. source replace backup은 post-commit byte 검증 후에만 폐기한다.
- Pattern Draft target generation이 교체되면 일반 synchronized update가 sequence를 비우기 전에 동일 Pattern
  timeline을 새 generation으로 다시 stage한다. Create의 durable 2문서 commit과 auto-open 실패는 분리 보고한다.

Focused 검증은 정상 Product unlink, shared asset 보존, Draft exact delete, stale selection/baseline 거부, publish
실패 source rollback, Cancel, active dirty guard와 target-generation 재바인딩을 포함한다.

## 21. 2026-08-27 Valtan Arena 입장 우선 전환

사용자 결정으로 Arena 입장 검증을 붉은 Vortex 시각 검증보다 우선한다. 현재 prospective
`CLevel_ValtanArena::Initialize`는 아직 `LOADING`이 current level인 시점에 Map Effect world admission을
수행하므로 `EFFECT_DOCUMENT` 배치는 target/current level 불일치로 입장을 막는다.

- `valtan.independent-effect.red-vortex-sky`를 Valtan `.mapeffects.json`의 활성 배치에서 제거한다.
- authored Effect와 물리 resource는 삭제하지 않지만 Product/Arena에서는 소비하지 않는다.
- 바닥 균열 `DEPLOY_SURFACE_OVERLAY`는 유지한다.
- source를 publisher로 runtime에 반영하고 source/runtime 동일성, world Effect 0건, Map Effect publisher와
  lifecycle contract를 검증한다.
- 이 전환 동안 Vortex visual PASS는 완료 조건이 아니며, 재도입은 target Level 활성화 이후 admission 경계를
  별도 수직 슬라이스로 닫은 뒤에만 허용한다.

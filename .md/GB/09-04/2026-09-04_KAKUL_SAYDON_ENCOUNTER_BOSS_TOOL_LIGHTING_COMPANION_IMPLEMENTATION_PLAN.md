# 2026-09-04 쿠크세이튼 1관문·전용 도구·통합 조명·건슬링어 AI 최종 구현 계획서

> 문서 종류: 구현 계획서
>
> 상태: P0-1·P0-2 구현 및 자동 검증 완료 / P0-3 독립 KoukuSaydon Composition Workbench 재설계 중
>
> 실측 기준: 2026-09-05, `GB/KoukuSaydon-DataFormat`, `36bcf7fa` + P0-2 검증 완료 worktree
> 역사적 파일명은 같은 작업의 PLAN을 갱신한다는 규칙 때문에 유지한다. 이 문서가 지시하는 신규 저작 이름의 정본은 `KoukuSaydon`이다.

현재 worktree에는 이 계획과 무관한 기존 수정 3건이 있다.

- `Client/Bin/DataFiles/Compositions/Bosses/Valtan.bosscomposition.json`
- `Client/Bin/DataFiles/Compositions/Composition.publish.receipt.json`
- `Client/Bin/DataFiles/Compositions/Sequences/KakulSaydonArena.sequencer.json`

구현자는 위 파일을 임의로 되돌리거나, generated Product를 손으로 고쳐 이번 변경에 섞지 않는다.

---

## 0. 최종 결론

이번 작업은 “기존 발탄 도구에 쿠크 분기를 더하는 일”이 아니다. 다음 네 수직 슬라이스를 명확히 분리해 순서대로 닫는 작업이다.

1. **KoukuSaydon 전용 raid domain**

   저작 데이터, pattern ID, Boss Tool, Action Workbench, draft, save job, Server brain, Client presentation, harness를 발탄과 분리한다.

2. **Rendering Tool과 lighting runtime**

   Scene Profile, Directional Light, Area Point/Spot Light, Light Group, rendering quality, pattern multiplier를 한 화면에서 튜닝하되 각 데이터의 저장 정본은 합치지 않는다.

3. **Gate 1 Server-authoritative mechanics**

   Pizza를 첫 실제 패턴으로 관통시키고 130/110/85/50줄 기믹을 독립적인 수직 슬라이스로 완결한다.

4. **한 명의 Gunslinger companion**

   같은 Server player actor가 전투와 가이드를 모두 수행한다. 챗봇이 응답하지 않아도 authored fallback 대사가 기존 제품 말풍선과 채팅 로그로 즉시 나온다.

핵심 결정은 다음과 같다.

| 항목 | 최종 결정 |
|---|---|
| Sequencer lane | `48px -> 24px`, padding `6px -> 2px`. 별도 48px 호환 모드는 만들지 않는다 |
| Timeline 크기 | 데이터 최대 duration을 늘리는 것이 아니라 **편집 viewport의 가용 면적과 수평 확대 범위**를 늘린다. 기본/최대화 모드, 세로 scroll, Fit을 둔다 |
| 상단 설명 | Composition Sequencer 위의 장문 3줄은 삭제한다. 상시 UI에는 dirty/error/lock/status만 남긴다 |
| Resource 선택 | 실제 사용자 single-click은 즉시 audition한다. 선택은 절대 append/save/dirty를 만들지 않는다 |
| 저작 명령 | `Bind/Replace/Create box`처럼 이름이 드러난 명시적 명령 또는 drag/drop만 문서를 바꾼다 |
| Debug/Product 재생 | Debug의 Valtan/Kouku는 입장 연출 뒤 `HOLD_UNTIL_COMMAND`, Release는 저장된 Product flow를 자동 재생한다. 두 domain은 policy state를 공유하지 않는다 |
| 입장 카메라 | `KAKULSAYDON_G1_ENTRANCE`의 Server stage가 world sequence를 시작하고, 그 Server duration이 끝난 뒤에만 다음 pattern으로 진행한다 |
| 기본 조명 | 삭제하거나 일괄 off하지 않는다. 먼저 Directional/Ambient, Map Group, transient effect, bloom, emissive의 기여도를 분리 진단한다 |
| Pattern 암전 | 별도 dark profile로 교체하거나 light를 끄지 않는다. Gate 1 base profile 위에 상대 multiplier token을 blend-in하고 종료 시 token을 해제한다 |
| Spot/Map Light | Point/Spot, group, blend, owner-token restore를 기믹보다 먼저 완결한다 |
| 발탄과 쿠크 | 같은 창의 target dropdown 두 모드가 아니다. 별도 tool/controller/source/draft/writer/harness다 |
| 공유 허용 범위 | immutable typed view, timeline/graph renderer, transaction primitive, audition transport, fixed-tick stage executor만 공유한다 |
| Gunslinger | 방마다 최대 1명인 sessionless Server player actor. 기존 Gunslinger 전투 수치만 참조하며 수치를 AI JSON에 복제하지 않는다 |
| Dialogue | 정본 파일은 `Data/Dialogue/KoukuSaydon.Gunslinger.json`. `KouKuSaydon`, `Gunsliger` 표기는 사용하지 않는다 |
| Gate 확장 | Gate 1을 실제 Server+Client 계약으로 끝낸 뒤 Gate 2/3, Mario, Bingo를 추가한다. 빈 placeholder 파일은 만들지 않는다 |

---

## 1. 현재 코드·데이터 실측

### 1.1 쿠크세이튼 제품 상태

- `LEVEL::KAKULSAYDON_ARENA`, `WORLD_ID::KAKULSAYDON_ARENA`, 맵 placement, camera shot, world sequence, animation reference는 존재한다.
- Level은 아직 `scene.development.neutral.v1`을 사용한다.
- `LV_LUT_MIDNIGHTC_ED`에는 현재 제품 maplights 문서와 `CMapLightPresentationRuntime` 연결이 없다.
- Gameplay world에는 player spawn과 trigger가 있지만 제품 boss placement가 없다.
- 현재 Boss Composition은 `REFERENCE_ONLY`, pattern 0건이며 Arena Sequencer는 `SHADOW`다.
- BossCatalog/BossProfiles, Server `CKoukuSaydonBrain`, Client 제품 `CKoukuSaydon` presentation이 없다.

따라서 맵과 애니메이션이 보인다는 사실을 raid product가 존재한다는 뜻으로 취급하면 안 된다. 첫 완료 단위는 반드시
`Data -> publisher -> catalog/world -> Server brain -> snapshot -> Client presentation -> tool/harness` 전체를 관통해야 한다.

### 1.2 현재 Sequencer의 정확한 화면

사용자가 편집하는 화면은 `Action Composition Workbench > Composition Sequencer`다. 별도 `CSequencerTool`은 Workbench cache를 읽는 read-only sibling이며 쿠크 source에는 요약만 표시한다.

현재 Workbench는 다음 값 때문에 세로로 과도하게 크다.

- row height 48px
- vertical padding 6px
- canvas minimum 420px
- Resources/Sequencer default height 2배 확대
- 가용 높이보다 420px을 우선하는 `max(420, available)`

기존 contract test도 48px/420px/2배/Append를 정답으로 고정하고 있으므로 C++만 고치지 않고 oracle을 함께 바꿔야 한다.

### 1.3 현재 조명 구조와 “맵에 기본으로 깔린 조명”

- `CRenderingProfileService`는 global quality와 한 개의 active scene profile을 즉시 적용한다.
- scene profile은 directional/ambient 성격의 값, exposure/bloom multiplier, shadow, fog를 가진다. profile blend와 pattern modifier는 없다.
- `CMapLightDocument` v1은 point-only, strict, 최대 64개다.
- Engine `LIGHT`와 `LIGHT_DESC`에는 Directional/Point만 있고 Spot은 없다.
- Kakul map은 maplights runtime을 로드하지 않는다.
- deferred 결과에는 material emissive와 bloom도 밝기에 기여한다.

따라서 현재 밝아 보이는 원인이 “삭제해야 할 map light”라고 단정할 수 없다. 현재 가장 가능성이 높은 순서는 scene directional/ambient,
exposure·bloom, emissive mesh이고, effect transient light가 그 다음이다. LIGHTING 이름의 map mesh는 조명 actor라는 증거가 아니다.

### 1.4 발탄 결합 상태

현재 `CBossTool`과 `CActionCompositionWorkbench`는 이름만 일반적이고 내부에서 Valtan 문서, pattern tree, audition service,
save state를 직접 소유한다. 여기에 `if (Kouku)`와 descriptor만 추가하면 draft·writer·live state가 계속 섞인다.

### 1.5 채팅/말풍선의 선행 누락

- `CMainApp`의 chat input/render admission은 Bern과 Valtan만 허용한다.
- `CLevel_KakulSaydonArena`에는 nameplate/chat-bubble view와 replicated player view render가 없다.
- `S2C_CHAT` 수신은 bubble map만 갱신하고 remote chat log에는 넣지 않는다.

그러므로 “기존 말풍선을 그대로 쓴다”는 것은 시각 디자인을 재사용한다는 뜻이지, 배선이 이미 완성됐다는 뜻이 아니다.
Gunslinger AI보다 먼저 쿠크 레벨에서 사람 채팅부터 end-to-end로 보여야 한다.

### 1.6 GameplayCatalog의 Valtan exact oracle과 전역 fail-close

현재 `CGameplayCatalog::Load_BootstrapPath`는 기존 catalog 전체를 rollback 대상으로 옮기고 새 `Gameplay.bootstrap`을 한 번에
파싱·검증한다. 한 boss/encounter row의 semantic validation이 실패해도 함수 전체가 false가 되어 다른 encounter까지 사용할 수 없다.

또 `GameplayCatalog.cpp`에는 `hasExactValtanHighJumpTypedVolley`, phase action, ghost portal/topology처럼 Valtan의 특정 pattern·stage·수치를
고정하는 exact oracle이 있다. 이런 검사는 공용 stage-action schema가 아니라 **`ENCOUNTER_VALTAN` partition의 제품 불변식**이다.
Kouku row가 같은 공용 action kind를 쓴다는 이유로 Valtan pattern ID/action ID/count를 요구하면 안 된다.

반대로 새 Kouku stage action kind의 enum, parser, field/range grammar는 공용 schema에 한 번 추가한다. schema가 action kind를
`KAKULSAYDON only`로 막는 것이 아니라, Valtan은 그 action을 저작하지 않을 뿐이다. encounter별 허용 결과·exact ID·개수 검사는
각 partition semantic oracle이 담당한다.

---

## 2. 명명 정본과 원자적 마이그레이션

사용자가 요청한 `KoukuSaydon`은 authored data·pattern·tool domain의 유일한 표기다. 기존 public enum과 실제 binary alias는
호환성을 위해 역할별로 구분한다.

| 역할 | 정본 | 예 |
|---|---|---|
| authored 폴더·문서·schema·C++ domain | `KoukuSaydon` | `Data/KoukuSaydon/Gate1`, `KoukuSaydonGate1.gameplay.json`, `CKoukuSaydonBossTool` |
| public/runtime stable ID | `KAKULSAYDON` / `kakulsaydon` | `KAKULSAYDON_G1_PIZZA`, `kakulsaydon.g1.pizza.windup-a` |
| 기존 Level/World public enum | `KAKULSAYDON_ARENA` 유지 | `WORLD_ID::KAKULSAYDON_ARENA` |
| physical Resource alias | `KoukuSaton` 유지 | `Character/KoukuSaton/MN_RPCZ_00/...` |
| physical map root | Area ID | `Map/LV_LUT_MIDNIGHTC_ED/...` |
| 사용자 표시 | 한국어 | `쿠크세이튼 1관문` |

신규 stable ID 예시는 다음으로 고정한다.

- `ENCOUNTER_KAKULSAYDON_G1`
- `BOSS_KAKULSAYDON_G1_KOUKU`
- `boss.kakulsaydon.g1.kouku`
- `KAKULSAYDON_G1_PIZZA`
- `KAKULSAYDON_G1_STAGGER_130`
- `KAKULSAYDON_G1_HEART_PING_110`
- `KAKULSAYDON_G1_DANCE_85`
- `KAKULSAYDON_G1_ROULETTE_50`
- `scene.kakulsaydon.gate1.base.v1`
- `lighting-modifier.kakulsaydon.gate1.mechanic-dark.v1`
- `light-group.kakulsaydon.gate1.ambient`
- `light-group.kakulsaydon.gate1.stage`
- `light-group.kakulsaydon.gate1.fixture`

여기서 사용자가 요청한 “data와 pattern을 `KoukuSaydon`으로 변경”은 authored 물리 경로, 문서명, schema domain,
Tool/C++ domain 이름을 뜻한다. JSON의 `patternId`, boss/encounter/light/profile ID는 wire·catalog·snapshot에서 소비되는
public logical ID이므로 저장소 고정 계약인 `KAKULSAYDON`을 유지한다. 파일명에서 public ID를 추론하지 않는다.

첫 변경 단위에서 다음을 함께 이동·수정한다.

- `Data/Animation/Reference/KakulSaydon -> Data/Animation/Reference/KoukuSaydon`
- `Data/Animation/Authored/KakulSaydon -> Data/Animation/Authored/KoukuSaydon`
- Boss composition을 `Data/Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json`으로
- Arena sequencer를 `Data/Compositions/Sequences/KoukuSaydonArena.sequencer.json`으로
- pipeline을 `Tools/KoukuSaydonPipeline`으로
- 모든 consumer, test fixture, BuildDomains input, project item, reference revision

금지 사항은 다음과 같다.

- 신규 authored 경로·문서명에 `KakulSaydon`, `KouKuSaydon`, `Gunsliger`를 남기지 않는다. 신규 public ID에는 bare `KAKUL_G1_*` 대신 `KAKULSAYDON_G1_*`만 허용한다.
- `KoukuSaton` physical asset을 authored domain 철자로 바꾸지 않는다.
- 사용자 소유 legacy binary root를 자동 삭제하지 않는다.
- `Client/Bin/DataFiles` generated 문서를 rename하면서 손으로 고치지 않는다. publisher가 stage/validate/commit한다.
- 구 이름을 무기한 허용하는 runtime alias fallback을 만들지 않는다. migration 한 번으로 consumer를 닫는다.

---

## 3. Valtan과 KoukuSaydon의 분리 구조

### 3.1 소유 구조

```text
Valtan domain                              KoukuSaydon domain
Data/Valtan/*                              Data/KoukuSaydon/Gate1/*
CValtanPatternSource                       CKoukuSaydonPatternSource
CValtanAuthoringService                    CKoukuSaydonAuthoringService
CValtanBossTool                            CKoukuSaydonBossTool
CValtanActionWorkbench                     CKoukuSaydonActionWorkbench
CValtanBrain                               CKoukuSaydonBrain
CValtan client presentation                CKoukuSaydon client presentation
Valtan-only harness                        KoukuSaydon-only harness

                 shared typed infrastructure only
 BOSS_PATTERN_VIEW / BOSS_STAGE_VIEW / timeline & graph pure renderer
 authoring transaction / scoped audition transport / fixed-tick stage executor
 Server combat primitives / Client occurrence presentation primitives
```

### 3.2 강제 경계

- MainApp는 Valtan/Kouku Boss Tool과 Action Workbench를 별도 객체로 소유한다.
- 한 창에서 boss dropdown으로 mutable state를 갈아 끼우지 않는다.
- 각 audition service instance는 immutable `{worldId, encounterId, bossPlacementId, bossArchetypeId}` scope를 받는다.
- Kouku 요청에 Valtan ID/world/revision이 섞이면 Client admission과 Server admission 모두 거부한다.
- `CValtanPatternTree`, `Save_Valtan*`, `Preview_Valtan*`, `Data/Valtan`, `Tools/ValtanPipeline`을 Kouku 코드가 호출하지 않는다.
- Server는 `CValtanBrain`과 `CKoukuSaydonBrain`을 별도로 유지한다. 선택·체력 기믹·특수 판정은 공유하지 않는다.
- Client는 `CValtan`에 Kouku switch를 추가하지 않고 별도 `CKoukuSaydon`을 둔다.
- 공용 runtime은 실제 Valtan과 Kouku 두 소비자가 동시에 사용하는 작은 계약만 추출한다. 두 번째 거대한 boss framework를 미리 만들지 않는다.
- 공용 audition wire로 이전할 경우 Valtan도 같은 변경에서 이전하고 기존 Valtan 전용 route를 병존시키지 않는다.

모든 신규 C++ 파일은 실제 물리 폴더와 함께 해당 `.vcxproj`, `.vcxproj.filters`, 필요한 harness project에 같은 변경 단위로 등록한다.

---

## 4. Rendering Tool 최종 설계

### 4.1 한 화면, 네 개의 transaction owner

Rendering Tool은 사용자가 조명을 한 흐름으로 비교·튜닝하는 **통합 저작 화면**이다. 그러나 저장 파일을 하나의 mega JSON으로 합치지 않는다.

| 화면 영역 | 저작 정본 | 의미 |
|---|---|---|
| Rendering Catalog | `Data/Rendering/Authored/RenderingProfiles.json` | 같은 catalog/revision 안의 global quality와 scene profiles: directional/ambient, shadow, fog, exposure, bloom |
| Area Lights | `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json` | 월드에 배치한 Point/Spot과 group |
| Pattern Lighting Catalog | `Data/Rendering/Authored/LightingModifiers.json` | base/group multiplier preset과 Point/Spot transient cue의 실제 수치 |
| Pattern timing/binding | `Data/KoukuSaydon/Gate1/KoukuSaydonGate1.presentation.json` | 어느 pattern/stage에서 어떤 modifier/cue preset을 어떤 anchor에 언제 적용할지 |

각 owner는 별도 `Draft/Saved/Published/Active` revision, dirty, validation error, Save 버튼을 가진다. 초기 구현에는
부분 실패를 숨기는 `Save All`을 두지 않는다. 통합 publisher가 필요하면 각 owner의 exact revision을 join한 뒤 전부 유효할 때만
runtime set을 atomic promote한다.

Global Quality와 Scene Profile은 같은 Rendering Catalog transaction이므로 UI tab과 draft subsection만 나누고 revision/Save는 공유한다.
둘을 독립 저장된 것처럼 표시하지 않는다.

### 4.2 Tool 코드와 화면

현재 `CMainApp::RenderRenderingWorkbench`의 대형 함수를 다음으로 추출한다.

- `Client/Public/RenderingTool.h`
- `Client/Private/RenderingTool.cpp`
- `Client/Public|Private/RenderingAuthoringSession.*`
- `Client/Public|Private/SceneLightCoordinator.*`
- `Client/Public|Private/LightingModifierCatalog.*`

MainApp는 construct/update/render와 service wiring만 소유한다. `AlwaysAutoResize`를 제거하고 resizable split workspace를 쓴다.

```text
[Area: LV_LUT_MIDNIGHTC_ED] [Gate 1] [Base Profile] [Modifier Preview] [A/B] [Validate/Publish]
-------------------------------------------------------------------------------------------
Hierarchy                    Viewport                         Details
  Scene Directional          point/spot gizmo                selected type properties
  light-group.*              contribution overlay            owner/revision/validation
    Point ...
    Spot  ...
-------------------------------------------------------------------------------------------
Scene Profile | Area Lights | Groups & Blend | Modifier Preview | Quality | Benchmark
```

- Directional은 scene profile당 정확히 1개다. 위치를 가진 fake light로 배치하지 않고 direction rotation과 profile detail만 편집한다.
- 첫 slice의 Point/Spot 배치는 기존 world picking을 이용한 `Pick Position`, numeric Details, Spot `Aim at picked target`, cone debug draw로 닫는다.
- 저장소에 없는 ImGuizmo를 전제하지 않는다. axis drag handle은 Effect/Map Tool과 공용 typed transform interaction을 실제로 추출하는 후속 slice에서만 추가한다.
- Rendering Tool이 debug input owner일 때만 picking/gizmo를 받는다.
- MapTool의 screen/world picking과 transform command는 공용 utility로 추출할 수 있지만 map light writer UI는 Rendering Tool 하나만 둔다.
- hierarchy에서 enable, solo, mute, duplicate, add/delete, group 이동을 제공한다. solo/mute는 preview token이며 제품 source를 조용히 바꾸지 않는다.
- Details는 type별 exact field만 보인다. unknown/mismatched field는 저장 때 버리지 않고 validator가 거부한다.
- `CRenderingAuthoringSession`은 maplight source revision CAS, typed add/delete/move/update, temp save, exact reparse, atomic promote를 필수로 소유한다.

### 4.3 Map lights v2

대표 데이터 형태는 다음과 같다.

```json
{
  "schema": "lostark.map-light-presentation",
  "formatVersion": 2,
  "areaId": "LV_LUT_MIDNIGHTC_ED",
  "provenance": "PROJECT_AUTHORED",
  "groups": [
    {
      "groupId": "light-group.kakulsaydon.g1.ambient",
      "displayName": "Gate 1 Ambient Fixtures",
      "scopeId": "ENCOUNTER_KAKULSAYDON_G1",
      "enabledByDefault": true,
      "defaultIntensity": 1.0,
      "blendInMs": 300,
      "blendOutMs": 300
    },
    {
      "groupId": "light-group.kakulsaydon.g1.stage",
      "displayName": "Gate 1 Stage Fixtures",
      "scopeId": "ENCOUNTER_KAKULSAYDON_G1",
      "enabledByDefault": true,
      "defaultIntensity": 1.0,
      "blendInMs": 300,
      "blendOutMs": 300
    },
    {
      "groupId": "light-group.kakulsaydon.g1.fixture",
      "displayName": "Gate 1 Fixtures",
      "scopeId": "ENCOUNTER_KAKULSAYDON_G1",
      "enabledByDefault": true,
      "defaultIntensity": 1.0,
      "blendInMs": 300,
      "blendOutMs": 300
    }
  ],
  "lights": [
    {
      "lightId": "light.kakulsaydon.g1.stage-center",
      "kind": "SPOT",
      "groupId": "light-group.kakulsaydon.g1.fixture",
      "enabled": true,
      "position": [0.0, 8.0, 0.0],
      "direction": [0.0, -1.0, 0.0],
      "rangeMeters": 18.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 14.0,
      "outerConeDegrees": 30.0,
      "color": [1.0, 0.72, 0.50, 1.0],
      "brightness": 1.0,
      "priority": 100
    }
  ]
}
```

검증 규칙:

- stable group/light ID unique, 모든 group·scope join 유효
- finite vector/color/range/brightness
- Spot direction normalize, `0 < inner <= outer < 90`
- Point에는 cone/direction 금지, Spot에는 필수
- v2 empty lights는 “profile only area”로 허용
- v1 문서는 in-memory legacy group + Point adapter로 읽어 Valtan source를 억지로 rewrite하지 않음
- authored 총량과 runtime active cap을 분리
- runtime active 64개 초과 시 group/range/frustum 뒤 `priority -> distance -> stable ID`로 결정론적으로 선택하고 진단을 남김

G03-2는 v2 source 생성, `Data/Maps/MapCatalog.json`의 exact `sourceLights/lights` pair, publisher 처리, Client runtime load를
한 transaction으로 추가한다. pair 없이 source 파일만 두어 publisher가 조용히 건너뛰게 하지 않는다. Save는

```text
loaded source revision -> typed draft mutation -> validate -> temp write
-> exact CMapLightDocument reparse -> compare expected revision -> atomic promote
```

순서를 따르고 실패하면 source와 active preview를 모두 이전 admitted 상태로 유지한다.

Engine 변경 지점은 `Engine/Public/Engine_Enum.h`, `Engine/Public/Engine_Struct.h`, `Engine/Private/Light.cpp`,
`Engine/Private/Presentation_Manager.cpp`, canonical `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`이다. Client의 shader copy는

`UpdateLib`/Client build가 만드는 generated copy이므로 직접 정본으로 편집하지 않는다. Spot shader pass는 기존 pass index 사이에
삽입하지 않고 technique 끝에 append해 기존 Combined/Final/overlay/chromatic index를 보존한다. `END`, renderer/static_assert,
compiled-shader closure를 함께 갱신한다.
`SPOT` 추가는 Engine public 계약이므로 `UpdateLib -> Product -> compiled-shader WARP closure`까지 같은 G에서 닫는다.

### 4.4 Base Profile, blend, group, pattern multiplier

Gate 1은 `scene.kakulsaydon.g1.base.v1` 하나를 먼저 만든다. Gate 2/3 base profile은 해당 관문 제품 작업 때 추가하고
지금 빈 파일로 예약하지 않는다.

맵/방이 9개라는 이유로 profile 9개를 먼저 만들지 않는다. profile 수는 **서로 다른 기본 atmosphere 수**로 결정한다.
Card Map이나 Mario 1~4가 같은 directional/ambient/fog/exposure를 쓰면 하나의 base profile을 공유하고, 방별로 다른 fixture는
Area Light group으로 나눈다. 실제로 공기가 달라지는 관문·Mario·Bingo만 해당 수직 슬라이스에서 profile을 추가한다.

짧은 패턴 연출은 scene profile 교체가 아니라 다음 modifier preset을 사용한다.

```json
{
  "schema": "lostark.lighting-modifier-catalog",
  "formatVersion": 1,
  "modifiers": [
    {
      "modifierId": "lighting-modifier.kakulsaydon.g1.mechanic-dark.v1",
      "areaId": "LV_LUT_MIDNIGHTC_ED",
      "sceneMultipliers": {
        "directionalDiffuse": 0.45,
        "directionalAmbient": 0.30,
        "directionalSpecular": 0.55,
        "exposure": 0.85,
        "bloomIntensity": 0.75
      },
      "groupMultipliers": [
        {
          "groupId": "light-group.kakulsaydon.g1.ambient",
          "intensityMultiplier": 0.25
        },
        {
          "groupId": "light-group.kakulsaydon.g1.stage",
          "intensityMultiplier": 1.0
        }
      ]
    }
  ],
  "transientCueDefinitions": [
    {
      "cueDefinitionId": "lighting-cue.kakulsaydon.g1.heart-role-spot.v1",
      "kind": "SPOT",
      "localOffset": [0.0, 8.0, 0.0],
      "direction": [0.0, -1.0, 0.0],
      "rangeMeters": 14.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 10.0,
      "outerConeDegrees": 24.0,
      "color": [1.0, 0.95, 0.82, 1.0],
      "brightness": 4.0
    }
  ]
}
```

위 숫자는 schema 예시와 최초 audition seed이며 최종 visual truth가 아니다. 실제 값은 대표 camera에서 사용자가 A/B로 판정한 뒤 저작한다.

합성은 매 frame immutable base에서 다시 계산한다.

```text
baseScene = Blend(profileA, profileB, baseBlendAlpha)
effectiveScene = baseScene × product(active scene modifier tokens)
effectiveFixture = authored light × base group weight × product(active group modifier tokens)
finalLights = selected effective fixtures + occurrence-owned transient cues
```

- 직전 effective 값에 다시 곱하지 않는다.
- token key는 `{bossEntityId, occurrenceSequence, invocationId}`다.
- 같은 source의 재적용은 replace하고 중첩 증가시키지 않는다.
- stage/pattern end, skip, interrupt, boss despawn, disconnect, level exit, revision change에서 token을 해제한다.
- 해제 시 last effective 값을 수동 복사하지 않고 현재 base에서 다시 계산해 `blendOutMs` 동안 복원한다.
- late join은 Server action start tick/occurrence sequence로 현재 blend progress를 재구성한다.
- Debug preview token은 제품 token과 분리하며 Tool close 또는 input-owner 상실 시 반드시 해제한다.
- profile switch/blend는 관문·방·컷신의 base atmosphere 전환에 사용하고, Pizza/Heart/Dance 암전은 modifier에 사용한다.

이를 위해 `CRenderingProfileService`의 admitted base catalog/authored draft와 composed runtime 값을 분리한다.
pattern modifier는 catalog를 바꾸는 기존 profile apply API를 호출하지 않고, 저장값을 전혀 변경하지 않는
`Apply_ComposedRuntime` 경계를 사용한다. `CSceneLightCoordinator`는 directional/shadow commit과 **정확히 한 개의 active area-fixture
transient provider**를 하나의 target activation으로 stage한다. target profile 또는 declared maplight가 실패하면 새 Level의 lighting
activation 전체를 rollback하며, 새 Area에 이전 Area fixture만 남기지 않는다.

`LightingModifiers.json`은 다음 경로로 제품화한다.

```text
Data/Rendering/Authored/LightingModifiers.json
  -> Tools/RenderingPipeline/Publish-LightingModifiers.ps1 -Mode Validate|Publish
  -> profile/map-group/cue exact join + parse/validate/stage/atomic commit
  -> Client/Bin/DataFiles/Rendering/LightingModifiers.runtime.json
  -> Client/Bin/DataFiles/Rendering/LightingModifiers.publish.receipt.json
  -> CLightingModifierCatalog -> CSceneLightCoordinator
```

publisher는 unknown field, duplicate modifier/cue ID, dangling Area/group, kind-specific field mismatch, invalid cone/vector/range를
거부하고 실패 시 runtime과 receipt의 이전 bytes를 유지한다. Client project의 `96.DataFiles`에는 authored JSON을 `None`으로만 등록한다.

Pattern presentation은 값 자체가 아니라 invocation을 소유한다.

```json
{
  "patternId": "KAKULSAYDON_G1_HEART_PING_110",
  "stageId": "SUMMON",
  "lightingInvocations": [
    {
      "invocationId": "heart-dark",
      "modifierId": "lighting-modifier.kakulsaydon.g1.mechanic-dark.v1",
      "trigger": "STAGE_ENTER",
      "transitionMs": 600,
      "restorePolicy": "PATTERN_END"
    }
  ],
  "lightCues": [
    {
      "cueInvocationId": "heart-role-spot",
      "cueDefinitionId": "lighting-cue.kakulsaydon.g1.heart-role-spot.v1",
      "anchor": {"kind": "SUMMON_ROLE_EACH", "roleSet": "HEART_PING"},
      "transitionMs": 250,
      "restorePolicy": "PATTERN_END"
    }
  ]
}
```

Server는 profile/light/group ID를 알지 않는다. Server의 committed pattern/stage occurrence를 Client presentation이 typed cue view와 join한다.
publisher의 dangling profile/group/anchor는 fail-close하고, runtime anchor가 일시적으로 없으면 해당 cue만 격리해 raid gameplay는 유지한다.

### 4.5 기본 맵 조명 처리와 튜닝 순서

Rendering Tool에 preview-only `Contribution Isolation`을 둔다.

- Scene Directional/Ambient
- Area Point/Spot groups
- Effect transient lights
- Bloom/Exposure
- Emissive visualization 또는 진단

현재 Area fixture와 Effect light가 같은 transient queue를 쓰므로 단순 UI checkbox만으로는 둘을 분리할 수 없다. G03에서
`TRANSIENT_LIGHT_SOURCE_CHANNEL { AREA_FIXTURE, EFFECT, PATTERN_CUE, DEBUG_PREVIEW }` metadata를 transient submission에 추가하고,
Debug Rendering Tool에만 channel별 suppression/count diagnostics를 제공한다. 이 channel은 damage·network·저장 identity가 아니며
Release의 light 선택 순서를 바꾸지 않는다. emissive는 transient channel이 아니므로 별도 deferred debug visualization으로만 격리한다.

판정 후의 처리 원칙은 다음과 같다.

- 원인이 Directional/Ambient면 Gate 1 base profile을 조정한다.
- 실제 fixture Point/Spot이면 light/group의 enabled/defaultIntensity를 조정한다.
- emissive mesh면 material/emissive 경로를 별도로 조정한다. map light flag로 가리지 않는다.
- MapCatalog가 light pair를 선언하지 않은 area는 정상적인 profile-only area다.
- pair를 선언했는데 source/runtime이 누락 또는 오염이면 기존 admitted lighting을 유지하고 fail-close한다.
- light를 “안 보이게 하려고” raw map mesh나 source row부터 삭제하지 않는다.

실제 튜닝 순서는 고정한다.

1. 대표 Gate 1 camera와 character/boss 위치를 고정한다.
2. contribution isolation으로 base 원인을 분리한다.
3. Directional diffuse/ambient와 색을 먼저 맞춘다.
4. shadow direction, bias, range를 맞춘다.
5. fixture Point group을 배치·조정한다.
6. Spot group과 cone/anchor를 맞춘다.
7. exposure/bloom은 마지막에 맞춘다.
8. base가 깨끗해진 뒤 Pizza/Heart/Dance dark modifier를 A/B 튜닝한다.
9. VFX/emissive를 다시 켜고 clipping/과노출을 조정한다.
10. benchmark와 사용자 visual smoke로 확정한다.

---

## 5. Composition Sequencer와 Resource audition

### 5.1 압축 규격

- `TIMELINE_ROW_HEIGHT = 24.f`
- `TIMELINE_BLOCK_VERTICAL_PADDING = 2.f`
- hit rectangle은 row 전체 24px을 사용해 box 내부 그래픽이 작아져도 선택·drag가 끊기지 않게 한다.
- 고정 `TIMELINE_CANVAS_MINIMUM_HEIGHT = 420.f`를 제거한다.
- 기본 canvas는 `laneCount * 24 + ruler`를 가용 viewport 안에서 clamp하고 내부 세로 scroll을 사용한다.
- `Maximize Timeline`은 현재 Tool viewport의 최대 96%까지 사용하고 Details/Resources는 collapse 또는 overlay로 전환한다.
- `Fit`은 전체 duration을 현재 폭에 맞춘다.
- horizontal zoom 상한은 500에서 1200px/s로 올린다.
- 저장 데이터 duration 안전 상한 600,000ms는 유지한다. “크게 본다”와 “무한 duration을 허용한다”를 섞지 않는다.
- 좌우 column은 고정 비율보다 최소 폭 + draggable splitter를 사용해 중앙 timeline 폭을 우선 확보한다.
- Resources/Sequencer default height 2배 확대는 제거한다.
- 상단의 장문 `TextDisabled` 3줄은 삭제하고 추가 설명은 tooltip 또는 Advanced Help로 이동한다.
- box를 편집하는 유일한 기본 화면은 Composition Sequencer다. 별도 read-only `CSequencerTool`은 기본 focus 목록에서 빼고
  `Composition Source Inspector (Advanced)`로 명확히 표시하거나, 고유 consumer가 사라진 뒤 제거한다.

상시 표시 정보는 `source revision / draft dirty / validation error / writer lock / audition status` 한 줄뿐이다.

### 5.2 single-click audition 계약

모든 Resource tab은 다음 공통 규칙을 따른다.

1. 실제 사용자가 leaf `Selectable`을 click했을 때만 audition한다.
2. filter/reload가 첫 행을 자동 선택한 경우에는 audition하지 않는다.
3. 새 후보의 resource와 playback state를 먼저 validate/stage한다.
4. stage가 성공했을 때만 같은 channel의 old preview를 멈추고 new preview로 atomic replace한다.
5. 같은 항목을 다시 click하면 성공한 replace로 0ms부터 restart한다.
6. 성공한 preview만 viewport/input owner를 claim한다.
7. 실패하면 기존 preview를 유지하고 선택 행 옆에 구체적 미지원/실패 이유를 표시한다.
8. click은 draft generation, dirty, box 생성, append, save를 절대 발생시키지 않는다.

| Resource | single-click 결과 |
|---|---|
| Animation | 현재 Kouku preview actor에서 clip/sequence를 즉시 재생 |
| Effect | preview anchor에서 effect를 즉시 재생 |
| Sound | 전용 bounded preview channel에서 즉시 재생, 다음 선택 시 이전 sound 정지 |
| Camera shot/sequence | Tool viewport에서 즉시 재생, Esc/Stop으로 복원 |
| 아직 지원하지 않는 항목 | 선택 상태와 정확한 미지원 이유만 표시; 재생 성공을 가장하지 않음 |

`Append to Stage Slots`를 선택 동작과 action strip에서 제거한다. 저작은 다음처럼 분명한 명령만 남긴다.

현재 Camera tab에는 resource leaf가 없으므로 G02에서 Area의 typed CameraShot/WorldSequence 목록을 열거하고, stable ID별
bounded preview player와 camera restore owner를 추가한다. `Open Camera Tool` 버튼만 둔 채 Camera single-click 완료로 기록하지 않는다.

- `Bind Selected Resource to Selected Box`
- `Replace Selected Box Resource`
- `Create Box on <Lane>`
- lane drag/drop

Kouku preview actor가 제품으로 들어오기 전에는 Kouku animation click이 “선택됨 / preview actor unavailable”을 보여야 한다.
G04에서 `CKoukuSaydon` preview presentation이 연결된 뒤 같은 audition interface의 실제 소비자로 닫는다.

### 5.3 Lighting lane

Kouku Workbench에는 `LIGHTING` lane을 추가한다. box는 다음만 저장한다.

- invocation/cue stable ID
- start/stop stage 또는 offset
- transition timing
- restore policy

multiplier, color, brightness, group 정의를 Composition 안에서 중복 편집하지 않는다. Lighting box 선택 시 Rendering Tool의 해당
modifier/group/cue로 deep-link하고, Rendering Tool 저장 owner가 값을 관리한다.

---

## 6. KoukuSaydon Gate 1 데이터와 제품 구조

### 6.1 직접 저작하는 정본

```text
Data/KoukuSaydon/Gate1/
  KoukuSaydonGate1.gameplay.json
  KoukuSaydonGate1.presentation.json
  KoukuSaydonGate1.combatobjects.json
  KoukuSaydonGate1.debugaudition.json

Data/Animation/Reference/KoukuSaydon/*.actionreference.json
Data/Animation/Authored/KoukuSaydon/*.actionbindings.json
Data/Animation/Authored/KoukuSaydon/*.patternbindings.json
Data/Effects/V2/Bindings/BOSS_KAKULSAYDON_G1_KOUKU.effectv2bindings.json
Data/Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json
Data/Compositions/Sequences/KoukuSaydonArena.sequencer.json
```

네 핵심 source는 exact schema를 사용한다.

| 파일 | schema/version | exact root owner |
|---|---|---|
| `KoukuSaydonGate1.gameplay.json` | `lostark.kouku-saydon-gameplay-authoring` v1 | `encounterId`, `bossArchetypeId`, `brainKind`, `sourceRevision`, `decisionModel`, `patterns` |
| `KoukuSaydonGate1.presentation.json` | `lostark.kouku-saydon-pattern-presentation-authoring` v1 | `encounterId`, `bossArchetypeId`, `gameplayRevision`, pattern/stage별 animation/effect/sound/camera/lighting invocation |
| `KoukuSaydonGate1.combatobjects.json` | `lostark.kouku-saydon-combat-object-authoring` v1 | `encounterId`, Server hit/movement/lifetime/damage-profile 참조를 가진 `objects` |
| `KoukuSaydonGate1.debugaudition.json` | 공용 `lostark.boss-debug-audition` v1 | `worldId`, `encounterId`, `bossPlacementId`, `gameplayRevision`, `orderedPatternIds` |

gameplay의 `decisionModel`은 exact `productFlow`, `normalPool`, `healthMechanics`를 소유한다. pattern은 stable ID/category/source action,
selection policy와 ordered stages를, stage는 stable stage/action ID, kind, duration, next/outcome edges, typed stage actions와
combat-object references를 소유한다. presentation은 같은 pattern/stage를 exact join할 뿐 damage·judgement를 다시 정의하지 않는다.
unknown root/stage/action field, duplicate ID, dangling edge, gameplay/presentation revision mismatch를 모두 거부한다.

`brainKind: KAKULSAYDON`의 유일한 authored owner는 gameplay root이고 projector가 Product로 복사한다. 첫 typed grammar가 들어갈 때
generated encounter는 `lostark.encounter-profile` v5로 올려 `brainKind`, decision model, typed stage action/outcome을 싣는다.
기존 Valtan v4를 조용히 v5로 해석하지 않으며, 같은 publisher 변경에서 explicit v4 adapter와 parity fixture를 둔다.
생성되는 combat-object/rotation 문서는 각각 `lostark.kakulsaydon-combat-objects` v1,
`lostark.kakulsaydon-pattern-rotations` v1을 사용한다.

위 신규 JSON과 `Data/AI`, `Data/Dialogue` 원본은 `Client/Default/Client.vcxproj`와 `.filters`의 `96.DataFiles` 아래
`None` 항목으로만 등록한다. build output Content나 프로젝트별 복사본을 만들지 않는다.

공용 catalog와 world에서 바뀌는 정본:

- `Data/Actors/BossCatalog.json`
- `Data/Balance/BossProfiles.json`
- 필요한 `Data/Balance/DamageProfiles.json`
- `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`
- `Data/Maps/MapCatalog.json`

Gameplay world에는 disabled `boss.kakulsaydon.g1.kouku` placement와 이를 활성화하는 typed `activateEncounter` trigger를 둔다.
`playerSpawn.archetypeId`는 계속 null이며 실제 class를 placement가 소유하지 않는다.

현재 world의 circus-finale trigger는 `playSequence`를 직접 실행한다. G04 world transaction에서 그 action을 **정확히 하나의
`activateEncounter`로 교체**하고, 같은 sequence를 시작하는 유일한 owner를 Entrance Server stage로 바꾼다. direct trigger와
Entrance stage를 병존시켜 sequence를 두 번 재생하지 않는다.

### 6.2 생성물

다음은 publisher만 교체할 수 있다.

- `Data/Encounters/KoukuSaydon/KoukuSaydonGate1Encounter.json`
- `Data/Encounters/KoukuSaydon/KoukuSaydonGate1CombatObjects.json`
- `Data/Encounters/KoukuSaydon/KoukuSaydonGate1PatternRotations.json`
- `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`
- `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.maplights.json`
- `Client/Bin/DataFiles/Rendering/*.runtime.json`
- `Client/Bin/DataFiles/Compositions/...`

`Publish-Compositions.ps1`은 join/manifest를 publish할 뿐 gameplay bootstrap의 두 번째 writer가 되지 않는다.
`Publish-GameplayBalance.ps1`이 모든 domain candidate가 유효할 때 최종 bootstrap을 한 번 stage/commit한다.

### 6.3 데이터와 호출 흐름

```text
Kouku gameplay/presentation/combatobjects authoring
  -> KoukuSaydon domain validator/projector
  -> BossCatalog/BossProfiles/World exact join
  -> one Gameplay.bootstrap atomic commit
  -> GameRoom routes brainKind KAKULSAYDON
  -> CKoukuSaydonBrain selects pattern and owns mechanic verdict
  -> shared fixed-tick stage/combat-object runtime
  -> Shared spawn/snapshot/action events
  -> CKoukuSaydon presentation
  -> animation/effect/camera/lighting occurrence consumers

CKoukuSaydonBossTool / CKoukuSaydonActionWorkbench
  -> scoped debug audition request with exact source/runtime revision
  -> Server KAKULSAYDON_ARENA admission
  -> same Product brain/stage runtime
  -> typed lifecycle/result
```

Tool preview를 위해 Client local boss gameplay를 새로 만들지 않는다.

재생 정책과 입장 연출은 다음으로 고정한다.

- encounter activation 뒤 첫 Product pattern은 `KAKULSAYDON_G1_ENTRANCE`다.
- 그 stage ENTER의 typed `PLAY_WORLD_SEQUENCE`가 기존 world-sequence broadcast 경로를 사용한다.
- published world-sequence duration과 Server stage duration을 validator가 exact join한다.
- Client의 카메라 종료 callback이 Server pattern을 시작하지 않는다. Server stage clock이 다음 pattern 진입 시점을 소유한다.
- Client camera/resource 재생 실패는 해당 presentation만 격리하며 Server flow를 멈추거나 로컬 pattern으로 대체하지 않는다.
- Debug는 Entrance 종료 뒤 `HOLD_UNTIL_COMMAND`로 대기한다.
- Release는 Entrance 뒤 저장된 Product flow를 순서대로 자동 재생하며 Debug audition packet을 거부한다.
- Valtan도 사용자가 요청한 Debug HOLD/Release AUTO 정책을 적용하되, Valtan policy와 queue는 Valtan domain 객체가 따로 소유한다.

### 6.4 Gate 1 패턴

| 패턴 | Server truth | Presentation/lighting |
|---|---|---|
| Entrance | encounter 시작과 intro timing | world sequence/camera; 필요할 때만 base profile blend |
| Pizza | source action 4219714, 6 stage, sector combat object와 safe-sector variant. normal pool/audition baseline이며 근거 없는 HP threshold를 붙이지 않음 | WINDUP에서 mechanic-dark, sweep telegraph, 종료/abort 시 base restore |
| Stagger 130 | HP 130 crossing once, stagger window·reflection outcome을 Server가 판정 | dark modifier와 warning cue는 gameplay outcome에 종속 |
| Heart Ping 110 | `VANISH -> SUMMON -> JUDGE -> RETURN`; shooter 1/heart 3 summon, 바라보기 판정, partial spawn 전체 rollback | pattern 시작 dark multiplier, summon role별 4 Spot, RETURN/abort/disconnect restore |
| Dance 85 | `INTRO -> STEP_1..3 -> RESOLVE/PUNISH -> RECOVERY`; Saydon summon variant와 Q/W/E/R window를 Server가 판정 | dark multiplier, dancer/stage Spot, step별 cue, recovery restore |
| Roulette 50 | HP 50 crossing once, roulette/card 결과와 damage를 Server가 판정 | stage/fixture group과 필요한 cue; 시각 결과가 Server verdict를 대신하지 않음 |

한 fixed tick에 여러 HP threshold를 넘으면 높은 HP threshold부터 안정적인 `order`로 pending queue에 넣고 현재 occurrence 종료 뒤 하나씩 실행한다.
각 mechanic은 encounter generation별 once ledger를 가진다. disconnect/restart 때 이전 generation ledger를 재사용하지 않는다.

Pizza의 action 4219714는 다음 여섯 stage를 첫 vertical slice로 사용한다.

```text
WINDUP_A -> SECTOR_SWEEP_A -> RECOVERY_A
WINDUP_B -> SECTOR_SWEEP_B -> RECOVERY_B
```

animation reference의 `3_01 / 3_07 / 3_09` 두 세트를 authored stage에 join하며, Client animation timing이 Server damage timing을 만들지 않는다.

Gate 1 완결에는 기존 요구의 다음 encounter 공통 상태도 포함한다.

| 공통 상태/패턴 | 소유와 규칙 |
|---|---|
| Card suit assignment | encounter generation 시작에 eligible human만 대상으로 Server가 stable player ID 순서와 authored policy로 배정한다. companion은 제외한다 |
| Madness gauge | 획득·감소·임계·clown form 전이를 Server player state가 소유하고 snapshot으로 복제한다. Client HUD/모델은 read-only 표현이다 |
| Clown form | 별도 playable class로 위장하지 않고 typed temporary form 상태와 허용 skill profile로 처리한다. 실제 model/animation asset admission 없이는 시각 완료로 기록하지 않는다 |
| Card matching | bind 대상, homing card, suit 일치와 실패 damage를 Server가 판정한다. Client projectile/effect 위치는 판정 근거가 아니다 |

카드 아이콘·광기 게이지가 필요하면 `Data/UI` stable slot/image 계약과 `CUIObject` 계열 제품 UI로 구현한다. ImGui를 제품 HUD로 승격하지 않는다.
이 상태들도 실제 첫 소비자가 들어오는 slice에서만 wire/bootstrap field를 추가한다.

### 6.5 Boss Tool과 Action Workbench

`CKoukuSaydonBossTool`은 다음만 소유한다.

- Kouku source/revision/encounter scope
- All Patterns / Current Product Flow
- row의 `[Live]` marker
- `Play Selected`, `Repeat`, `Play Full Pattern`, `Stop After Current`, `Restart Active`, `Next Pattern`, `Revive`
- Server result/lifecycle와 exact request correlation

`CKoukuSaydonActionWorkbench`은 다음만 소유한다.

- Kouku pattern/stage immutable view
- Kouku draft와 authoring transaction
- animation/effect/sound/camera/lighting box
- candidate validation, save, exact revision apply/restart

Valtan과 selection, next queue, dirty flag, undo, writer lock, save path를 공유하지 않는다.

---

## 7. Gunslinger AI와 Dialogue

### 7.1 목표와 역할

방마다 최대 한 명의 `GUNSLINGER` companion을 둔다. 이 actor는 한 객체 안에서 다음 모드를 가진다.

- `GUIDE`: owner를 navigation으로 따라가고 encounter/pattern/stage 안내를 한다.
- `ASSIST`: 같은 Server combat/navigation/damage 계약으로 실제 전투한다. 시간 종료 후 GUIDE로 돌아간다.
- `DISMISSED`: despawn transaction을 수행한다.

MVP에서는 첫 human의 Kouku admission이 commit된 뒤 빈 일반 player spawn slot에 GUIDE 1명을 transactionally 만든다.
`도움!`은 새 actor spawn이 아니라 기존 actor의 ASSIST 전환이다. 현재 입구 spawn과 human-only movePlayer trigger를 우회해
owner 근처에 순간 생성하는 동작은 만들지 않는다.

owner는 생성 당시의 `FIRST_ADMITTED_HUMAN` stable PlayerId로 commit한다. mode 변경/dismiss command는 owner-only이고,
질문은 같은 room의 모든 admitted human에게 rate limit 하에 허용한다. owner가 나가면 이 MVP에서는 재지정하지 않고 despawn한다.

companion은 일반 actor capacity와 spawn slot을 사용한다. 4칸이 모두 차면 생성은 원자적으로 거부하고 human을 내보내거나 5번째 actor를 만들지 않는다.
combat target과 damage의 대상은 될 수 있지만 party/invite/reward/inventory/world transfer/vote/card·dance·heart assignment와
encounter/change-level/gimmick trigger에는 참여하지 않는다. 입구에서 arena까지 이동할 수 있도록 기존 `jump.1~3`의 `movePlayer`에만
typed `actorPolicy: HUMAN_AND_COMPANION`을 저작하고, 다른 trigger의 기본은 `HUMAN_ONLY`로 유지한다.

### 7.2 AI 정본

파일명은 사용자 요청 그대로 `Data/AI/Gunslinger.json`이다.

```json
{
  "schema": "lostark.companion-ai-profile",
  "formatVersion": 1,
  "profileId": "companion.gunslinger.raid-guide.v1",
  "characterClass": "GUNSLINGER",
  "displayNickname": "건슬링어 (AI)",
  "allowedWorldIds": ["KAKULSAYDON_ARENA"],
  "maximumInstancesPerRoom": 1,
  "ownerPolicy": "FIRST_ADMITTED_HUMAN",
  "commandAuthority": "OWNER_ONLY",
  "questionAuthority": "ANY_ADMITTED_HUMAN",
  "ownerLeavePolicy": "DESPAWN",
  "chatCommands": {
    "assist": ["도움!", "/도움"],
    "guide": ["/따라와"],
    "dismiss": ["/그만"],
    "questionPrefix": "@건슬 "
  },
  "guide": {
    "followStartDistanceMeters": 8.0,
    "followStopDistanceMeters": 5.0,
    "maximumOwnerDistanceMeters": 25.0,
    "navReplanTicks": 6
  },
  "assist": {
    "durationTicks": 600,
    "decisionIntervalTicks": 6,
    "preferredDistanceMeters": 6.0,
    "evadeHorizonTicks": 12,
    "basicAttackSkillId": 38000,
    "skillCandidates": [
      {"candidateId": "active-q", "skillId": 38020, "baseScore": 1.0},
      {"candidateId": "active-w", "skillId": 38050, "baseScore": 0.9}
    ]
  }
}
```

위 AI 파일은 decision/follow/evade/command/lifecycle만 소유한다. HP/resource, damage, cooldown, action duration, animation을 복제하지 않는다.
`Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, authored animation binding이 기존 전투 수치와 표현의 정본이다.

### 7.3 Human과 AI가 공유하는 전투 경계

```text
Human C2S packet
  -> session/auth/anti-replay adapter
  -> PLAYER_ACTOR_MOVE_INTENT / PLAYER_ACTOR_SKILL_INTENT

CompanionBrain decision
  -> PLAYER_ACTOR_MOVE_INTENT / PLAYER_ACTOR_SKILL_INTENT

both -> one Server player actor executor
     -> navigation / cooldown / resource / skill / damage / snapshot
```

`Handle_Move`나 `Handle_UseSkill`에 fake session/C2S packet을 만들어 재진입하지 않는다. `SERVER_PLAYER_CONTROL_KIND { HUMAN_SESSION, SERVER_COMPANION }`을
player actor에 두고 spawn에는 확장 가능한 `REPLICATED_PLAYER_KIND`를 싣는다. 정적 identity를 매 snapshot에 반복하지 않는다.

decision 우선순위는 `EVADE_HAZARD -> FOLLOW/REPOSITION -> ACTIVE_SKILL -> BASIC_ATTACK -> HOLD`이며
Server navigation과 Server XZ combat/hazard geometry만 사용한다.

room tick 순서는 다음으로 고정한다.

```text
human packet drain/auth/anti-replay validation
  -> companion perception/decision
  -> stable PlayerId order로 normalized actor intent 실행
  -> movement/combat simulation
  -> snapshot/broadcast
```

LMB `38000`은 3단 COMBO이므로 Brain은 같은 action의 Server-authored continuation window에서만 다음 continuation intent를 낼 수 있다.
combo stage를 AI가 임의 증가시키지 않고, 일반 player와 같은 `iComboStage` Server 결과를 소비한다.

### 7.4 Dialogue 정본과 trigger

사용자 입력의 오타를 정규화한 유일한 파일명은 `Data/Dialogue/KoukuSaydon.Gunslinger.json`이다.

```json
{
  "schema": "lostark.companion-dialogue",
  "formatVersion": 1,
  "dialogueId": "dialogue.kakulsaydon.gunslinger.ko-kr.v1",
  "profileId": "companion.gunslinger.raid-guide.v1",
  "locale": "ko-KR",
  "providerPolicy": {
    "prefetchOnEncounterEnter": true,
    "realtimePlayerQuestion": true,
    "timeoutMs": 1500,
    "maximumResponseBytes": 256
  },
  "lines": [
    {
      "lineId": "g1-heart-summon",
      "triggerKind": "PATTERN_STAGE_ENTER",
      "encounterId": "ENCOUNTER_KAKULSAYDON_G1",
      "patternId": "KAKULSAYDON_G1_HEART_PING_110",
      "stageId": "SUMMON",
      "requiredChatFlags": ["GUIDE_ENABLED"],
      "presentationFlags": ["CHAT_LOG", "HEAD_BUBBLE"],
      "bubbleDurationMs": 3000,
      "priority": 100,
      "text": "하트 표시가 아닌 진짜 쿠크를 찾아 바라봐!"
    },
    {
      "lineId": "g1-question-default",
      "triggerKind": "PLAYER_QUESTION",
      "encounterId": "ENCOUNTER_KAKULSAYDON_G1",
      "patternId": null,
      "stageId": null,
      "requiredChatFlags": ["GUIDE_ENABLED"],
      "presentationFlags": ["CHAT_LOG", "HEAD_BUBBLE"],
      "bubbleDurationMs": 3000,
      "priority": 10,
      "text": "지금 패턴과 안전 위치를 먼저 확인해 줘. 연결이 복구되면 더 자세히 답할게."
    }
  ]
}
```

trigger는 Server가 commit한 `ENCOUNTER_ENTER`, `PATTERN_ENTER`, `PATTERN_STAGE_ENTER`, `ASSIST_START/END`,
`PLAYER_QUESTION`만 사용한다. pattern 안내 lookup은 exact encounter+pattern+stage, pattern default, encounter default, silence 순이다.
질문은 같은 encounter의 `PLAYER_QUESTION` default row를 fallback으로 사용하고 그 row가 없을 때만 침묵한다.
한 occurrence generation에서 같은 stage line은 한 번만 재생한다.

`requiredChatFlags`는 JSON을 수정하는 값이 아니라 room/owner별 runtime state다. UI는 flag를 직접 set하지 않고 chat request를
보낼 뿐이며 Server command parser가 owner/world/rate/state를 검증한 뒤 set/clear한다.
unknown flag, duplicate key, invalid scope, invalid UTF-8, 256-byte 초과 text는 publish를 거부한다.

`C2S_CHAT`에는 known-mask `CHAT_REQUEST_FLAG::COMPANION_QUESTION`을 추가한다. Client는 `@건슬 ` prefix일 때만 flag를 세우고,
Server가 prefix, requester, owner/room admission, rate, byte limit를 다시 검증한다. `도움!`, `/따라와`, `/그만`은 Server가
command로 소비해 일반 chat으로 broadcast하지 않고 `S2C_COMPANION_COMMAND_RESULT` typed receipt를 보낸다. `@건슬` 질문은
human chat line으로 한 번만 broadcast한 뒤 별도 companion response를 만든다.

### 7.5 기존 말풍선과 선택적 챗봇

먼저 쿠크 레벨에서 기존 `CWorldPlayerNameplateView`, `CWorldPlayerChatBubbleView`, `CChatWindowView`를 연결한다.
말풍선의 스타일과 제품 image를 새로 만들거나 ImGui widget으로 대체하지 않는다.
긴 응답은 기존 글꼴의 실제 측정 폭으로 UTF-safe wrap하고, 최대 폭·최대 줄·대기열을 bounded하게 둔다. 이는 같은 말풍선의
텍스트 배치 안정화이며 새 말풍선 디자인을 만드는 작업이 아니다.

외부 챗봇 연결은 선택적 sidecar다.

```text
Server fixed tick
  -> ServerApp-owned bounded localhost JSONL gateway
  -> 127.0.0.1:7778 sidecar
  -> provider HTTPS adapter
  -> typed ROOM_COMMAND result on the next room tick
  -> context/ledger revalidation
  -> exactly one S2C_CHAT or stale drop
  -> existing chat log + head bubble
```

- Client는 HTTP와 API key를 알지 않는다.
- Server fixed tick은 network/provider를 기다리지 않는다.
- sidecar만 provider HTTP와 process environment의 secret을 소유한다.
- cache/stale key는 `roomEpoch`, companion `NetEntityId+generation`, `profileId`, dialogue revision, trigger kind,
  encounter/pattern/stage/line ID, occurrence sequence를 포함한다. 질문은 requester PlayerId와 request sequence도 포함한다.
- pattern/stage 안내는 event tick에 **유효 prefetch cache 또는 authored JSON 중 하나**를 occurrence ledger에 commit한다.
  cache miss로 JSON을 commit한 뒤 도착한 provider text는 버려 중복 발화하지 않는다.
- player 질문은 request sequence별 provider success 또는 1500ms timeout fallback 중 정확히 하나만 commit한다.
- room/revision/generation/owner/occurrence가 달라진 늦은 응답은 버린다.
- queue/rate/response bytes/circuit breaker를 제한하고 협력 취소와 bounded join을 사용한다.
- LLM text는 gameplay trigger, pattern verdict, movement 명령을 만들 수 없다.

encounter clear/wipe/restart, companion death, owner/world exit마다 mode, HP/resource/cooldown/path, pending intent,
chat flags, dialogue occurrence ledger를 명시적으로 reset 또는 폐기한다. 마지막 human 퇴장 시 새 결정 중지,
dialogue/sidecar 작업 취소, companion despawn broadcast와 entity/map 제거, Kouku empty-room reset 순으로 정리한다.
현재 reset admission에 Kouku world를 실제로 추가하지 않은 채 완료 처리하지 않는다.

### 7.6 AI publisher와 sidecar 파일

```text
Tools/AIPipeline/Schemas/lostark.companion-ai-profile.v1.schema.json
Tools/AIPipeline/Schemas/lostark.companion-dialogue.v1.schema.json
Tools/AIPipeline/companion_ai_pipeline.py
Tools/AIPipeline/Publish-CompanionAI.ps1
Tools/AIPipeline/test_companion_ai_pipeline.py
Tools/Build/BuildDomains.json
Server/Bin/DataFiles/AI/CompanionAI.bootstrap

Tools/CompanionChat/companion_chat_sidecar.py
Tools/CompanionChat/companion_chat.config.json
Tools/CompanionChat/Start-CompanionChatSidecar.ps1
Tools/CompanionChat/Schemas/lostark.companion-dialogue-request.v1.schema.json
Tools/CompanionChat/Schemas/lostark.companion-dialogue-response.v1.schema.json
Tools/CompanionChat/test_companion_chat_sidecar.py
```

AI publisher는 `Data/AI/**`, `Data/Dialogue/**`, PlayerSkills/DamageProfiles, Kouku encounter/pattern/stage를 exact join하고
`parse -> validate -> stage -> atomic commit`한다. 실패하면 기존 bootstrap을 유지하며 Server는 authored JSON을 직접 읽는 fallback을 만들지 않는다.

sidecar request는 request ID와 위 stale key, locale, bounded question/context만 싣고 response는 request ID, status, bounded text만 싣는다.
newline-delimited UTF-8 JSON의 최대 frame을 제한하고 loopback 이외 bind를 거부한다. sidecar만
`LOSTARK_COMPANION_LLM_API_KEY`를 읽으며 model/endpoint는 deployment config에 있고 gameplay JSON에는 없다. ServerApp는 gateway와
cooperative cancel/bounded join만 소유하고 sidecar process를 fixed tick에서 시작하거나 기다리지 않는다.

---

## 8. 사용자 우선순위와 G별 구현 단위

각 G는 별도 PLAN delta, RESULT, 검증 가능한 commit 단위다. 모든 G는 **설명/목표 -> 코드·데이터 반영 -> 자동·수동 검증** 순서로 닫는다.
앞 G가 실패하면 다음 G가 local fallback, 수동 JSON 수정, Valtan 경로 재사용으로 우회하지 않는다.

| 사용자 우선순위 | 구현 묶음 | 눈에 보이는 도착점 |
|---|---|---|
| 0순위 | G00~G04 | Kouku 진입, 24px Workbench, raw animation 탐색, 이름 있는 pattern 생성, exact revision Apply, `Play Server`로 실제 Server stage 재생, Composition 확장 row |
| 1순위 | G05~G07 | Rendering Tool에서 base profile, 전체 light 기여도, Point/Spot, group on/off, blend, pattern multiplier를 배치·저장·재생 |
| 2순위 | G08~G13 | Stagger/Heart/Dance/Roulette/Card Matching과 stage branch/judgement/summon을 Server authority로 완결하고 Logic Graph로 편집 |
| 3순위 | G14~G17 | Kouku chat 표시, 한 명의 Gunslinger GUIDE/ASSIST, JSON fallback, 선택적 chatbot sidecar |

### 0순위 — 이름·애니메이션·Server 재생·Composition

#### G00 — KoukuSaydon authored 명명과 Valtan mutable owner 분리

**설명/목표**

모든 저작 경로·문서·Tool 이름을 먼저 `KoukuSaydon`으로 통일한다. 이 G에서는 아직 generic boss framework를 추출하지 않는다.
기존 Valtan mutable state의 실제 이름과 경계를 분명히 해 이후 K 구현이 Valtan object 안으로 들어가는 것을 막는다.

**반영**

- `Data/Animation/{Reference,Authored}/KakulSaydon -> KoukuSaydon` 원자적 이동
- composition/sequencer, pipeline, test fixture, BuildDomains, project/filter의 authored 이름 동시 갱신
- 기존 `CBossTool`과 `CActionCompositionWorkbench`를 실제 역할인 `CValtanBossTool`, `CValtanActionWorkbench`와 Valtan source/writer state로 명확히 정리
- public ID `KAKULSAYDON`과 physical asset alias `KoukuSaton`은 `2의 호환 예외로 유지
- generated Product는 publisher로만 재생성
- Valtan Debug 기본 HOLD, Release AUTO라는 명시적 변경 외에는 behavior parity 유지

**검증**

- authored 경로 금지 철자와 bare `KAKUL_G1_*` 0건
- 새 asset ID는 `Character|Effect|Sound|UI/KoukuSaton/...`만 참조
- old authored path reader 0건, rename 중간 실패 시 old/new 반쪽 commit 0건
- Valtan source digest, pattern count, save/reload와 Product flow parity
- 이 G에는 가짜 Kouku descriptor, registry, empty Tool/harness를 만들지 않음

#### G01 — 24px Workbench와 raw Resource 탐색

**설명/목표**

사용자가 Kouku animation을 빠르게 찾을 수 있도록 화면부터 줄인다. single-click은 로컬 read-only audition이고,
Server 재생은 G02 이후 이름 있는 pattern revision에만 허용한다.

**반영**

- lane 24px, padding 2px, 가용 높이 cap/scroll, splitter, Fit, Maximize Timeline, horizontal zoom 1200px/s
- 장문 3줄 삭제, 나머지 설명은 tooltip/Advanced로 이동
- standalone read-only Sequencer를 기본 focus에서 제외하고 Advanced inspector로 명확히 표시
- Valtan Workbench의 실제 leaf click seam에서 animation/effect/sound audition을 atomic replace로 구현
- CameraShot/WorldSequence typed enumeration, bounded preview player, camera restore owner 추가
- filter/reload auto-selection은 audition 금지
- `Append to Stage Slots` 제거, `Bind/Replace/Create Box`와 drag/drop만 writer로 유지
- G02의 Kouku consumer가 생길 때 두 Workbench가 쓰는 pure timeline/resource view seam을 같은 변경에서 추출할 수 있도록 Valtan 코드를 작은 함수 단위로 정리하되, common singleton은 만들지 않음

**검증**

- click 전후 source bytes/revision/draft generation 동일
- 새 preview stage 실패 시 기존 preview·input owner 유지
- 재클릭 0ms restart, channel별 성공 후 교체, auto-selection playback 0건
- Camera Stop/Esc 후 원래 camera restore
- 48/420/2x/Append를 요구하던 Python oracle 전면 갱신
- 사용자가 직접 24px 선택·drag와 Timeline maximize를 판정

#### G02 — Kouku Product admission, Pizza, scoped Server Play

**설명/목표**

첫 실제 두 번째 boss consumer를 만든다. 이 G부터 Kouku Resource에서 고른 binding을 이름 있는 pattern으로 저장하고,
exact runtime revision을 적용한 뒤 `Play Server`로 같은 Product stage runtime에서 볼 수 있다.

**반영**

- §6의 네 Kouku source schema, BossCatalog/BossProfiles, disabled boss placement, encounter projector/bootstrap
- 기존 direct circus `playSequence` trigger를 하나의 `activateEncounter`로 교체
- `CKoukuSaydonBrain`, Client `CKoukuSaydon`, `CKoukuSaydonPatternSource`, `CKoukuSaydonAuthoringService`
- `CKoukuSaydonBossTool`과 `CKoukuSaydonActionWorkbench`의 실제 Pizza consumer
- Pizza 4219714의 여섯 stage, Server sector hit/safe-sector variant, Client animation occurrence
- 이 실제 K consumer와 같은 commit에서만 immutable view, scoped audition transport, authoring transaction, fixed-tick stage executor를 추출하고 Valtan/Kouku 양쪽을 이전
- generic debug audition request는 immutable `{world, encounter, placement, archetype, gameplayRevision}` scope와 request sequence를 소유
- stage-action enum/parser/field grammar는 encounter-neutral schema에 추가하고, action의 Kouku 의미는 `CKoukuSaydonBrain`/semantic validator가 소비
- `hasExactValtanHighJumpTypedVolley`, exact phase action/ghost portal/topology와 같은 Valtan oracle의 counter·final assertion을
  `BOSS_VALTAN + ENCOUNTER_VALTAN` partition 안으로 한정. Kouku row에는 Valtan ID/count/value oracle을 적용하지 않음
- Debug Entrance 뒤 HOLD, Release Entrance 뒤 Product AUTO
- protocol/bootstrap grammar가 바뀌면 병합 시점의 실제 최신 version을 한 번 올리고 양 endpoint/harness를 같이 갱신

**검증**

- Pizza가 Client local AI가 아니라 Server stage clock/snapshot으로 재생
- `Play Selected`, `Play Full Pattern`, Repeat, Stop, Next와 lifecycle receipt
- V/K world·ID·revision 교차 요청 양방향 거부
- K save/play가 V selection, queue, dirty, writer, source bytes에 영향 0
- direct trigger와 Entrance sequence의 이중 재생 0건
- Release debug operation reject
- Valtan 명시적 HOLD 변경 외 stage/damage/presentation parity
- Kouku가 공용 action kind를 써도 Valtan exact oracle에 걸리지 않고, 반대로 Valtan row의 exact 회귀는 계속 거부

이 G에서 다음 실행형 harness를 실제로 만든다.

```text
Tools/KoukuSaydonBossToolHarness/
  Default/KoukuSaydonBossToolHarness.vcxproj
  Default/KoukuSaydonBossToolHarness.vcxproj.filters
  Private/KoukuSaydonBossToolHarness.cpp
  Private/KoukuSaydonSourceAndIsolationContractTests.cpp
  Private/KoukuSaydonAuthoringTransactionContractTests.cpp
```

`Framework.sln`과 `Tools/Build/Invoke-BuildAndRegression.ps1`의 FullDiagnostic에 등록하고
`Tools\KoukuSaydonBossToolHarness\Bin\Debug\KoukuSaydonBossToolHarness.exe`를 직접 실행한다.

#### G02A — encounter별 GameplayCatalog admission

**설명/목표**

단일 `Gameplay.bootstrap` publisher는 유지하되 Server runtime admission을 encounter partition별로 나눈다. Kouku row를 작업하다
잘못 만들어도 Valtan/Bern/Lobby Server 검증과 LAN 입장이 함께 멈추지 않아야 한다. 이 변경은 첫 Kouku Product가 실제 두 번째
consumer가 된 G02 뒤에만 수행한다.

**반영**

- bootstrap header/version/revision과 player/skill/damage 같은 공용 definition table을 먼저 immutable staging model로 parse
- boss/profile/part/pattern/combat-object/intro/rotation/sequence와 encounter-local semantic join을 `encounterId`별 partition으로 stage
- `ENCOUNTER_CATALOG_ADMISSION { encounterId, status, admittedRevision, attemptedRevision, reason }`
- global syntax·header·공용 table 자체가 깨져 partition을 만들 수 없으면 전체 load 실패
- encounter-local dangling ID, exact oracle, graph, count, range 실패는 그 partition만 `REJECTED`
- 여러 partition이 같은 globally unique boss/pattern/combat-object ID를 충돌시키면 충돌한 partition들을 모두 거부하고 나머지는 유지
- GameRoom/world admission은 target encounter의 `ADMITTED`와 exact revision을 확인하고, 거부 시 typed encounter-unavailable reason을 반환
- Server startup은 global catalog가 유효하고 적어도 공용 player/skill table이 admitted되면 계속되며 partition status를 구조화 로그로 출력
- 이 G는 runtime hot reload를 새로 켜지 않는다. staged reload가 이미 호출되는 경로에서는 active room이 pinned old immutable partition을
  끝까지 쓰되, 최신 candidate가 거부된 encounter의 **새 입장**은 stale partition으로 fallback하지 않고 막는다

**검증**

- valid Valtan + invalid Kouku: Server load 성공, Valtan 입장/contract PASS, Kouku만 exact reason으로 거부
- invalid Valtan + valid Kouku: Kouku 입장 가능, Valtan만 거부
- invalid global header/player/skill table: 전체 load 실패와 이전 catalog rollback
- K row가 Valtan high-jump/phase/ghost oracle의 count를 바꾸지 않음
- 공용 stage action parser는 V/K row에 동일하게 적용되고 encounter semantic oracle만 분리
- rejected candidate 뒤 active room pinned revision 보존, 새 입장 stale fallback 0건
- `ServerGameplayContractTests`와 전용 Kouku harness에 양방향 partition fixture 추가

#### G03 — 기믹 animation intake와 이름 있는 debug pattern 생성

**설명/목표**

무력화, 진짜 쿠크 찾기, 댄스타임, 룰렛, 카드 맞추기의 animation을 먼저 눈으로 찾고, 승인된 source action/clip만
정확한 이름의 pattern draft로 만든다. 이 단계의 불완전한 기믹은 Release rotation에 넣지 않는다.

**사용 흐름**

```text
Kouku Resources에서 animation single-click -> local read-only audition
-> 사용자 source action/clip/순서 선택
-> Create Kouku Pattern
-> stage 이름·duration·binding 입력
-> Validate Candidate
-> Apply Debug Revision
-> Play Server
-> Server occurrence/snapshot으로 같은 animation 확인
```

**반영**

- `KAKULSAYDON_G1_STAGGER_130`
- `KAKULSAYDON_G1_HEART_PING_110`
- `KAKULSAYDON_G1_DANCE_85`
- `KAKULSAYDON_G1_ROULETTE_50`
- `KAKULSAYDON_G1_CARD_MATCHING`
- 각 pattern의 sourceActionId, clip sequence, duration, authored binding, 사용자 확인 상태를 stable row로 저장
- 아직 judgement가 없는 pattern은 `debugaudition.json`에만 admission하고 `decisionModel.productFlow/normalPool/healthMechanics`에는 넣지 않음
- `Play Server`는 asset path나 Prototype tag를 보내지 않고 applied pattern ID와 exact gameplay revision만 제출
- Animation Tool/Resources가 raw clip을 Server 명령으로 직접 보내는 우회 금지

**검증**

- 각 row의 source action/clip이 실제 model clip과 exact join
- missing/duplicate clip, empty sequence, wrong class asset, stale revision 거부
- debug-only row가 Release Product selection에 들어가지 않음
- 사용자가 Server 재생을 보고 pattern별 animation occurrence를 승인한 뒤에만 RESULT의 visual 상태를 기록

#### G04 — Kouku Composition row/slot 확장과 P0 통합 closure

**설명/목표**

`KoukuSaydonGate1.bosscomposition.json`을 mega runtime으로 만들지 않고, Kouku source 전체를 한눈에 검증·저작하는
K-only manifest/join view로 확장한다.

**반영**

Composition row/lane은 다음 typed 역할을 가진다.

| row/lane | 값의 source owner | P0에서 보이는 것 |
|---|---|---|
| `ANIMATION` | Animation Authored | G03에서 승인한 sequence |
| `EFFECT` / `SOUND` / `CAMERA` | 각 presentation catalog | 선택·audition·명시적 binding |
| `SCENE_PROFILE` | Rendering Catalog | Gate 1 base profile reference; 실제 튜닝은 1순위 |
| `LIGHT_GROUP` | Map Light authoring | empty 허용, dangling ID는 금지; 실제 group은 1순위 |
| `TRANSIENT_LIGHT_CUE` | Pattern Lighting Catalog | empty 허용; 실제 cue value와 binding은 1순위 |
| `SUMMON_GAMEPLAY` | gameplay stage action | 현재 landed summon action만 projection |
| `SUMMON_PRESENTATION` | presentation stage | gameplay summon role exact join |
| `LOGIC` | gameplay decision/stage graph | 현재 linear/default edge의 read-only projection |
| `STAGE_BRANCH` | gameplay stage outcomes | 현재 landed branch만 projection; typed editor는 2순위 |

- pattern마다 source revision, admission status, stage count와 각 owner join 상태를 한 row로 표시
- manifest에는 Kouku source만 들어가며 Valtan path/reference 0건
- composition Save는 각 domain authoring service에 typed transaction을 위임하고 자체 gameplay writer가 되지 않음
- `SCENE_PROFILE/LIGHT_GROUP/TRANSIENT_LIGHT_CUE`는 exact referenced ID가 있을 때만 저장; 빈 lane은 허용하지만 fake ID/placeholder row는 금지
- Gate 1 base profile은 P0에서 현재 값으로 정식 생성·publish하고, 1순위 Rendering Tool에서 값을 튜닝
- candidate Save -> reparse -> project -> exact revision Apply -> Server replay round trip

**검증**

- row/slot roundtrip preservation, unknown type/version/ID 거부
- animation 선택만으로 row/slot이 생성되지 않음
- explicit Create/Bind만 dirty generation 증가
- Save 중간 실패 시 모든 source와 manifest 이전 bytes 유지
- composition publish가 Gameplay.bootstrap을 직접 쓰지 않음
- P0 종료 시 다섯 기믹 pattern의 이름·animation·stage row와 Server replay가 보이고, judgement/light가 아직 미완료임을 명확히 표시

### 1순위 — Rendering Tool, 모든 light, on/off와 blend

#### G05 — Engine Spot, maplights v2, Area Light writer

**설명/목표**

Directional은 profile, Point/Spot은 Area 문서라는 경계를 닫고 Kouku map에 실제 배치 가능한 light layer를 만든다.

**반영**

- Spot pass를 기존 index 뒤에 append하고 Engine enum/struct/light/shader/static_assert 갱신
- maplights v2 group/scope/Point/Spot exact parser와 v1 Valtan adapter
- `CRenderingAuthoringSession`의 CAS, typed mutation, temp->reparse->atomic promote
- Kouku MapCatalog `sourceLights/lights` pair와 empty v2 source/runtime을 같은 transaction으로 추가
- `TRANSIENT_LIGHT_SOURCE_CHANNEL`과 debug channel diagnostics
- authored total과 active 64 cap 분리, priority/distance/stable-ID 선택
- MainApp scene coordinator가 정확히 한 active Area fixture provider만 제출

**검증**

- bad cone/direction/group/scope, duplicate, nonfinite, stale save, declared-missing file
- v1 Valtan parity, v2 empty roundtrip
- target lighting activation 중간 실패 전체 rollback
- Spot pass index 보존과 WARP compiled-shader draw/readback
- Engine public header 후 UpdateLib -> Product

#### G06 — 통합 Rendering Tool과 Gate 1 base 분위기

**설명/목표**

Rendering Benchmark를 별도 툴로 키우지 않고 Rendering Tool 안의 tab으로 둔다. 한 창에서 Quality, Scene Profile,
Directional, Area group, Point/Spot을 비교·배치·저장한다.

**반영**

- `CRenderingTool` 추출, resizable hierarchy/viewport/Details/tabs
- Rendering Catalog 한 revision 안에서 Global Quality와 Scene Profiles 편집
- profile enumerate/activate/Duplicate As, Gate 1 base profile 튜닝
- group enable/default intensity, Point/Spot Pick Position, numeric Details, Aim At, cone debug
- owner별 Save/Validate/Publish와 Draft/Saved/Published/Active 상태
- channel별 Contribution Isolation, solo/mute preview token, A/B와 Benchmark
- 제품 on/off는 authored group/profile 값으로, 임시 solo/mute는 Debug preview token으로 구분

**검증**

- profile/map owner CAS와 failure preservation
- profile-only Area, declared empty maplights, group enable/reload
- Tool close/input-owner 상실 시 preview token 복원
- 사용자가 Directional -> Point -> Spot -> Exposure/Bloom 순서로 Gate 1 base atmosphere를 판정

#### G07 — Profile/group blend, Pattern Lighting Catalog, LIGHTING lane

**설명/목표**

Pizza/Heart/Dance 같은 짧은 연출은 base light를 끄지 않고 relative multiplier와 transient cue preset을 occurrence 동안 적용한다.

**반영**

- `Publish-LightingModifiers.ps1`, runtime catalog/receipt, `CLightingModifierCatalog`
- authored base catalog/draft와 `Apply_ComposedRuntime` 분리
- profile A/B base blend, scene/group multiplier token, blend-in/out, late join
- Point/Spot transient cue definition과 occurrence anchor
- Pizza/Heart/Dance가 참조할 actual modifier/cue values
- Kouku Workbench `LIGHTING` lane, Rendering Tool deep-link, exact revision Save/Apply
- pattern/stage end, skip, abort, death, disconnect, level exit, revision change restore

**검증**

- modifier/cue/profile/group exact join
- non-cumulative recomposition, same-source replace, nested token order, late join
- missing anchor는 cue만 격리, gameplay 유지
- 모든 exit에서 Gate 1 base로 blend-back
- 사용자가 Pizza darkening, Heart role Spot, Dance stage Spot을 A/B 판정

### 2순위 — Server boss logic과 흐름

#### G08 — Health mechanic scheduler + Stagger 130

**설명/목표**

health crossing queue와 once ledger를 첫 consumer인 Stagger와 동시에 만든다. 다른 threshold pattern보다 반드시 먼저 완료한다.

**반영**

- multi-threshold crossing의 high-HP/order pending queue
- encounter-generation once ledger, interruption/restart policy
- `KAKULSAYDON_G1_STAGGER_130`의 shield/stagger window/front reflection/failure verdict
- gameplay branch와 Workbench Composition projection
- lighting invocation과 warning cue join

**검증**

- 한 hit로 130/110 동시 crossing 시 130부터 한 번씩 실행
- shield break/timeout/reflection/damage/failure branch
- restart generation에서 ledger 초기화, 같은 generation 중복 금지

#### G09 — Heart Ping 110

**설명/목표**

진짜 쿠크 찾기의 summon, 바라보기 judgement, 암전과 Spot을 한 수직 슬라이스로 닫는다.

**반영**

- `VANISH -> SUMMON -> JUDGE -> RETURN`
- shooter 1/heart 3 typed summon role, partial spawn atomic rollback
- authoritative player yaw/facing tolerance와 failure response
- mechanic-dark token과 role별 four Spot anchor
- stage/abort/disconnect restore

**검증**

- role 배정·facing 경계값·partial spawn rollback
- Client visual/Spot이 verdict source가 아님
- late join과 모든 종료 경로 복원
- 사용자 진짜 쿠크/하트 animation·Spot 판정

#### G10 — Dance 85

**설명/목표**

Saydon summon animation과 Q/W/E/R 입력 judgement, 단계별 lighting을 같은 occurrence로 묶는다.

**반영**

- `INTRO -> STEP_1..3 -> RESOLVE/PUNISH -> RECOVERY`
- Server-seeded dance variant와 first-input-only 3000ms window
- mechanic input packet/admission, normal skill input suppression
- summon presentation, stage Spot/cue, recovery restore

**검증**

- early/late/duplicate/wrong input
- summon missing rollback, pass/punish branch
- Client가 variant 또는 judgement를 만들지 않음
- 사용자 stage animation/Spot/restore 판정

#### G11 — Roulette 50

**설명/목표**

roulette world sequence와 Server outcome/card response를 50줄 once mechanic으로 닫는다.

**반영**

- threshold queue의 50 consumer
- Server-seeded outcome, typed card/status response와 damage
- world sequence occurrence correlation
- fixture/stage group cue와 restore

**검증**

- outcome determinism, sequence 중복 0, stale occurrence 거부
- disconnect/restart/card cleanup
- 사용자 animation/sequence와 Server result 일치 판정

#### G12 — Card assignment, Madness/Clown, Card Matching

**설명/목표**

Gate 1 공통 player state와 카드 맞추기를 실제 소비자 단위로 추가한다.

**반영**

- eligible human-only suit assignment와 snapshot
- madness gain/threshold/reset, temporary clown form와 admitted skill/profile
- bind 대상과 homing card Server combat object
- suit match/failure damage와 cleanup
- Data/UI stable slot의 card/madness HUD; companion 제외

**검증**

- suit uniqueness/policy, AI exclusion
- madness/form/skill transition과 asset admission
- bind/homing/match boundary, partial object rollback
- ImGui 제품 HUD 0건

#### G13 — Kouku Logic Graph와 Product flow

**설명/목표**

이미 landed한 typed stages/outcomes만 Kouku 전용 graph에서 편집한다. Logic Graph가 먼저 schema를 발명하지 않는다.

**반영**

- pure graph model/renderer만 Valtan과 공유
- Kouku selection/draft/undo/save/revision/writer는 전용
- stage branch, counter/judgement, health route, Product order 편집
- Save -> candidate project -> one Gameplay.bootstrap -> exact revision Apply/Restart
- Boss Tool Live node/edge와 Composition row 연동

**검증**

- finite graph, dangling/cycle policy, duplicate node/edge
- stale save/apply rollback
- V/K graph state와 writer bytes 완전 격리
- landed Gate 1 Product flow 전체 Server replay

### 3순위 — Gunslinger AI

#### G14 — Kouku 제품 chat/log/nameplate/bubble

**설명/목표**

AI보다 먼저 human-to-human chat을 쿠크 Level에서 완결해 기존 말풍선 소비 경로를 증명한다.

**반영**

- MainApp chat admission과 Kouku command sink
- replicated player view/nameplate/head bubble
- `S2C_CHAT` presentation flags/duration, remote bounded log
- font-metric UTF-safe wrap와 bounded queue, local echo dedupe

**검증**

- 두 human의 log+bubble exactly-once
- invalid flag/oversize/UTF failure
- disconnect/level-exit cleanup
- 사용자 기존 말풍선 스타일 판정

#### G15 — 공통 player actor executor + GUIDE 1명

**설명/목표**

첫 admitted human이 owner인 sessionless Gunslinger 하나를 일반 actor slot에 transactionally 만들고 따라오게 한다.

**반영**

- human adapter와 normalized move/skill intent executor
- control/replicated actor kind와 stable PlayerId execution order
- `Data/AI/Gunslinger.json` schema/publisher/catalog
- first-human owner, empty-slot commit, movePlayer-only companion traversal
- GUIDE follow/nav, owner-only commands, typed receipt, all lifecycle cleanup

**검증**

- human behavior parity와 fake session 0건
- full/no-slot/invalid-owner/mid-commit rollback
- trigger/party/reward/gimmick exclusion
- owner leave/death/wipe/restart/last-human reset

#### G16 — ASSIST combat + deterministic authored Dialogue

**설명/목표**

같은 actor가 기존 Gunslinger 수치로 전투하고, chatbot이 없어도 encounter/pattern/stage 안내를 한다.

**반영**

- `도움!` ASSIST -> authored duration -> GUIDE
- Server hazard query와 38020/38050, 38000 COMBO intent
- `Data/Dialogue/KoukuSaydon.Gunslinger.json` publisher/runtime
- chat command/question flag, owner/rate admission, typed result
- occurrence ledger와 exact/pattern/encounter/question fallback

**검증**

- cooldown/resource/action/combo stage parity
- AI<->boss damage와 deterministic trace
- dialogue once-per-occurrence, JSON-only full operation
- command consumption과 normal chat echo 중복 0건

#### G17 — 선택적 chatbot sidecar

**설명/목표**

챗봇은 대사 품질만 확장하며 raid와 Server tick의 필수 의존성이 아니다.

**반영**

- `Tools/CompanionChat` request/response schema, JSONL loopback gateway, mock provider
- encounter prefetch, pattern cache-or-JSON single commit
- question success-or-timeout single commit
- full stale key, rate/queue/frame cap, circuit breaker, cooperative cancel/bounded join
- secret/model/deployment config는 sidecar process만 소유

**검증**

- provider success/timeout/malformed/oversize/disconnect
- late response drop와 중복 발화 0
- Server key/HTTP/fixed-tick wait 0
- 실제 유료 provider 호출은 Core/FullDiagnostic에서 제외

Gate 2/3, Mario, Bingo는 Gate 1의 P0~P2가 끝난 뒤 관문별 base profile, Area group, encounter, Server mechanic,
Client presentation, Tool row, publisher/harness를 같은 방식으로 추가한다. 빈 future node/source 파일을 지금 만들지 않는다.

---



## 9. 검증 매트릭스

### 9.1 Publisher/데이터

- Kouku naming contract: 허용 spelling 위치와 금지 경로
- Kouku source parse/version/unknown field/duplicate ID
- gameplay-stage-presentation-animation/effect/camera/light exact join
- dangling profile/group/modifier/summon role/asset ID 거부
- invalid path, absolute path, `..` 거부
- health threshold 중복과 multi-cross order
- partial source/candidate/publish 실패 시 이전 bytes 보존
- Valtan source bytes가 Kouku save로 변하지 않음
- Valtan exact oracle의 encounter scope와 encounter별 catalog admission
- maplights v1 adapter/v2 roundtrip/empty/invalid cone/active cap
- AI class/skill/range/hysteresis/command/dialogue scope/UTF-8/256-byte 검증

### 9.2 C++/wire/runtime

- NetworkProtocol roundtrip과 unknown mask
- V/K audition scope/revision/request correlation
- Valtan 명시적 Debug HOLD 변경 외 behavior parity
- valid V/invalid K 및 invalid V/valid K partition admission
- Server contract: Pizza sectors, 130/110/85/50 thresholds, summon rollback, input window
- Server contract: card assignment uniqueness, madness/form transition, card matching verdict와 companion exclusion
- modifier non-cumulative/replace/restore/late join
- chat local/remote duplication, companion presentation flags
- human/companion actor executor parity와 lifecycle
- sidecar timeout/stale/malformed cancellation

### 9.3 정본 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/KoukuSaydonPipeline/Project-KoukuSaydonPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate
# G07에서 추가한 뒤 실행: Tools/RenderingPipeline/Publish-LightingModifiers.ps1 -Mode Validate
# G15에서 추가한 뒤 실행: Tools/AIPipeline/Publish-CompanionAI.ps1 -Mode Validate
Server\Bin\Debug\Server.exe --contract-test
Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
Tools\KoukuSaydonBossToolHarness\Bin\Debug\KoukuSaydonBossToolHarness.exe
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
git diff --check
```

Engine public header를 바꾼 G05는 `UpdateLib` 뒤 Client까지 검증한다. 신규 C++ 파일은 project/filter XML parse와 Product build에서 누락을 잡는다.

### 9.4 사용자 전용 visual smoke

에이전트는 Client/UI를 자율 실행하거나 visual PASS를 대신 기록하지 않는다. 자동 검증 뒤 사용자가 `Client/Default`에서 직접 확인한다.

1. 24px lane 선택·drag, maximize/Fit, Details/Resources 접근
2. raw Animation/Effect/Sound/Camera single-click 즉시 preview와 non-mutation
3. 다섯 기믹 이름의 draft 생성, Apply Debug Revision, `Play Server` animation
4. Rendering Tool의 Gate 1 base directional/ambient와 point/spot group A/B
5. Pizza 시작 암전과 종료 원복
6. Heart Ping 암전, 진짜 쿠크/하트 role spotlight, 모든 종료 경로 원복
7. Dance 단계별 spotlight와 recovery 원복
8. Roulette/Card cue와 Server verdict 일치
9. human chat과 Gunslinger 안내의 기존 chat log/머리 위 말풍선
10. `도움!` 전투, GUIDE 복귀, owner leave despawn

---

## 10. 완료 정의와 비범위

완료는 문서나 Tool mock이 아니라 다음 조건을 모두 만족하는 상태다.

- Kouku authored 경로·문서·Tool 이름이 `KoukuSaydon`으로 원자적 마이그레이션되고 public `KAKULSAYDON`/physical `KoukuSaton` 예외가 검증됨
- Valtan과 Kouku의 mutable 저작·runtime 선택·검증 상태가 완전히 분리됨
- 한 encounter의 semantic 실패가 다른 admitted encounter의 입장과 LAN 검증을 막지 않음
- Rendering Tool에서 Gate 1 base/profile/group/Point/Spot/modifier를 저장·재로드·publish할 수 있음
- Pizza와 130/110/85/50 기믹이 실제 Server authority로 실행됨
- 각 pattern modifier가 종료·중단·퇴장·재시작에서 정확히 base로 복원됨
- one-click Resource audition이 문서를 수정하지 않음
- 한 명의 Gunslinger가 같은 player executor로 전투하고, authored dialogue/optional chatbot 결과를 기존 말풍선으로 표시함
- 관련 publisher, protocol/server harness, Product/FullDiagnostic, `git diff --check` 통과
- 화면 품질은 사용자 서면 관찰로만 PASS 처리

이번 범위에 포함하지 않는다.

- 발탄 data를 Kouku schema로 복제하거나 한 Boss Tool dropdown에 합치기
- 두 번째 model/runtime path
- Client local boss/AI combat authority
- ImGui 말풍선 또는 제품 UI
- pattern마다 absolute scene profile을 복제해 암전
- raw map mesh의 추측 삭제
- Gate 2/3/Mario/Bingo placeholder
- AI 전용 damage/cooldown/animation 수치 복제
- Client 또는 Server fixed tick에서 직접 provider HTTP 호출

이 순서의 기준은 단순하다. **먼저 Kouku에서 실제 animation을 찾아 이름 있는 pattern으로 Server 재생하고,
그 row 위에 조명과 판정을 얹은 뒤, 마지막으로 같은 확정 이벤트를 Gunslinger 가이드가 설명하게 한다.**

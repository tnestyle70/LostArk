# Effect Tool G07 레퍼런스 워크스페이스와 월드 Preview 계획

## G07-00. 이 문서의 기준

- 이전 단계: [G05~G06 계획](2026-08-05_EFFECT_TOOL_G05_G06_PLAN.md)
- 코드 설명 형식 정본: [G06 Shader와 남은 반영 가이드](2026-08-05_EFFECT_TOOL_G06_SHADER_REMAINING_GUIDE.md)
- 다음 단계: [G08 Particle·Trail·AfterImage 계획](2026-08-05_EFFECT_TOOL_G08_PARTICLE_TRAIL_RENDERING_PLAN.md)
- 화면 기준: `C:/Users/user/Desktop/툴/01_EffectTool_Texture_EffectDetail.png`,
  `03. EffectTool_ModelView.png`, `10_EffectTool_Integrated_Workspace.png`,
  `11_EffectTool_DataFiles.png`

G07의 본질은 현재 세로로 긴 한 창을 레퍼런스처럼 다섯 작업 창으로 나누고,
`EFFECT_DOCUMENT_DESC`의 모든 Element를 실제 Development 월드에서 한 Effect로 동시에 보는 것이다.
저장 형식은 v4를 유지한다. 창 배치, 선택 상태, timeline, character, animation, anchor는 Tool session이며
`.effect.json`에 저장하지 않는다.

## G07-01. 종료 화면과 종료 증거

F1에서 Effect Tool을 열면 다음 다섯 창이 동시에 보인다.

```text
왼쪽       Effect Tool     Element type, Add Element, resource slot, thumbnail grid
가운데     Model View      실제 Development world, 합성 Effect, timeline, character animation
오른쪽     Effect Detail   선택한 Element 하나의 transform/color/UV/timing
오른쪽 아래 All Effects    현재 Document.Elements stack
아래       Data Files      Create/Load/Save/Discard와 EffectAssetId
```

G07 완료 판정은 다음 열 동작이 모두 이어지는 것이다.

1. `Mesh / Texture / Particle / Decal / Trail` 중 종류를 고른다. UI의 `Texture`는 저장 enum `SPRITE`의 표시 이름이다.
2. Base/Noise/Mask/Emissive/Dissolve slot과 resource category를 고르고 DDS thumbnail을 누르면 현재 slot에 stable asset ID가 commit된다.
3. 성공한 thumbnail click은 별도 Bind 버튼 없이 월드 Effect를 즉시 갱신한다.
4. Add Element로 만든 Mesh/Sprite/Decal layer가 `All Effects`에 쌓인다.
5. 모든 layer가 같은 sample time과 같은 root transform으로 월드에 동시에 보인다.
6. Play/Pause/Loop/Reset이 Document를 수정하지 않고 preview clock만 바꾼다.
7. `CCharacterPreviewPanel`에서 현재 roster Character를 고르고 실제 clip을 재생할 수 있다.
8. `Set Effect Pivot Player`, `Set Effect Pivot Weapon`, `Clear Effect Pivot`이 각각 검증된 root/socket/world pivot으로 바뀐다.
9. Model View에서 world를 누르면 `CGameInstance::Picking` 결과가 Mouse Viewport/World Position에 표시되고 WORLD pivot으로 사용할 수 있다.
10. 선택한 pivot을 따라 Effect가 움직이며 잘못된 socket/bone에서는 origin으로 떨어지지 않는다.

## G07-02. 저장 상태와 session 상태를 분리한다

| 상태 | owner | 저장 여부 | 의미 |
|---|---|---:|---|
| `EFFECT_DOCUMENT_DESC` | `CEffect_Tool` | 저장 | 완성 Effect asset 하나 |
| `Document.Elements` | Document | 저장 | 동시에 합성되는 visual layer stack |
| 선택 Element/slot/resource | `CEffect_Tool` | 미저장 | 현재 UI가 편집하는 대상 |
| 창 docking 배치 | ImGui | 미저장 | 작업 화면 배치 |
| play/pause/loop/sample time | `CEffect_Tool` | 미저장 | preview 재생 상태 |
| character/clip/frame | `CCharacterPreviewPanel`와 `CModel` | 미저장 | Effect를 확인하기 위한 target |
| world/player/weapon/bone pivot | `CEffect_Tool` | 미저장 | preview에만 공급하는 attachment transform |
| selected weapon socket | `CEffect_Tool` | 미저장 | `CHARACTER_SPEC::pWeapons`에서 고른 stable socket 이름 |
| mouse viewport/world position | `CEffect_Tool` | 미저장 | Model View click과 `CPicking`으로 얻은 임시 world pivot |

`Add Element`는 완성 Effect를 하나 더 만드는 버튼이 아니다. 현재 Document의
`Elements`에 layer 하나를 stage하고 검증한 뒤 commit하는 버튼이다. `Data Files Save`가
그 stack 전체를 `<EffectAssetId>.effect.json` 하나로 저장한다.

## G07-03. 추가·수정 파일과 의존 관계

### 새 파일

| 파일 | 한 줄 책임 | 직접 연결 |
|---|---|---|
| `Client/Public/Effect_DocumentRenderer.h` | 한 Document를 현재 render target에 그리는 공용 renderer 계약 | `CEffectPreview`, `CEffectObject` |
| `Client/Private/Effect_DocumentRenderer.cpp` | Element resource cache와 Mesh/Sprite/Decal draw를 한 경로로 실행 | 기존 Preview HLSL, `CModel`, `CVIBuffer_Rect` |
| `Client/Public/Effect_Object.h` | world layer에 놓이는 Effect GameObject | `CGameInstance`, `RENDERGROUP::BLEND` |
| `Client/Private/Effect_Object.cpp` | sample time과 attachment root를 받아 Document renderer를 호출 | `CEffect_DocumentRenderer` |
| `Client/Public/Effect_ThumbnailCache.h` | 화면에 보이는 DDS만 SRV로 lazy load하는 bounded cache | Effect Tool resource grid |
| `Client/Private/Effect_ThumbnailCache.cpp` | Resources-relative ID 해석, 실패 상태, LRU 제거를 소유 | `CRuntimeAssetRoot`, DirectXTK DDS loader |

### 기존 파일

| 파일 | 이번 G에서 바뀌는 이유 |
|---|---|
| `Client/Public/Effect_Preview.h` | 직접 shader/model cache를 소유하지 않고 `CEffect_DocumentRenderer`를 사용한다. |
| `Client/Private/Effect_Preview.cpp` | off-screen target 설정 후 같은 Document renderer를 호출하는 진단 창으로 축소한다. |
| `Client/Public/Effect_Tool.h` | 다섯 창, timeline, thumbnail, world object, character target session을 소유한다. |
| `Client/Private/Effect_Tool.cpp` | 기존 panel을 다섯 window에 재배치하고 world preview command를 보낸다. |
| `Client/Public/CharacterPreviewPanel.h` | 현재 roster target과 실제 weapon socket 목록을 Tool에 read-only로 제공한다. |
| `Client/Private/CharacterPreviewPanel.cpp` | preview target 교체 시 socket selection도 같은 generation으로 무효화한다. |
| `Client/Public/AnimationTargetService.h` | player root와 `CHARACTER_SPEC` 기반 weapon anchor 목록/transform 조회를 제공한다. |
| `Client/Private/AnimationTargetService.cpp` | level/layer/index 검색 없이 현재 target의 stable weapon socket을 해소한다. |
| `Client/Public/MainApp.h` | Effect Tool의 `Update` 호출 경계를 유지한다. |
| `Client/Private/MainApp.cpp` | Debug Effect prototype 등록과 매 frame Tool session update를 연결한다. |
| `Client/Default/Client.vcxproj` | 새 H/CPP 여섯 개를 실제 빌드에 등록한다. |
| `Client/Default/Client.vcxproj.filters` | 물리 폴더와 같은 Header/Source 필터에 등록한다. |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | G07 파일·프로젝트 등록·금지된 두 번째 preview renderer를 검사한다. |

## G07-04. `Effect_DocumentRenderer.h` 계약

### 이 파일이 존재하는 이유

현재 `CEffectPreview` 안에는 model/texture load, shader 값 bind, draw가 한꺼번에 들어 있다.
이를 복사해 world renderer를 만들면 Tool Preview와 이후 제품 runtime의 결과가 달라진다.
`CEffect_DocumentRenderer` 하나가 실제 draw를 소유하고, off-screen과 world는 render target과 root만 공급한다.

### 추가 선언

```text
class CEffect_DocumentRenderer final

Initialize()
  공통 Mesh/Rect shader, Rect buffer, fallback texture를 한 번 만든다.

Stage_Document(const EFFECT_DOCUMENT_DESC&)
  새 Document의 모든 resource를 local stage에 준비하고 전부 성공했을 때 cache를 교체한다.

Render(f32 sampleTime, const float4x4_t& rootWorld)
  Elements 순서를 유지하며 start delay/lifetime을 평가하고 보이는 Element만 그린다.

Clear_Document()
  Document별 model/texture cache만 비우고 공통 GPU resource는 유지한다.

Get_Status()
  마지막 stage/render 실패 이유를 Tool에 제공한다.
```

### 핵심 멤버

| 멤버 | 의미 |
|---|---|
| `m_Document` | 검증과 resource stage까지 성공한 현재 draw snapshot |
| `m_ElementResources` | Element ID를 key로 찾는 model/texture GPU cache; 저장 정본이 아니다. |
| `m_pMeshShader`, `m_pRectShader` | 기존 G06 Preview HLSL을 공유하는 GPU program |
| `m_pRect` | Sprite/Decal이 공유하는 사각형 geometry |
| `m_pWhiteTexture`, `m_pBlackTexture` | 선택 resource가 없을 때 slot 의미를 보존하는 fallback |
| `m_strStatus` | 실패 원인을 삼키지 않고 Tool에 보여 주는 진단 문자열 |

### 불변식

- `Stage_Document` 실패 시 기존 `m_Document`와 기존 GPU cache를 유지한다.
- cache key는 pointer나 vector index가 아니라 `strElementId + resource slot + asset ID`다.
- asset ID는 `Resources/Effect/...` 기준 상대 ID만 허용한다.
- Particle와 Trail은 G07에서 조용히 Rect로 대신 그리지 않는다. status에 `G08 required`를 남긴다.

## G07-05. `Effect_DocumentRenderer.cpp` 함수 흐름

### `Stage_Document`

한 줄 책임: 새 Document 전체가 그려질 수 있을 때만 현재 GPU resource snapshot을 교체한다.

```text
Effect Tool의 create/load/apply 성공
→ Document version, Element ID, kind, binding을 다시 확인
→ local staged resource map 생성
→ Mesh Model은 CModel::Create
→ texture slot은 DDS SRV load
→ 어느 하나라도 실패하면 staged map 폐기
→ 모두 성공하면 m_Document와 m_ElementResources를 한 번에 교체
```

### `Render`

한 줄 책임: 하나의 sample time에서 활성화된 모든 Element를 같은 effect root 아래 그린다.

```text
CEffectPreview 또는 CEffectObject
→ elementLocalTime = sampleTime - startDelay
→ 0 미만이거나 lifetime 이후면 skip
→ Detail.Transform과 revolution으로 Element local matrix 계산
→ elementWorld = local * rootWorld
→ Color/UV/dissolve 값을 shader에 bind
→ Mesh는 CModel, Sprite/Decal은 CVIBuffer_Rect draw
→ 실패하면 Element ID를 포함한 status 기록
```

## G07-06. `Effect_Object.h/.cpp` 계약

`CEffectObject`는 Effect의 저장 문서를 소유하는 Tool이 아니다. world layer가 소유하는 draw instance다.
Tool은 이 객체를 weak reference로 기억하고, level이 바뀌면 이전 객체를 붙잡지 않는다.

### 추가 함수

| 함수 | 한 줄 책임 |
|---|---|
| `Initialize_Prototype` | 공통 prototype 자체를 준비한다. |
| `Initialize` | Device/Context 기반 Document renderer를 준비한다. |
| `Stage_Document` | Tool이 보낸 검증 완료 Document copy를 renderer에 stage한다. |
| `Set_SampleTime` | Tool timeline의 현재 초 값을 world instance에 전달한다. |
| `Set_RootWorld` | world/character/bone에서 얻은 현재 attachment matrix를 전달한다. |
| `Set_Visible` | anchor가 해소되지 않은 frame에는 origin 오표시를 막는다. |
| `Late_Update` | visible Document만 `RENDERGROUP::BLEND`에 제출한다. |
| `Render` | `CEffect_DocumentRenderer::Render` 한 경로만 호출한다. |

world object 생성 흐름은 다음 하나다.

```text
Effect Tool open in Development
→ Prototype_GameObject_EffectObject clone
→ current level / Layer_EffectPreview에 stage
→ 성공 후에만 기존 preview object 제거
→ weak_ptr 교체
```

레벨 전환, Tool discard, Tool destruction에서는 현재 level과 object가 여전히 같은지 확인한 뒤 제거한다.

## G07-07. `Effect_ThumbnailCache.h/.cpp` 계약

### 본질

832개 resource를 Tool open 때 전부 GPU texture로 만들지 않는다. Catalog는 문자열 목록만 들고 있고,
현재 scroll 영역에 보이는 DDS card가 `Request`될 때만 SRV를 만든다.

### 추가 함수와 흐름

| 함수 | 한 줄 책임 |
|---|---|
| `Begin_Frame(frameNumber)` | 이번 frame의 visible request와 load budget을 시작한다. |
| `Request(assetId)` | cache hit면 SRV, miss면 이번 frame budget 안에서 DDS를 load한다. |
| `Invalidate(catalogRevision)` | Refresh Resources 성공 뒤 이전 catalog cache를 비운다. |
| `Trim()` | 최근 사용하지 않은 항목을 최대 192개 아래로 제거한다. |
| `Clear()` | Tool 종료 때 모든 SRV를 해제한다. |

```text
ImGuiListClipper가 현재 보이는 resource card 범위를 계산
→ DDS card만 Request(assetId)
→ CRuntimeAssetRoot로 안전한 실제 경로 해석
→ CreateDDSTextureFromFile
→ 성공 SRV 또는 실패 status cache
→ 같은 깨진 파일을 매 frame 다시 열지 않음
```

상단에는 레퍼런스와 같은 `Base / Noise / Mask / Emissive / Dissolve` slot card를 항상 표시한다.
그 아래 category combo는 `All`, `BaseTextures`, `NoiseTextures`, `MaskTextures`, `EmissiveTextures`,
`DissolveTextures`, `Meshes`처럼 Resources-relative 경로에서 파생한 보기 filter다. category 이름은 저장하지 않는다.

`UV.bSequence`가 켜진 texture는 `Keyframes` strip에서 tile count/index를 thumbnail crop으로 보여 준다.
tile click은 `UV.iTileIndex`를 stage/validate/commit하며 원본 DDS를 잘라 새 파일로 만들지 않는다.

WMODEL은 이름 card와 종류 icon을 표시하고, 선택했을 때 가운데 Model View가 실제 모델을 보여 준다.
G07에서는 모든 WModel의 별도 회전 thumbnail을 미리 만들지 않는다.

## G07-08. `Effect_Tool.h`에 추가되는 session 상태

### enum class

```text
EFFECT_PREVIEW_PIVOT_KIND
  WORLD          mouse picking 또는 입력한 고정 world transform을 사용한다.
  PLAYER_ROOT    Set Effect Pivot Player가 선택하며 Resolve_RootTransform을 사용한다.
  WEAPON_SOCKET  Set Effect Pivot Weapon이 선택하며 CHARACTER_SPEC의 selected socket을 사용한다.
  MODEL_BONE     고급 입력에서 적은 stable bone 이름을 Resolve_AnchorTransform으로 검증한다.
  END            초기화 누락을 검출하는 sentinel이다.
```

### 멤버 변수

| 멤버 | 의미 |
|---|---|
| `m_pThumbnailCache` | DDS thumbnail GPU cache의 유일 owner |
| `m_pWorldPreviewObject` | level이 소유한 Effect Object의 weak reference |
| `m_PreviewPanel` | 기존 Character preview 생성·교체·해제를 재사용하는 owner |
| `m_ePreviewPivotKind` | WORLD/PLAYER_ROOT/WEAPON_SOCKET/MODEL_BONE 선택; Document 값이 아니다. |
| `m_PreviewBoneName` | MODEL_BONE일 때만 읽는 session 입력 buffer |
| `m_strSelectedWeaponAnchorSlotId` | 현재 CharacterSpec에서 고른 stable socket ID; vector index를 저장하지 않는다. |
| `m_PreviewWorldRoot` | WORLD일 때 사용하는 고정 world matrix |
| `m_vMouseViewportPosition` | Model View 위 cursor의 viewport pixel 좌표 |
| `m_vPickedWorldPosition` | `CGameInstance::Picking`이 성공한 world meter 좌표 |
| `m_bPendingWorldPivotPick` | 다음 유효 Model View click을 WORLD pivot으로 commit할 때만 true인 session command |
| `m_bPreviewPlaying` | Tool clock이 매 frame 증가하는지 나타낸다. |
| `m_bPreviewLoop` | document duration 끝에서 0으로 wrap할지 나타낸다. |
| `m_fPreviewTimeSeconds` | 모든 Element가 공유하는 sample time |
| `m_fPreviewDurationSeconds` | `max(startDelay + lifetime + afterImage)`로 계산한 표시 범위 |
| `m_iDockLayoutVersion` | default dock를 한 번만 만들고 사용자 배치를 매 frame 덮지 않게 한다. |

## G07-09. `Effect_Tool.cpp` 창별 함수

### `Render_Workspace`

한 줄 책임: full viewport dockspace를 열고 다섯 독립 창을 같은 Tool session에 연결한다.

```text
CEffect_Tool::Render
→ DockSpaceOverViewport
→ dock node가 없거나 layout version이 바뀐 최초 1회만 Build_DefaultDockLayout
→ Render_EffectToolWindow
→ Render_ModelViewWindow
→ Render_EffectDetailWindow
→ Render_AllEffectsWindow
→ Render_DataFilesWindow
```

`imgui.ini`는 사용자 로컬 배치 산출물이므로 Git에 넣지 않는다. default layout은 최초 생성만 하고
사용자가 옮긴 창을 다음 frame에 원위치시키지 않는다.

### 기존 panel 이동

| 기존 함수 | 이동 대상 |
|---|---|
| `Render_EffectTypeSelector`, `Render_AddElementPanel`, `Render_MaterialPanel` | `Render_EffectToolWindow` |
| `Render_DetailPanel` | `Render_EffectDetailWindow` |
| `Render_ElementList` | `Render_AllEffectsWindow` |
| `Render_NewDocumentPanel`, `Render_ActiveDocumentPanel` | `Render_DataFilesWindow` |
| `Render_PreviewPanel` | 작은 진단 영역으로만 남기고 주 preview 책임은 `Render_ModelViewWindow`로 이동 |

### `Render_ModelViewWindow`

한 줄 책임: world 결과를 가리지 않는 조작 패널만 제공하고 character/clip/anchor/timeline을 편집한다.

```text
CCharacterPreviewPanel::Refresh_Level
→ Render_Selector(dirty 여부, lock reason)
→ CAnimationTargetService::Resolve_Model
→ scene/preview Character의 CHARACTER_SPEC weapon socket 목록 표시
→ CModel animation name 목록 표시
→ 선택 시 Set_Animation + track position 0
→ Play/Pause/Frame/Restart/Loop 조작
→ Set Effect Pivot Player / Set Effect Pivot Weapon / Clear Effect Pivot 버튼
→ WORLD / PLAYER_ROOT / WEAPON_SOCKET / MODEL_BONE 선택
→ Model View가 hovered이고 다른 ImGui item이 mouse를 소비하지 않은 click에서 Picking
→ timeline slider와 status 표시
```

레퍼런스의 `Slayer/Gunslinger/Destroyer/...` 고정 checkbox와 `Sword/Hand/Shot/...` 문자열은 그대로
하드코딩하지 않는다. 현재 저장소의 다섯 roster와 `CHARACTER_SPEC::pWeapons`가 실제 정본이며,
화면 동작은 동일하게 `target 선택 → animation 선택 → player/weapon pivot 선택`으로 제공한다.

### 레퍼런스 버튼 이름과 현재 command의 정확한 연결

| 화면 이름 | 실행 command | 저장 데이터 변경 |
|---|---|---:|
| `Reset` | preview clock, Particle/Trail history, world preview session을 0으로 되돌린다. | 아니요 |
| `CreateEffect` | 활성 Document에 현재 `Mesh/Texture/Particle/Decal/Trail` layer 하나를 `Try_AddElement`로 추가한다. | 예 |
| `Update Textures` | 같은 resource catalog를 다시 scan하고 texture filter 결과와 thumbnail revision을 갱신한다. | 아니요 |
| `Update Meshes` | 같은 resource catalog를 다시 scan하고 WModel filter 결과를 갱신한다. | 아니요 |
| `Select Class` | 현재 roster의 선택 target을 `CCharacterPreviewPanel`로 생성·교체한다. | 아니요 |
| `Class Clear` | Tool이 만든 preview target을 해제하고 유효한 Scene Character가 있으면 그 target으로 돌아간다. | 아니요 |
| `Select Weapon` | 선택 CharacterSpec의 weapon/socket 항목을 현재 weapon pivot 후보로 고른다. | 아니요 |
| `Weapon Clear` | weapon 선택만 비우며 Character 장비나 Document를 변경하지 않는다. | 아니요 |
| `Set Effect Pivot Player` | `PLAYER_ROOT`를 선택하고 현재 target root matrix를 사용한다. | 아니요 |
| `Set Effect Pivot Weapon` | `WEAPON_SOCKET`을 선택하고 검증된 selected socket matrix를 사용한다. | 아니요 |
| `Clear Effect Pivot` | attachment를 끊고 마지막으로 commit된 WORLD pivot으로 돌아간다. | 아니요 |
| `Time Reset All` | 모든 Element가 공유하는 preview clock과 stateful playback history를 0으로 되돌린다. | 아니요 |
| `Delete / Clear All` | 선택 Element 하나 또는 전체 stack을 stage/validate/commit으로 제거한다. | 예 |

`Update Textures`와 `Update Meshes`를 화면에 둘 수는 있지만 catalog owner나 filesystem scan 경로를
두 벌로 만들지 않는다. 둘 다 기존 `Refresh Resources` 한 경로를 호출하고 완료 뒤 서로 다른 보기 filter만 선택한다.

### `CAnimationTargetService`에 추가하는 조회 함수

```text
Get_WeaponAnchorChoices(targetGeneration)
  현재 target과 generation을 확인한다.
  → target의 CHARACTER_SPEC::pWeapons를 순서대로 읽는다.
  → 빈 socket과 중복 stable socket ID를 제외한다.
  → UI가 표시할 weapon name + anchor slot ID의 read-only 목록을 반환한다.

Resolve_WeaponAnchorTransform(targetGeneration, anchorSlotId)
  현재 target/generation과 stable anchorSlotId를 확인한다.
  → CharacterSpec에서 정확히 같은 socket을 찾는다.
  → 현재 CModel pose의 bone/socket combined matrix를 구한다.
  → Character root world matrix를 곱한다.
  → finite matrix일 때만 outWorld를 교체하고 성공한다.
  → 실패하면 identity/origin을 반환하지 않고 false와 이유를 보존한다.
```

이 함수가 `AnimationTargetService`에 있어야 Effect Tool이 level/layer/vector index를 추측하지 않고,
Animation Tool과 같은 현재 target generation 및 실제 pose를 소비할 수 있다.

### `Try_ApplyResourceFromThumbnail`

한 줄 책임: resource card 한 번의 click을 선택 Element의 활성 slot과 월드 Preview에 원자적으로 반영한다.

```text
visible DDS/WModel thumbnail click
→ Active Document와 selected Element 존재 확인
→ Element kind와 active slot compatibility 검사
→ Element copy의 binding 교체
→ Document copy를 CEffectObject::Stage_Document에 먼저 stage
→ 성공하면 Active Document와 selected resource를 commit
→ 같은 frame의 world Effect 갱신
→ 실패하면 기존 Document/Preview 유지와 card별 오류 표시
```

### `Update(fTimeDelta)`

한 줄 책임: Render 함수와 분리해 preview clock, anchor, world object snapshot을 한 frame에 한 번 갱신한다.

```text
CMainApp::Update
→ CEffect_Tool::Update
→ playing이면 sample time 증가
→ document duration에서 loop 또는 clamp
→ 선택 anchor의 current world matrix resolve
→ 실패면 EffectObject visible=false와 이유 기록
→ Document revision이 바뀌었으면 Stage_Document
→ Set_RootWorld + Set_SampleTime + visible=true
```

## G07-10. 편집 command와 rollback

- resource thumbnail click은 `Try_ApplyResourceFromThumbnail` command다. local copy와 renderer stage가 모두
  성공한 경우에만 Document를 바꾸므로 별도 `Bind Selected Resource`를 주 작업 흐름으로 남기지 않는다.
- Empty/Clear Slot은 같은 command 경로에서 해당 binding만 제거하고 fallback texture로 즉시 갱신한다.
- Delete Element는 선택 ID를 찾은 Document copy를 만들고, renderer stage 성공 후 commit한다.
- Clear All은 확인 command 뒤 빈 Elements Document를 stage하고 성공할 때만 교체한다.
- load/create/apply 실패 시 기존 Document, world Effect, selected Element를 유지한다.
- anchor resolve 실패 시 world Effect만 숨기며 Document를 수정하거나 origin으로 fallback하지 않는다.

## G07-11. project/filter 등록

`Client.vcxproj`의 기존 Effect Tool 항목 근처에 다음 물리 파일을 각각 `ClInclude`/`ClCompile`로 등록한다.

```text
Public\Effect_DocumentRenderer.h
Public\Effect_Object.h
Public\Effect_ThumbnailCache.h
Private\Effect_DocumentRenderer.cpp
Private\Effect_Object.cpp
Private\Effect_ThumbnailCache.cpp
```

`Client.vcxproj.filters`에는 기존 `Effect_Tool.h/.cpp`가 속한 동일 Header/Source 필터를 사용한다.
새 가상 폴더를 만들거나 기존 필터를 재배치하지 않는다.

## G07-12. 구현 순서

```text
G07-1  Effect_DocumentRenderer로 G06 draw 경로 추출
G07-2  CEffectPreview가 공용 renderer를 사용하도록 교체
G07-3  EffectObject prototype/clone/layer/world draw 연결
G07-4  Effect Tool Update와 document revision/anchor sync 연결
G07-5  다섯 dock window로 기존 panel 재배치
G07-6  slot card/category/Keyframes/DDS thumbnail과 click 즉시 commit 연결
G07-7  Character selector, actual clip, CharacterSpec weapon 목록 연결
G07-8  mouse world picking과 Player/Weapon/Bone/Clear pivot 연결
G07-9  Delete/Clear/Time Reset All과 실패 rollback 마감
```

## G07-13. 검증

### 자동 검증

```powershell
msbuild Engine/Default/Engine.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

G07은 Engine public 계약을 바꾸지 않으므로 Engine build는 회귀 확인이며 새 Engine 파일을 추가하지 않는다.

### 수동 smoke

```text
Client/Default에서 Debug Client.exe 실행
→ Lobby / Test / Development 진입
→ F1 / Effect Tool
→ 다섯 창이 default layout으로 동시에 표시
→ 새 Document와 Mesh/Sprite/Decal 세 layer 생성
→ Base/Noise/Mask/Emissive/Dissolve slot과 category 전환
→ DDS thumbnail 한 번 click으로 texture bind와 월드 결과 즉시 갱신
→ UV sequence Keyframes tile click으로 TileIndex와 화면 frame 즉시 갱신
→ WModel card click으로 model bind
→ Play 시 세 layer가 world에서 함께 변화
→ 현재 roster target 선택과 실제 clip 재생
→ Model View click으로 Mouse Viewport/World Position 확인과 WORLD pivot 설정
→ Set Effect Pivot Player에서 Character root를 따라감
→ 실제 CharacterSpec weapon을 고른 뒤 Set Effect Pivot Weapon에서 socket을 따라감
→ Clear Effect Pivot 뒤 마지막 picked WORLD pivot 유지
→ 실제 존재하는 bone 이름에서 effect가 pose를 따라감
→ 존재하지 않는 bone 입력 시 effect가 origin에 나타나지 않고 기존 Document 유지
→ Save / Discard / Load 후 layer와 binding이 동일
```

## G07-14. G07에서 하지 않는 것

- Particle와 Trail을 Rect 한 장으로 가짜 표시하지 않는다.
- arbitrary curve editor, particle simulation, trail history는 G08이다.
- EffectAssetId 제품 admission, animation EFFECT cue, gameplay spawn은 G09이다.
- legacy 459개 `.effect/.weffect` 후보를 현재 v4 JSON으로 자동 승인하지 않는다.
- RGB split, zoom blur, Alt+V cinematic은 G07 완료 조건이 아니다.

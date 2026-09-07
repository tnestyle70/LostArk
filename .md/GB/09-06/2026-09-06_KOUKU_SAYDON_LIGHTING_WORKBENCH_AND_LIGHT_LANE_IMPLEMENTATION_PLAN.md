# 2026-09-06 쿠크세이튼 조명 저작 — Rendering Workbench Light 확장과 Composition Light lane 구현 계획서

> 문서 종류: 구현 계획서. 범위, 데이터 정본, 실제 호출 흐름과 G별 변경 단위를 설명한다.
> 상위 계획: `../09-04/2026-09-04_KAKUL_SAYDON_ENCOUNTER_BOSS_TOOL_LIGHTING_COMPANION_IMPLEMENTATION_PLAN.md` 4절·G05~G07.
> 2026-09-07 교정 기준: 사용자의 최신 Map / Character / Boss 분류, 편집 가능한 기본 방향광,
> Level별 quality 저장, Map 원본 조명의 Workbench 재사용 요구와 현재 구현 구조를 반영한다.
> 구현·자동 검증·사용자 화면 확인의 완료 여부는 대응 RESULT가 소유한다. 이 문서는 실행 이력이나 PASS를 대신하지 않는다.

## 0. 목표와 적용 범위

### 0.1 사용자가 조작할 모델

Rendering Workbench에서 Level을 선택하고, 그 Level의 Scene Profile과 조명을 한 화면에서 편집한다.
`Light Resources` 탭의 `All Lights` 목록은 `All / Map / Character / Boss`로 분류한다.
이 분류는 조명이 놓이거나 따라갈 기준이고, 광원의 종류는 별도 `Directional / Point / Spot`이다.

- **Map**은 맵의 기본 방향광, Area에 저장한 배치 조명, 패턴에서 재사용할 MAP resource를 보여 준다.
- **Character**는 `PLAYER` anchor resource다. 패턴에서 살아 있는 복제 플레이어마다 조명을 제출한다.
- **Boss**는 `BOSS` anchor resource다. 해당 패턴을 실행하는 보스 entity를 기준으로 한다.
- **Default Directional Light**는 선택한 Scene Profile의 기존 방향광을 편집하는 행이다. 방향·Diffuse·Ambient·Specular를
  수정하고 저장할 수 있다. 별도 Map 배치나 LightResources 복사본을 만들지 않는다.

쿠크세이튼 Workbench의 `Light` 탭은 재사용 resource와 현재 Area의 Map 원본 조명을 함께 보여 준다.
선택한 항목을 Append하면 timeline의 LIGHT box가 되고, box의 시작·길이·anchor·offset·rotation·brightness·fade가
패턴 중의 재생을 정한다. 조명의 색·거리·cone 등 정의 값은 Rendering 쪽 정본에 남는다.

### 0.2 첫 입력 데이터

쿠크 Area는 `LV_LUT_MIDNIGHTC_ED`, Level base profile은 `scene.kakulsaydon.g1.base.v1`이다.
초기 Map light source는 formatVersion 2, `nextLightOrdinal: 1`, `lights: []`로 둔다.
실제 무대 spot 위치를 대신 결정하거나 저장하지 않는다. 사용자가 Create와 Detail에서 배치한다.

LightResources의 초기 두 항목은 아래 값으로 만든다. 이는 튜닝 시작값이며 화면 적합성의 확정값은 아니다.

| ID | 이름 | anchor | localOffset(m) | rotationDegrees | range / falloff | inner / outer | RGB / brightness |
|---|---|---|---|---|---|---|---|
| `light.runtime.1` | 스포트라이트조명_캐릭터 | PLAYER | `(0,8,0)` | `(90,0,0)` | 16 / 2 | 8° / 18° | white / 4 |
| `light.runtime.2` | 스포트라이트조명_세이튼 | BOSS | `(0,10,0)` | `(90,0,0)` | 16 / 2 | 10° / 24° | white / 4 |

두 항목의 kind는 SPOT이며 catalog의 `revision`은 1, `nextLightResourceOrdinal`은 3이다.
Map source가 빈 상태라는 사실과 resource가 두 개 있다는 사실을 구분한다.

## 1. 현재 소유자와 유지할 실행 경계

### 1.1 조명과 quality의 소유자

| 입력 | 저장 정본 | 소비자와 수명 |
|---|---|---|
| Level base와 Scene mood | `Data/Rendering/Authored/RenderingProfiles.json` | `CRenderingProfileService`; Level 기본값 및 패턴의 일시적 Scene Profile |
| Area 조명 배치 | `Data/Maps/Authoring/<Area>/<Area>.maplights.json` | `CMapLightPresentationRuntime`; 현재 Level의 배치 light 또는 저작 preview |
| 재사용 조명 정의 | `Data/Rendering/Authored/LightResources.json` | `CLightResourceCatalog`; Composition resource ID lookup 및 Debug preview |
| 패턴 조명 사용 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | `CKoukuSaydonPresentationPlayer`; Server pattern clock의 LIGHT box 창 |
| Effect V1 조명 | 기존 Effect light element | 기존 `Try_BuildEffectPointLightDesc`와 transient light 제출 |

Engine은 범용 POINT/SPOT/DIRECTIONAL을 렌더링한다. Client는 Area·resource·패턴 anchor와 시간을 소유한다.
Server에는 광원, RGB, cone, asset ID를 보내지 않는다. Effect V2에 별도의 light family를 추가하지 않는다.

### 1.2 Level quality와 임시 Scene Profile을 분리한다

`CRenderingProfileService`는 active Scene Profile ID와 Level quality owner ID를 따로 보존한다.
`SCENE_RENDERING_PROFILE`의 optional `qualityOverride`가 Level별 SSAO·Bloom·Exposure·FXAA 설정을 저장한다.
설정이 없는 기존 profile은 catalog의 global quality를 사용한다.

- `Activate_LevelProfile(id)`는 Level quality owner와 기본 Scene Profile을 함께 활성화한다.
- `Activate_Profile(id)`는 현재 Level quality를 유지한 채 Scene mood와 exposure/bloom 배율을 적용한다.
- Workbench의 `Selected Level Quality`는 Level descriptor의 base profile에 저장한다. 선택한 mood가 달라도 저장 대상은
  표시된 `Quality owner`다. 일반 Scene 항목과 Default Directional Light는 선택한 Scene Profile에 저장한다.
- Level 이동·로딩·진입 실패 rollback은 quality owner와 active Scene을 함께 다룬다. 패턴이 어두운 profile을 켜거나 복구해도
  Level quality가 다른 Level/global 값으로 바뀌지 않아야 한다.
- Duplicate한 mood는 `qualityOverride`를 복제하지 않고 현재 Level quality를 상속한다. 삭제는 active/Level owner,
  필수 base 및 Composition 참조 profile을 보호한다.

### 1.3 Frame transaction

`CPresentation_Manager`는 frame provider를 제출하면서 transient light 목록을 새로 만든다.
최대 64개이며 `Add_TransientLight` 실패는 provider 통계를 깨뜨려 전체 frame 실패로 이어질 수 있다.
따라서 각 provider는 유효한 후보만 만들고, 남은 예산 안의 개수만 제출한다.
잘못된 LIGHT occurrence, 잠시 없는 anchor, 예산 부족 때문에 다른 presentation row를 지우지 않는다.

scene light와 transient light는 기존 deferred light accumulation에 각각 full-screen draw로 합산한다.
Spot은 기존 point 거리 감쇠에 cone 감쇠를 곱하고, transient directional에는 directional shadow를 적용하지 않는다.

## 2. 변경 파일과 책임

아래는 이번 조명 기능의 물리 소스 경계다. 기존 MainApp의 상태 소유권을 유지하고 조명 UI 구현만 별도 CPP로 나눈다.

| 구분 | 절대 경로 | 책임 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Engine_Enum.h` | LIGHT::SPOT, DEFERRED::SPOT 값 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Engine_Struct.h` | LIGHT_DESC 뒤에 cone cosine 두 필드 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Light.cpp` | Spot 검증·상수 binding·pass 선택 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Light_Manager.cpp` | scene light 혼합 유형 검증과 rollback |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Presentation_Manager.cpp` | transient 유형·방향·cone 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp` | 기존 pass index 및 새 pass 끝 위치 단언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | 기존 Point 공통 계산과 Spot pass 16 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/LightResourceCatalog.h` | LIGHT_RESOURCE, authored/runtime 조회·수정·저장 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/LightResourceCatalog.cpp` | strict catalog, Map 원본 alias, desc 변환, atomic 저장, publisher 호출 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/AreaLightAuthoringSession.h` | Area draft와 preview provider의 수명 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/AreaLightAuthoringSession.cpp` | Create/Update/Delete/Save/Publish, 외부 변경 감지 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp` | All Lights 목록·Detail·wire·3초 preview |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h` | 선택 Level/profile/light와 저작 session 멤버 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | 기존 Rendering Workbench, Level lifecycle, resource inventory 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapLightDocument.h` | v1 보존과 v2 레코드·ordinal·Serialize 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapLightDocument.cpp` | v2 exact parse, staged replacement, v1 저장 거부 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapLightPresentationRuntime.cpp` | kind별 desc와 enabled light 제출 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_KakulSaydonArena.h` | Map light runtime 및 저작 override |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_KakulSaydonArena.cpp` | 진입 load, update 제출, reload 및 퇴장 정리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/RenderingProfileService.h` | 선택 profile 편집과 별도 Level quality owner |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/RenderingProfileService.cpp` | qualityOverride parse/save, 활성화·복제·삭제·rollback |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp` | 쿠크 base profile ID |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp` | LIGHT Product 읽기·anchor 샘플·frame provider |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | LIGHT 정의 join·box 검증·Product projection |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/RenderingPipeline/light_resources_pipeline.py` | LightResources 및 map v2 strict 검증·원자적 resource publish |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/RenderingPipeline/Publish-LightResources.ps1` | LightResources domain의 명시적 Validate/Publish 진입점 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1` | 기존 Map file-set publisher에 v2 검증 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp` | 기존 point/Valtan 증거에 Spot·v2 보존 검사 추가 |

Composition document/workbench/editor H/CPP는 기존 presentation family enum, resource 목록, box Detail와 timeline에 LIGHT를 추가한다.
`Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`는 optional qualityOverride와 새 필수 쿠크 base를 검증한다.
대응 Python 테스트는 새 schema와 실제 publish의 실패 보존을 검사한다.

## 3. 등록과 배포

- `Client/Default/Client.vcxproj`: 새 `LightResourceCatalog`, `AreaLightAuthoringSession` H/CPP,
  `MainApp_RenderingLighting.cpp`를 `<ClInclude>`/`<ClCompile>`로 등록한다.
- `Client.vcxproj.filters`: 각각 기존 `03. Tools\10. Rendering`, `03. Tools\00. Map`, `00.MainApp`에 둔다.
  새 `CRenderingTool` 클래스나 별도 ownership 경로를 만들지 않는다.
- LightResources source와 쿠크 maplights source는 `<None>`으로만 등록하고 `96.DataFiles` 계열에 노출한다.
- `Data/Maps/MapCatalog.json`의 쿠크 Area에 `sourceLights/lights` pair를 함께 선언한다. reader·publisher·빈 v2 source·runtime을
  같은 기능 변경으로 묶는다. Valtan의 기존 v1 문서 22개 값과 배포 문서는 그대로 보존한다.
- LightResources publish 목적지는 `Client/Bin/DataFiles/Rendering/LightResources.runtime.json`이다.
  Map publish 목적지는 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.maplights.json`이다.
- `Tools/Build/BuildDomains.json`의 쿠크 Product 입력에는 LightResources와 쿠크 maplights source 및 검증 tool 의존을 등록한다.
  쿠크 map domain 출력에는 `.maplights.json`을 포함한다. Product 빌드에 광역 진단을 새 기본조건으로 붙이지 않는다.
- Engine shader와 public header의 SDK/Client 배포는 정상 Product MSBuild가 담당한다. 생성된 runtime JSON과 source는 같은 기능
  변경 단위로 다루고 binary·EngineSDK·중간 산출물은 소스 커밋에 넣지 않는다.

## 4. 데이터와 실행 계약

### 4.1 Area maplights v2

문서 root의 exact field는 `schema, formatVersion, areaId, provenance, nextLightOrdinal, lights`다.
`schema=lostark.map-light-presentation`, `formatVersion=2`, `provenance=PROJECT_AUTHORED`를 사용한다.
lights는 빈 배열을 포함해 최대 64개다.

| field | 의미와 유효 조건 |
|---|---|
| `lightId` | 1~128-byte stable ASCII ID, 중복 불가. vector index나 포인터를 저장하지 않음 |
| `displayName` | 1~256-byte 유효 UTF-8 이름 |
| `kind` | POINT / SPOT / DIRECTIONAL |
| `groupId` | 비어 있지 않은 stable ASCII ID |
| `enabled` | bool; Map의 상시 제출 여부 |
| `position` | world XYZ(m), 3개 유한값 |
| `rotationDegrees` | XYZ Euler degree 3개; 회전한 +Z가 광원 방향 |
| `rangeMeters` | POINT/SPOT의 거리 감쇠 반경 0.01~1000m; DIRECTIONAL은 0 |
| `falloffExponent` | 0.01~64 |
| `innerConeDegrees`, `outerConeDegrees` | SPOT은 `0 < inner <= outer <= 89.9`; 다른 type은 모두 0 |
| `color` | RGBA 4개, 각 0~1 |
| `brightness` | 0~64 |

position/rotation 각 성분은 -100000~100000이다. DIRECTIONAL position은 `(0,0,0)`이다.
`nextLightOrdinal`은 1~4294967294 정수다. parser는 arbitrary stable lightId를 허용하며 suffix 형식을 강제하지 않는다.
Create는 `light.<AreaId>.<nextLightOrdinal>`을 생성하고 검증 성공 후 ordinal을 증가시킨다. 기존 ID와 충돌하면 교체하지 않는다.

기존 C++ 레코드 이름 `MAP_POINT_LIGHT_RECORD`를 유지하고 kind·name·group·enabled·rotation·cone을 확장한다.
v1은 기존 sourceLevel/sourceObjectId/radiusMeters 계약 그대로 parse하고 `kind=POINT, enabled=true`로 소비한다.
v1에 `Serialize`나 authoring replacement를 허용하지 않는다. v2 Parse/Replace/Load는 실패하면 마지막 정상 문서를 유지한다.

### 4.2 LightResources catalog와 Map alias

문서 root exact field는 `schema, formatVersion, revision, nextLightResourceOrdinal, lights`다.
`schema=lostark.light-resources`, `formatVersion=1`, lights 0~4096개를 사용한다.
row exact field는 다음과 같다.

```text
lightResourceId, displayName, kind, defaultAnchorKind,
localOffset, localRotationDegrees, rangeMeters, falloffExponent,
innerConeDegrees, outerConeDegrees, color, brightness
```

ID/name/type/숫자 규칙은 4.1과 같다. `defaultAnchorKind`는 MAP/PLAYER/BOSS이고 사용자 화면은 Map/Character/Boss로 표시한다.
revision과 nextLightResourceOrdinal은 1~4294967294다. 자동 생성 ID `light.runtime.N`은 N이 양수이고
nextLightResourceOrdinal보다 작아야 한다. 그 외 stable ID도 허용한다.

`CLightResourceCatalog`는 authored와 published runtime 목록을 분리한다.
`Refresh_MapResources(areaId)`는 Area v2의 source/runtime을 읽어 같은 lightId의 MAP alias를 각각 만든다.
이 alias는 조회용 view이며 LightResources.json에 복사·저장하지 않는다. Map의 position은 MAP identity anchor 기준 localOffset으로
그대로 해석한다. Map `enabled`는 Area 상시 제출 상태이며, alias를 Append한 LIGHT box의 재생 창은 occurrence가 정한다.

`Find_Resource`는 저작 preview/Workbench용, `Find_RuntimeResource`는 제품 재생용이다.
Map source를 Save하면 authored alias를 새로 읽고, Publish/Reload하면 runtime alias도 새로 읽는다.
Composition에서 참조 중인 ID의 삭제는 거부하고 원본과 기존 preview를 보존한다.

C++ 저장은 `validate -> serialize -> reparse -> temp write -> re-read -> source 변경 검사 -> atomic replace` 순서를 따른다.
Python publisher는 원본 JSON의 중복 key, 알 수 없는 field, bool을 숫자로 쓴 입력, NaN/Infinity, 잘못된 enum/ID/범위를 거부한다.
float32 경계는 C++ 저장 왕복과 맞춘다. 실패하거나 저장 도중 source가 바뀌면 기존 runtime을 보존한다.

### 4.3 Composition LIGHT

`KOUKU_SAYDON_PRESENTATION_KIND::LIGHT`는 COLLIDER 다음 값 4다. presentation family 배열은 5개가 된다.
Resources의 Light 탭은 `CLightResourceCatalog`의 합쳐진 조회 목록을 소비하며, resourceId와 assetId는 lightResourceId를 참조한다.
resourceKind는 빈 문자열이다. 광원의 거리·cone·색을 Composition에 중복 저장하지 않는다.

LIGHT occurrence는 기존 startMs/durationMs/positionOffset/rotationDegrees/fadeInMs/fadeOutMs/followBoss를 사용하고
`brightnessMultiplier`를 추가한다. LIGHT scale은 `(1,1,1)`이며 anchor는 MAP/PLAYER/BOSS만 허용한다.
다른 presentation의 WORLD anchor 계약은 기존대로 남는다.

| anchor | pivot과 수명 |
|---|---|
| MAP | identity 기준 고정 world 위치. Map alias는 원본 position에서 재생 |
| PLAYER | 살아 있는 모든 복제 player의 transform. follow 필수, bone 없음. player가 잠시 없으면 다음 frame에 다시 평가 |
| BOSS | 해당 pattern session의 boss transform과 선택 bone. follow 해제 시 시작 pivot 유지 |

local resource transform과 occurrence offset/rotation, anchor transform을 순서대로 합성한다.
`Try_BuildLightDesc`는 basis scale을 제거하고 방향을 정규화해 캐릭터 model scale이 광원 range를 바꾸지 않게 한다.
fade와 brightnessMultiplier는 resource brightness에 곱한다.

projector는 재사용 catalog와 해당 Area map source를 strict join하고 Product에는 stable assetId와 occurrence 값만 넣는다.
LIGHT는 Client presentation Product만 생성하며 Server encounter의 Collider 영역으로 투영하지 않는다.
잘못된 LIGHT Product row는 그 row를 격리하고 정상 pattern/presentation을 유지한다.

### 4.4 Engine SPOT 계약

```text
LIGHT: DIRECTIONAL, POINT, SPOT, END
DEFERRED: 기존 0~15 유지, SPOT=16, END=17
LIGHT_DESC: 기존 필드 뒤에 fSpotInnerCos=1.f, fSpotOuterCos=1.f append
sizeof(LIGHT_DESC)=100; fRange=36, fFalloffExponent=40, vDiffuse=44 offset 유지
fSpotInnerCos offset=92; fSpotOuterCos offset=96
```

`CLight::Render_Desc`는 SPOT의 position/direction/range/falloff/cone cosine을 바인딩하고 `DEFERRED::SPOT`을 선택한다.
POINT/SPOT은 양수 range, 모든 타입은 양수 유한 falloff를 요구한다.
SPOT/DIRECTIONAL 방향은 유한하고 길이 제곱이 1e-6보다 커야 한다. SPOT은 `0 < outerCos <= innerCos < 1`이다.
모르는 enum을 정상 타입으로 치환하지 않는다.

`Shader_Deferred.hlsl`은 기존 point 계산을 `Resolve_LocalLight`로 공유한다.
Spot에서는 radial attenuation에 다음 cone weight를 곱한다.

```text
cone = saturate((dot(normalize(worldPosition-lightPosition), normalize(lightDirection)) - outerCos)
                / max(innerCos-outerCos, 0.0001))
attenuation *= cone * cone
```

광원 원점의 normalize 예외를 막는다. `pass Spot`은 technique 마지막에 추가하고 Point와 동일한 additive blend를 사용한다.
Scene과 transient 모두 SPOT을 허용한다. transient directional과 spot에는 profile directional shadow를 적용하지 않는다.

### 4.5 Frame 제출과 정리

```text
Kouku Level update -> Map runtime 또는 authored override -> Add_FrameProvider
Kouku PresentationPlayer update -> boss/preview session LIGHT 샘플 -> Collect_FrameLights -> Add_FrameProvider
MainApp preview update -> 3초 resource preview -> Add_FrameProvider
Renderer -> Submit_FrameProviders -> scene/transient deferred light accumulation
```

Map provider는 enabled 후보만 만들며 현재 제출 수에서 56개까지의 여유를 사용해 후속 조명 여지를 남긴다.
Composition provider와 3초 preview는 64개 전체 cap에서 이미 사용한 개수를 뺀 범위만 제출한다.
후속 provider까지 포함한 전역 우선순위 scheduler를 새로 만들지 않는다. 각 provider는 실제 제출 개수에 맞는
expectation을 등록하며 skip을 오류 제출로 기록하지 않는다. UI에는 frame light 수와 budget skip 수를 표시한다.

LIGHT의 asset/pivot/desc가 없거나 잘못되면 해당 occurrence의 제출만 생략한다.
PLAYER가 없거나 bone이 아직 준비되지 않은 경우와 예산 부족은 다음 frame에 다시 평가한다.
box 종료·Reset·Level 퇴장·Product 교체에서는 더 이상 그 row를 제출하지 않는다. 별도 restore token은 필요하지 않다.

### 4.6 Rendering Workbench 입력과 저장

상단 `Level category`는 여섯 제품 Level의 descriptor/base를 선택한다. `Scene profile` 선택은 active profile과 독립적이다.
패턴이 active profile을 바꿔도 사용자가 고른 draft를 바꾸지 않는다. `Activate Selected`, `Level Base`, `Duplicate As`,
`Delete Selected Profile`로 선택과 적용을 분리한다.

`Light Resources -> Create Light`는 Name, Anchor type, Light type을 받는다.
Map 기본 생성은 현재 선택 Level에 진입해 있고 편집 가능한 Area source와 player가 있을 때 player 위치+8m에서 시작한다.
Map/Directional은 기존 Default Directional Light를 선택한다. `Reusable Map resource (pattern lifetime)`을 체크하면
MAP resource를 만든다. Character/Boss는 항상 재사용 resource를 만든다.

Detail은 이름·position 또는 offset·rotation·range·falloff·RGB·brightness·cone을 편집한다.
Spot의 `Point down`은 `(90,0,0)` 회전으로 바꾸며 floor radius helper와 Debug wire가 거리/cone 튜닝을 돕는다.
Spot의 radial range 끝 wire는 중심축 거리 `range*cos(outer)`와 반경 `range*sin(outer)`로 표시한다.

- Default Directional Light의 Save/Publish/Reload는 RenderingProfiles 저장 경로로 간다.
- Map placement는 `CAreaLightAuthoringSession`으로 draft와 preview를 함께 갱신한 뒤 Save/Publish/Reload한다.
  v1은 읽기 전용이다. 쿠크 Level은 저작 override를 기존 Map runtime 대신 제출하고 override 해제 뒤 published runtime으로 돌아간다.
- 재사용 resource는 catalog의 Create/Update/Delete/Save/Publish/Reload를 사용한다. 3초 preview는 선택 anchor가 준비되어야 시작한다.
  MAP preview는 시작 위치를 유지하고 Character/Boss preview는 해당 target을 따라간다. Level 변경과 Stop Preview에서 정리한다.
- Map 및 resource의 Delete는 Composition reference를 확인한다. 저장 실패·잘못된 입력·외부 source 변경은 기존 문서와 preview를 보존한다.

## 5. G별 변경 단위

### G00 — Resource schema, publisher, 데이터와 등록

LightResources source 두 seed, strict Python validator/atomic publisher와 runtime 산출 경로를 연결한다.
Map publisher에 v2 분기를 추가하되 기존 v1 검증은 유지한다. C++/None/filter/BuildDomains 의존을 등록한다.
검증은 LightResources Validate, 기존 publisher 실패 보존 테스트, JSON/XML parse와 scoped diff check다.

### G01 — Engine Spot

4.4의 enum/struct/light manager/presentation/shader를 한 변경으로 적용한다.
기존 PointLightFalloffContractHarness에 cone binding·유효 혼합 타입·잘못된 cone/direction rollback을 추가한다.
검증은 Product Debug, 기존 harness 빌드·실행, compiled shader closure, Engine/Client shader 동일성이다.

### G02 — Map v2 reader, runtime와 Area 연결

v2 Parse/Serialize/Replace_Authored, 기존 레코드 확장, kind별 enabled 제출과 Level override/reload를 연결한다.
쿠크 MapCatalog pair와 빈 v2 source/runtime을 함께 등록한다. Valtan v1 22개를 재저장하지 않는다.
기존 harness에서 v2 Parse→Serialize→Parse 값 보존, 잘못된 cone의 last-good 보존, 빈 v2와 v1 Serialize 거부를 검사한다.
Map publisher의 실제 file-set publish에서도 실패 시 기존 runtime 전체가 유지되는지 확인한다.

### G03 — Light catalog, Map 원본 alias와 preview

CLightResourceCatalog가 authored/runtime과 Map alias를 분리해 조회한다. resource 수정·삭제·atomic Save·Publish를 연결한다.
Workbench resource inventory에는 두 seed와 저장된 Area alias가 들어간다. alias를 별도 catalog에 쓰지 않는다.
MainApp의 3초 preview는 선택 anchor, Level 수명, 유효 desc와 남은 예산을 사용한다.

### G04 — Rendering Workbench와 Level quality

MainApp 소유권을 유지하고 `MainApp_RenderingLighting.cpp`로 조명 UI를 분리한다.
Map/Character/Boss 목록, 편집 가능한 기본 방향광, Area 저작, resource Detail, profile 목록/복제/보호 삭제를 제공한다.
optional qualityOverride와 Level owner lifecycle을 연결해 Scene Profile box 전환에도 Level quality를 보존한다.
RenderingProfiles source/runtime, 새 쿠크 base, required ID, LevelRegistry를 같은 변경으로 묶는다.

### G05 — Composition LIGHT와 제품 재생

기존 presentation family에 LIGHT를 추가하고 Light 탭 Append, timeline, Box Detail, 저장·projection·Product reader를 닫는다.
MAP/PLAYER/BOSS와 brightness/fade를 적용하고 잘못된 row·일시적 anchor 부재·budget skip을 격리한다.
진짜/가짜 쿠크는 기존 각 boss session의 패턴 소유로 구분한다. 별도 fake-boss anchor나 Server 조명 protocol을 만들지 않는다.

## 6. 필요한 검증과 사용자 확인

자동 검증은 실제 변경 domain에 한정한다. 명령 실행 결과와 로그는 RESULT에 따로 기록한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-LightResources.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
python -m unittest Tools.RenderingPipeline.test_publish_light_resources -v
python -m unittest Tools.RenderingPipeline.test_publish_rendering_profiles -v
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode validate
python -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Debug
git diff --no-index Engine/Bin/ShaderFiles/Shader_Deferred.hlsl Client/Bin/ShaderFiles/Shader_Deferred.hlsl
git diff --check
```

harness 실행 전에는 기존 harness project를 Debug|x64로 빌드한다. Product와 병렬로 중복 빌드하지 않는다.
LightResources/Map/RenderingProfiles Publish는 각 domain의 명시적 publisher로 실행한다.
저장·runtime 파일·project/filter XML parse, 실패 보존과 Valtan v1 parity를 함께 확인한다.

사용자는 팀 LAN Server가 준비된 상태에서 지정된 Client 시작 대상으로 직접 실행하고 Lobby에서 KoukuSaydon으로 진입한다.
F1 Developer Tools의 Rendering Workbench에서 Level category `KoukuSaydon`을 선택하고 다음을 확인한다.

1. Light Resources의 초기 두 resource와 Map의 Default Directional Light가 표시되는지 확인한다.
2. 기본 방향광을 수정하고 저장·재로드했을 때 값이 보존되는지 확인한다.
3. Map Spot을 Create하고 위치·방향·cone·RGB를 튜닝한 뒤 Save/Publish/재진입으로 배치를 확인한다.
4. 쿠크 Workbench Light 탭에서 Character/Boss/Map 항목을 Append하고 box 시간과 anchor를 설정한다.
5. local preview 및 publish 후 실제 패턴에서 진짜/가짜 쿠크·살아 있는 플레이어의 조명 창과 위치를 확인한다.
6. Scene Profile box 전후와 Level 이동 뒤에도 저장한 quality가 해당 Level 기준으로 유지되는지 확인한다.

에이전트는 Client/UI를 자율 실행·조작하거나 화면을 캡처하지 않는다.
무대의 실제 밝기·색·cone 폭과 visual fidelity는 사용자 관찰만 최종 증거로 기록한다.

## 7. 유지할 경계

- 초기 Map source가 비어 있으므로 무대 spot은 사용자의 배치·튜닝 대상이다. 자동 검증으로 위치를 확정하지 않는다.
- Area 상시 light와 같은 Map alias를 LIGHT box로 함께 켜면 두 번 합산될 수 있다. `enabled`와 box 사용 목적을 저작 시 구분한다.
- Scene Profile의 `blendMs`는 기존 저장 계약이며 현재 profile 전환은 즉시 적용이다. 이번 G00~G05는 보간 기능을 추가하지 않는다.
- modifier catalog, light group blend, token restore, 새 protocol, Effect V2 light family와 전역 light scheduler는 범위에 넣지 않는다.
- 다른 팀의 미커밋 변경을 되돌리지 않는다. 실행하지 않은 검증과 사용자가 하지 않은 화면 확인을 완료로 기록하지 않는다.


## 8. 09-07 사용자 검증 후 리소스 용도·독립 창·Sequencer 확장

현재 Character/Boss Spot preview는 사용자 확인을 받았다. 저장한 Directional 값과 기존 조명 재생을 보존한다.
Light Resources는 왼쪽, Light Sequencer는 아래, Light Detail과 Rendering Workbench는 오른쪽의
독립 창으로 나눈다. 기존 Action Workbench의 첫 배치·Windows 메뉴·레이아웃 초기화 방식을 따른다.

Light Resources의 상위 용도는 Map Profile / Scene Profile / Anchor Light다.
Map Profile은 현재 Level의 기본 Directional과 맵 배치 조명, Scene Profile은 패턴에서 쓰는
기존 RenderingProfiles의 분위기 profile, Anchor Light는 기존 LightResources의 Map/Character/Boss 정의다.
Create는 용도와 광원 종류(Direction/Point/Spot)를 구분하고 해당 기존 정본에만 저장한다.
Create Light와 All Lights, profile 생성·목록은 모두 Light Resources가 소유한다.

Light Detail은 선택 조명과 scene 분위기의 편집·저장을, Rendering Workbench는 Level의 FXAA/SSAO/Bloom 등
quality와 benchmark를 소유한다. 같은 광원 정본을 두 번째 저장소에 복사하지 않는다.
Light Sequencer는 선택 리소스의 Lifetime, Play/Pause/Stop/Seek를 기존 preview 제출 경로에 연결한다.
Map 재생은 임시 preview 문서를 사용하고 저장 배치 enabled나 profile 값을 파괴하지 않는다.
기본 Directional은 기존 Scene 광원을 편집하며 중복 transient Directional을 생성하지 않는다.

사용자 편집 원본을 보존하고 관련 codec/publisher 검증과 Debug Product 빌드 후 실행 준비를 마친다.
추가 화면 확인은 사용자가 직접 하며 자동 시각 PASS는 기록하지 않는다.


### 8.1 암전과 전원 Spot 패턴 연결

기존 scene.kakulsaydon.find-true-dark.v1은 ID를 유지하고 optional displayName에 씬프로필_암전을 저장한다.
RenderingProfiles codec·publisher와 Light Resources/Detail/Sequencer가 같은 이름을 소비한다.
기존 LightResources 두 항목도 ID와 튜닝값을 유지하면서 스포트라이트_캐릭터 / 스포트라이트_세이튼으로 표시한다.

진짜 세이튼 찾기는 기존 Scene 창 2007~26134ms를 보존하고 두 Spot을 같은 창에 배치한다.
댄스타임은 0~31467ms, 룰렛은 0~33669ms에 Scene Profile과 두 Spot을 배치한다.
Character 조명은 같은 방 Server snapshot의 현재 캐릭터 전원을 대상으로 하며 HP로 제외하지 않는다.
Server가 확정한 patternId/startTick/sequence를 기존 Client presentation이 소비한다. 별도 protocol은 추가하지 않는다.

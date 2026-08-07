# 2026-08-07 Effect Tool Mesh 제작 레퍼런스 정합 구현 계획서

## 1. 결정

Cascade 자동 추출은 시간·Transform·원본 리소스 증거를 보존하는 참고 경로로 유지한다. 이번 작업의 제품 목표는 원본 픽셀 자동 복원이 아니라, 레퍼런스 Effect Tool과 같은 순서로 큰 Mesh 이펙트를 빠르게 직접 조립하고 저장하는 제작 경로를 완성하는 것이다.

```text
Effect class domain 선택
→ Mesh thumbnail 선택
→ Base 필수 선택
→ Noise / Mask / Emissive / Dissolve 선택
→ Create Effect 원자 생성
→ Effect Detail live preview 편집
→ Apply / Save / Reload
→ All Effects에서 여러 Mesh layer 조립
```

신규 제작 화면에는 Particle 종류를 노출하지 않는다. 다만 기존 차원술사 11스킬의 Cascade 문서, `EFFECT_ELEMENT_KIND::PARTICLE`, SourceRecipe, playback과 진단 계층은 삭제하거나 변환하지 않는다.

## 2. 실측 근거

`Client/Bin/Resources/Effect`의 최상위 5개 폴더는 자산 종류가 아니라 제작 도메인이다.

| Domain | WModel | DDS | 기타 |
|---|---:|---:|---:|
| Artist | 35 | 237 | 0 |
| DimensionMaster | 140 | 701 | VectorField 4 |
| LanceMaster | 69 | 286 | 0 |
| Valtan | 52 | 346 | 0 |
| Warlord | 149 | 652 | 0 |

현재 Tool에는 다음 기반이 이미 존재한다.

- Resources-relative stable asset ID와 재귀 catalog scan
- Domain / 물리 folder / 검색 / clipped thumbnail grid
- DDS와 WModel offscreen thumbnail cache
- `meshModel`, Base, Noise, Mask, Emissive, Dissolve typed slot
- Transform, Color, UV, Timing, Lerp, render profile Detail
- staged world preview와 atomic Save/Reload/stale writer guard

현재 핵심 결손은 선택된 기존 Element를 편집하는 경로만 있고, 리소스를 먼저 고른 뒤 한 번에 새 Mesh layer를 생성하는 레퍼런스식 builder가 없다는 점이다.

## 3. 고정 제작 계약

### 3.1 Manual Mesh Builder

생성 전 상태는 active document와 분리된 `Mesh Authoring Draft`가 소유한다.

```text
elementId
domainId
meshModel                required WModel
base                     required safe 2D DDS
noise                    optional DDS
mask                     optional DDS
emissive                 optional DDS
dissolve                 optional DDS
renderProfile            stable enum
```

Thumbnail 선택은 builder draft만 변경한다. `Create Effect`는 이름이 같은 editable active document를 복사하거나 새 문서를 만들고 Element 하나를 추가한 뒤 `validate → stage playback/renderer → atomic save → active commit`을 한 번 수행한다.

```text
kind                     MESH
materialTemplate         effect.standard
useModelMaterial         false
sourceRecipe             disabled
meshModel + base          required
optional texture slots    selected bindings only
```

중복 ID, 다른 domain, 잘못된 file kind, unsafe Base, resource stage 실패 또는 document 검증 실패 시 active document와 현재 preview는 변하지 않아야 한다.

### 3.2 Resource browser

- Domain은 실제 5개 폴더를 그대로 사용한다.
- Mesh와 texture 후보를 누락 없이 `All`에서 탐색할 수 있어야 한다.
- 물리 folder와 검색은 현재 catalog를 재사용한다.
- 이름 기반 Ring/Slash/Crack/Sphere/Cylinder/Helix/Box 분류는 UI filter hint일 뿐 저장 정본이나 자동 binding 근거로 사용하지 않는다.
- 원본 binding 증거와 이미 같은 slot에 사용된 자산은 우선 노출할 수 있지만 후보를 숨기지 않는다.
- Base는 `blankwhite`, normal/bump, `_n` normal 표기와 cube texture를 차단한다. 이름 추측으로 normal을 Noise에 자동 연결하지 않는다.

### 3.3 Shader slot 의미

현재 셰이더 계약을 UI와 동일하게 유지한다.

| Slot | 실행 의미 |
|---|---|
| Base | RGB 기본색, A 기본 opacity |
| Noise | RG signed UV distortion, R은 dissolve 보조 |
| Mask | R opacity multiplier |
| Emissive | RGB HDR contribution × intensity |
| Dissolve | R threshold |

표준 `effect.standard`에서 Noise가 Base 표면 UV를 실제로 왜곡하도록 기존 Distortion Intensity를 사용한다. `PassName` 자유 문자열은 만들지 않고 현재 5개 Blend/Depth/Cull render profile enum을 유지한다.

### 3.4 Effect Detail과 저장

기존 v12 필드를 재사용한다.

- Lerp Position / Rotation / Revolution / Scaling / Velocity / Color
- Color Offset / Clip / Multiply, Bloom, Distortion, Radial
- UV Start / Speed / Wave / Sequence / Loop / Tile
- Lifetime / Start Delay / After Image / Dissolve Start
- stable Render Profile

Position/Rotation/Revolution/Scale/Velocity/Color/UV/Timing의 Start 값은 Lerp 체크 여부와 관계없이 입력 프레임에 즉시 live preview한다. 각 `Lerp ...` 체크박스는 해당 Start→End 수명 보간만 활성화하며, OFF→ON 시 preview 시간을 0초로 되돌리고 재생하여 회전·이동·크기 변화가 바로 보이게 한다.

Bloom Intensity는 Emissive texture가 연결된 경우에만 standard shader의 Emissive RGB에 적용한다. Base-only 색을 Bloom 값으로 증폭하지 않는다.

Mesh에는 의미가 없는 Sprite Billboard를 거짓으로 연결하지 않는다. After Image의 interval/copy/alpha 세부값은 기존 확장 계약을 유지한다.

`Apply`는 live authored document 반영, `Save`는 Authored JSON 저장이다. Assembly/WFX/Runtime Catalog publish와는 계속 분리해 표시한다.

## 4. 구현 단위

### G01. 레퍼런스형 Mesh Workbench 전면 배치

- 기존 전체 kind radio를 신규 제작 화면에서 제거하고 Mesh 제작 전용임을 표시한다.
- Reset / Create Effect / Refresh Resources를 상단 작업 흐름으로 배치한다.
- Mesh 카드와 Base/Noise/Mask/Emissive/Dissolve slot 카드를 표시한다.
- 선택 slot에 맞춰 WModel 또는 DDS thumbnail grid를 표시한다.
- Effect Name이 없거나 mesh/base 계약이 불완전하면 Create를 비활성화하고 이유를 표시한다. 기존 active document 선택은 요구하지 않는다.

### G02. 원자적 Create Effect

- 별도 draft에서 typed bindings를 생성한다.
- 성공 시 Authored Data File과 Mesh Element 한 개를 원자 저장하고 그 Element를 선택한다.
- preview time을 재시작하고 Effect Detail draft를 연다.
- 이후 Detail 수치 변경은 기존 Apply/Save/Reload 경계로 저장한다.

### G03. 기존 UI 계층 정리

- All Effects의 primary 영역은 active Authored Mesh layer를 먼저 보여 준다.
- 빈 typed Element를 만드는 기존 Add Element는 primary 흐름에서 제거한다.
- 기존 Cascade/Particle hierarchy는 `Imported Cascade Diagnostics`로 분리하여 보존한다.
- selected existing Element의 resource 교체는 advanced edit로 유지하되 unapplied Detail draft guard를 지킨다.

### G04. Shader와 안전 계약

- standard profile의 Noise UV surface warp를 연결한다.
- 신규 Mesh Create에만 mesh+safe Base 필수 규칙을 적용한다.
- 기존 source-material profile의 Base 없는 합법 경로에는 전역 필수 규칙을 적용하지 않는다.
- Warlord domain mapping 누락과 `v8` 성공 문구를 현재 v12 계약으로 교정한다.

### G05. 검증

- Mesh/Base 필수와 optional four slots
- duplicate element ID와 wrong-kind/domain/unsafe Base 거부
- Create 실패 rollback과 성공 single commit
- Effect Detail Apply/Revert, Save/Reload canonical 동일성
- 기본 수치 즉시 preview와 Position/Rotation/Scale/Velocity lifetime Lerp 실행
- Emissive 미연결 시 Bloom Intensity가 Base에 영향을 주지 않음
- stale external writer 보존
- 기존 Q/T Authored와 Particle runtime load 회귀 없음
- standard shader Base/Noise/Mask/Emissive/Dissolve token 검사
- Effect Tool audit, ClientFrontendHarness, Client x64 Debug build
- ProjectAudit와 `git diff --check`

## 5. 완료 기준

- 실제 5개 Effect domain이 선택 가능하다.
- DimensionMaster의 WModel 140개와 DDS 701개가 All/search/folder thumbnail에서 누락 없이 탐색된다.
- 기존 Data Files 선택이 없어도 Effect Name + WModel + safe Base만으로 `Create Effect`가 새 Authored 문서를 원자 저장하고 Data Files 목록에 추가한다.
- Data Files의 `New` 및 Create 실패는 이미 선택한 Mesh/Material builder slot을 초기화하지 않는다.
- builder thumbnail 클릭은 별도 `Use Selected` 의존 없이 현재 slot에 즉시 반영한다.
- WModel과 safe Base를 고르기 전 Create Effect는 실행되지 않는다.
- 성공한 Create는 Mesh layer 하나만 원자적으로 추가하고 즉시 preview와 Effect Detail을 연다.
- 새 수제 carrier Mesh의 시작 Scale은 모델 변환 단위에 맞춰 `(0.01, 0.01, 0.01)`로 생성하며, 기존 Authored/Cascade Element의 저장 Scale은 변경하지 않는다.
- Mask/Dissolve R, Emissive RGB, Noise RG surface warp가 standard profile에서 실행된다.
- 생성한 carrier Mesh는 model dummy material이 아니라 선택한 Base override를 사용한다.
- Apply + Save 후 Reload canonical 문서가 동일하다.
- 기존 Cascade/Particle 문서와 runtime은 삭제·변환되지 않는다.
- 자동 검증과 Debug build가 통과한다.

수동 GPU 완료 판정은 한 carrier Mesh에 대해 `Base only → +Mask → +Noise → +Dissolve → +Emissive` 순서로 캡처하여 slot별 변화와 불투명 card 경계를 확인한 뒤 기록한다.

## 6. 변경 경계

주 변경 파일은 다음으로 제한한다.

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`

v12 schema, codec, playback, renderer와 기존 atomic save 구현은 현재 계약으로 충분하므로 이번 슬라이스에서 불필요하게 다시 작성하지 않는다.

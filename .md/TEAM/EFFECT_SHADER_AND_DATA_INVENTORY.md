# Effect 셰이더·데이터 전수 인벤토리

기준 commit: `d6b27084`

이 문서는 `Client/Bin/ShaderFiles` 42개 파일과 `Data/Effects` 15개 폴더가 각각 무엇을 소유하고
누가 소비하는지를 실측으로 정리한다. 구조가 바뀌면 이 문서를 갱신한다.

## 0. 왜 복잡해 보이는가

한 가지 이유다. **세 세대가 동시에 살아 있다.**

```text
1세대  Engine 공용 렌더링        Effect 와 무관. 캐릭터·맵·디퍼드
2세대  Effect V1 (현재 제품)     carrier 5종 + 공용 profile chain + family
3세대  Effect V2 (신규 계통)     별도 문서/런타임/툴. 아직 병행 중
       + 원본 증거 저장소        DXBC 171 / translated HLSL 169. 런타임 미연결
```

폴더 하나가 커 보이는 것이 아니라, 세 세대의 산출물이 같은 트리에 쌓여 있다.

## 1. 셰이더 42개

### 1.1 Engine 공용 — Effect 아님 (12개)

| 파일 | 역할 |
|---|---|
| `Engine_Shader_Defines.hlsli` | 전 셰이더 공용 defines |
| `Shader_Deferred.hlsl` | 디퍼드 라이팅·합성 |
| `Shader_Cell.hlsl` | 네비게이션 셀 디버그 |
| `Shader_VtxTex.hlsl` | 2D 텍스처 쿼드 |
| `Shader_VtxMeshBinary.hlsl` / `Shader_VtxMeshMapInstance.hlsl` | 정적 메시 / 맵 인스턴싱 |
| `Shader_VtxAnimMeshBinary.hlsl` | 스키닝 캐릭터 |
| `Shader_VtxAnimMesh.hlsl` | **미사용** (C++ 로드 없음) |
| `Shader_VtxMeshPreview.hlsl` | Tool 메시 미리보기 |
| `Shader_VtxEstherNpc.hlsl` | Esther NPC |
| `Shader_VtxSkillGroundTargetPreview.hlsl` | 지면 조준 프리뷰 |

### 1.2 Effect V1 carrier 진입점 — 실제로 픽셀을 만드는 5개

C++ 이 로드하는 `.hlsl` 은 이것뿐이고, **모두 같은 `Shader_EffectCommon.hlsli` 를 include 한다.**

| 진입점 | carrier |
|---|---|
| `Shader_VtxEffectParticle.hlsl` | sprite particle |
| `Shader_VtxEffectMeshPreview.hlsl` | mesh particle |
| `Shader_VtxEffectDecal.hlsl` | local decal |
| `Shader_VtxEffectTrail.hlsl` | trail / ribbon |
| `Shader_VtxEffectRectPreview.hlsl` | standalone sprite rect |

### 1.3 Effect V1 계산식 include — carrier 가 공유하는 8개

| 파일 | 줄 | 소유 내용 |
|---|---:|---|
| `Shader_EffectCommon.hlsli` | 2428 | **profile 0~41 전체 chain.** 여기가 본체다 |
| `Shader_EffectUe3MaterialFamilies.hlsli` | 799 | RuntimeMaterialV2 fixed opcode dispatch |
| `Shader_EffectStandardColorV1.hlsli` | 218 | 명시적 radiance/coverage/dissolve 표준 ABI |
| `Shader_EffectLocalDecalAdapter.hlsli` | 115 | decal projection |
| `Shader_EffectRuntimeMaterialPacket.hlsli` | 43 | class-neutral packet 계약 |
| `Shader_Artist31470RuntimeMaterial.hlsli` | 1179 | 도화가 F 재구성 evaluator |
| `Shader_Artist31470Diagnostic.hlsli` | 741 | 도화가 F 진단 상수 |
| `Shader_Artist31470Active{003,011,022}*.hlsli` | 85~164 | 도화가 F ribbon / outer / decal |

`Shader_EffectCommon.hlsli` 2428줄이 부담스러워 보이지만 구조는 단순하다.
`Shade_EffectParticleUV()` 하나에 `if (N == g_SourceMaterialProfile)` 분기가 42개 붙어 있고,
carrier 5종이 전부 이 함수를 부른다. profile 번호가 곧 family ID다.

### 1.4 family 전용 셰이더 — 자기 파일을 가진 4계열

| 진입점 | 계산식 | 상태 |
|---|---|---|
| `Shader_VtxEffectGlasshole02.hlsl` | `Shader_Ue3Glasshole02.hlsli` | 차원술사 유리, Tool canary |
| `Shader_VtxEffectUe3ValtanGround04.hlsl` | `Shader_Ue3ValtanGround04.hlsli` | 발탄 지면 decal, **Product OFF** |
| `Shader_VtxEffectUe3ValtanDissolve01.hlsl` | `Shader_Ue3ValtanDissolve01.hlsli` | 발탄 masked dissolve, **Product OFF** |
| `Shader_VtxEffectUe3ValtanCrack01.hlsl` | `Shader_Ue3ValtanCrack01.hlsli` | 발탄 균열, **Product OFF** |

발탄 3종은 `static_assert(!VALTAN_TRANSLATED_CANARY_PRODUCT_ENABLED)` 로 제품 진입이 막혀 있다.

### 1.5 authoring 전용 bridge — 2개

| 파일 | 역할 |
|---|---|
| `Shader_EffectExactSpriteBridge.hlsl` | 원본 cooked PS 를 sprite VF 에 붙이는 저작 전용 다리 |
| `Shader_EffectExactLocalMeshBridge.hlsl` | 같은 목적의 mesh 판 |

제품 경로가 아니다. 원본 픽셀 셰이더를 그대로 실행해 비교하기 위한 장치다.

### 1.6 Effect V2 — 별도 계통 4개

`Shader_EffectV2_Common.hlsli` + `Shader_Effect{Rect,Mesh,AnimMesh}V2.hlsl` +
`Shader_VtxAnimMeshPreview_V2.hlsl`. 소비자는 `EffectV2_Object.cpp`, `Effect_Tool_V2.cpp` 다.
V1 chain 과 코드를 공유하지 않는다.

### 1.7 미사용 2개

- `Shader_VtxAnimMesh.hlsl` — `Shader_VtxAnimMeshBinary.hlsl` 이 대체
- `Shader_Ue3Glasshole02Translation.hlsl` — 6줄짜리 translation 표지

## 2. Data/Effects 15 폴더

### 2.1 원본 증거 — 읽기 전용, 정본 아님

| 폴더 | 규모 | 내용 |
|---|---|---|
| `Imported/` | 691 files / 643MB | **최대 폴더.** 클래스별 원본 추출·변환본, source receipt, conversion receipt |
| `CookedShaders/` | 171 `.dxbc` | 원본 컴파일 셰이더 바이트코드. 수식의 oracle |
| `TranslatedShaders/` | 169 `.hlsli` | DXBC → 읽을 수 있는 HLSL 번역본. **아직 런타임 미연결** |
| `Contracts/` | 15 files | family manifest, material registry, cooked variant 색인 |
| `V2/` | Authored·Bindings + 15 계약 | V2 계통의 증거·계약 |

`TranslatedShaders/Shade_Ue3_<material>.hlsli` 169개는 `Client/Bin/ShaderFiles` 어디에서도
include 되지 않는다. 지금은 **데이터**이지 실행 코드가 아니다.

### 2.2 저작 정본 — 사람이 편집하는 곳

| 폴더 | 규모 | 내용 |
|---|---|---|
| `Authored/` | 420 files / 177MB | Effect 문서 정본. `unified` 109 / `authored-baseline` 85 / `restoration-candidate` 11 |
| `AuthoredCorrections/` | 121 files | occurrence 단위 보정과 generated contract |
| `Components/` | 555 files | 클래스별 재사용 component |
| `Assemblies/` | 101 files | 조립 단위 |
| `Policies/`, `ScreenOverlays/` | 3 files | 승인 정책, 화면 overlay 계약 |
| `EffectCatalog.json` | 1 file | **Tool addressable 목록.** 여기 없으면 Tool 에서 못 연다 |

문서 접미사가 계보를 뜻한다.

```text
authored-baseline      이전 세대 저작물. parentMaterialPath 없음
unified                원본 변환 계보. 현재 제품
restoration-candidate  Tool 전용 복원 후보. catalog 미등록
```

`authored-baseline` 과 `unified` 를 before/after 로 읽으면 안 된다. 서로 다른 계보다.

### 2.3 파생물 — 도구가 생성, 직접 편집 금지

| 폴더 | 내용 |
|---|---|
| `VisualPrograms/` | occurrence admission sidecar. GPU program 아님 |
| `MaterialPrograms/` | program/layout/descriptor/adapter/binding registry. 현재 각 1행 |
| `ResourceIndex/` | 리소스 색인 |
| `Baselines/` | legacy cue projection 기준선 |

## 3. 화면을 만드는 최소 경로

복잡해 보여도 실제로 픽셀이 나오는 길은 하나다.

```text
animevents  effectref=asset cue
  -> EffectCatalog.json                      Tool addressable
  -> Publish-Effects.ps1                     Product membership 결정
  -> Client/Bin/DataFiles/Effect/            runtime catalog + 문서
  -> Effect_DocumentRenderer.cpp             carrier 선택 + profile 해석
  -> Shader_VtxEffect*.hlsl                  carrier 5종 중 하나
  -> Shader_EffectCommon.hlsli               profile 0~41 중 하나
  -> RT0 픽셀
```

나머지 폴더와 셰이더는 이 경로의 **증거**, **저작 입력**, **미연결 연구**, 또는 **다른 세대**다.

## 4. 지금 상태 요약

| 항목 | 수 |
|---|---:|
| 셰이더 파일 | 42 |
| 그중 Effect 관련 | 25 |
| 실제 carrier 진입점 | 5 |
| `Shader_EffectCommon.hlsli` 의 typed profile | 42 (0~41) |
| classifier 가 아는 (profileId, parent) 쌍 | 41 |
| 확보한 DXBC | 171 |
| 번역된 family HLSL (미연결) | 169 |
| Product Effect 문서 | 207 |

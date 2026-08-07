# 2026-08-07 차원술사 Renderer Family 우선 복원 구현 계획서

## 1. 목표

차원술사 Effect를 하나의 `Particle` 목록으로 취급하지 않는다. 원본 실행 의미를 다음 세
Renderer Family로 분리하고, 통째로 복원 가능한 모델을 Material carrier보다 먼저 닫는다.

```text
Skeletal Model Cue
  원본 SkeletalMesh + AnimSet + Material Instance + clip을 WModel로 조리해 재생

Static Mesh Effect Instance
  원본 Static Mesh 형상에 Cascade의 시작 시각·Transform·회전·크기·수명과
  Material Mask/Noise/Dissolve/Fresnel을 적용

Sprite / Procedural Effect Layer
  Texture/SubUV 또는 부모 Material 계산식으로 실루엣을 생성
```

`Particle`은 Cascade가 Mesh/Sprite instance의 생성과 수명 곡선을 계산하는 내부 실행 방식으로만
남긴다. Tool과 복원 판단에서는 Mesh와 Sprite를 분리한다.

## 2. 현재 실측

현재 기본 11스킬의 실행 문서는 644 Elements다.

| Renderer Family | 수량 |
|---|---:|
| Static Mesh Effect Instance | 163 |
| Sprite / Procedural Effect Layer | 418 |
| Light | 23 |
| Screen Post | 38 |
| Decal | 2 |
| Skeletal Model Cue | 1 |

기본 11스킬에서 원본 `PlaySkeletalMesh`와 검증된 runtime binding이 모두 존재하는 것은
T `2050500`의 `Dimension Summon` 한 건이다. ALT_V `2050540`에도 별도 source cue가 있으나
compound payload가 아직 해석되지 않았고 이번 기본 11스킬 범위에는 포함하지 않는다.

현재 canonical Static Mesh 163개를 원본 `bOverrideMaterial`과 대조한 결과는 정상 override
carrier 126개, 모델 내장 Material 후보 2개, 서로 모순된 계약 35개다. 모순 35개는 원본이
`bOverrideMaterial=true`인데 Authored가 `useModelMaterial=true`인 상태다.

교정한 staging은 override carrier 159개, 모델 내장 Material 후보 4개, 모순 0개다. 수정 필드는
총 37개다. 35개는 잘못 켠 model material을 끄고, D/T의 2개는 원본 class default에 따라 model
material을 켠다. 네 embedded 후보는 모두 `fm_d_crack_037`과 Engine default Material 계열이므로
완성형 모델로 자동 승격하지 않는다.

## 3. A 2050210 검격 구조

A는 검격 Mesh 네 개가 하나의 skeletal clip으로 변형되는 구조가 아니다.

```text
swinghit의 Static Mesh base layer 11개
  × source event 4회: 0.25 / 0.60 / 0.90 / 1.30초
  = 44 Mesh instances

독립 Mesh layer 4개
  = 총 48 Mesh instances
```

네 event는 위치를 바꾸고, 각 Mesh instance는 `ParticleModuleMeshRotation`,
`ParticleModuleMeshRotationRate`, `ParticleModuleMeshRotationRateMultiplyLife`의 원본 곡선을
소비한다. Vertex가 bone animation으로 변형되는 Summon과 달리, A는 정적 검격 Mesh의
instance Transform과 Material 시간이 변한다.

A의 대표 검격 한 겹은 다음 입력을 조립한다.

```text
fm_h_swing_02.wmodel geometry/UV
+ Base 또는 색 carrier
+ Mask
+ Noise/Flow
+ Dissolve
+ Dynamic Parameter와 life curve
+ Additive/Translucent, Depth, Cull 상태
= 최종 검격 layer
```

Normal/ORM은 모든 Effect Mesh에 공통으로 필수인 입력이 아니다. 물리 표면 조명을 받는 완성형
모델에는 중요하지만, 자체발광 검격 carrier는 Mask/Flow/Dissolve가 최종 외곽을 더 크게 결정한다.

## G00. Renderer Family 전수 감사

### 수정 파일

- `Tools/LevelPlacementExtractor/audit_dimensionmaster_renderer_families.py`
- `Tools/LevelPlacementExtractor/test_audit_dimensionmaster_renderer_families.py`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/materialize_dimensionmaster_base_effects.py`
- `Data/Effects/Imported/DimensionMaster/DimensionMaster.renderer-family-audit.json`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

감사기는 PlayerSkills와 Animation binding이 함께 인정한 기본 11스킬만 읽는다. Authored 문서,
Action Cue recipe, Model Cue runtime binding과 Resources asset 존재를 교차 검증한다.

출력 분류는 다음과 같다.

```text
BOUND_SKELETAL_MODEL_CUE
UNBOUND_SOURCE_MODEL_CUE
STATIC_MESH_MATERIAL_OVERRIDE_CARRIER
STATIC_MESH_EMBEDDED_MATERIAL_CANDIDATE
STATIC_MESH_CONTRACT_CONTRADICTION
SPRITE_MATERIAL_CARRIER
SPRITE_PROCEDURAL_PROFILE
SPRITE_UNRESOLVED_MATERIAL
```

`STATIC_MESH_EMBEDDED_MATERIAL_CANDIDATE`는 완성형 판정이 아니다. 원본
`bOverrideMaterial`, Authored `useModelMaterial`, source Material 증거가 모두 일치한 뒤에만
후속 복원 후보가 된다.

### 종료 증거

- 기본 11스킬 join 성공
- 581 Cascade layer가 Mesh 163 / Sprite 418로 정확히 분할
- 현재 canonical의 Mesh 계약 모순 35개를 명시적으로 격리
- 교정 staging의 Mesh 163이 override carrier 159 / embedded candidate 4 / 모순 0으로 분할
- T Summon source cue와 runtime WModel binding 1건 일치
- A Mesh 48, base group 15, 네 번 반복되는 group 11개 확인
- 모든 기록된 WModel asset 존재
- 잘못된 Mesh shape/binding, cue identity, 중복 ID는 fail-closed

## G01. 완성형 Model Cue 우선 복원

G00에서 `BOUND_SKELETAL_MODEL_CUE`로 확인된 자산만 기존
`CModel -> CMaterial -> animation clip` 경로로 실행한다. T Summon의 남은 작업은
2.901422초 `PlaySkeletalMeshMaterialParam` 의미를 해석하고 네 Material section에 적용하는 것이다.

ALT_V는 기본 11스킬 밖에서 별도 체크포인트로 둔다. compound payload를 완전히 해석하기 전에
T binding을 복사하거나 이름으로 추측하지 않는다.

## G02. Static Mesh carrier 복원

Mesh geometry/UV, 시작 시간, position, rotation, rotation-rate, scale/life는 원본 Cascade 값을
보존한다. 그 위에 부모 Material family별 finite shader profile을 연결한다.

우선순위는 화면 면적과 반복 재사용 수로 정한다.

```text
Q: Blackline Aura -> Local Crack
A: swing/linear-flow/blackline/local-crack family
T: hemisphere/ring/cylinder carrier family
E: box/hole/master family
```

전역 Scale/Pivot은 Material alpha가 정상화되기 전까지 변경하지 않는다.

## G03. Sprite/Procedural 복원

Sprite는 texture-backed와 textureless procedural을 분리한다. 부모 그래프 근거가 없는 procedural은
흰 카드로 그리지 않고 숨김과 diagnostic을 유지한다. Q Glow/Impact처럼 UModel direct dump로
scalar와 render state가 확인된 family만 finite profile로 승격한다.

## 4. 검증 순서

```text
1. Renderer Family 감사 unit tests
2. 실제 기본 11스킬 audit 생성과 JSON parse
3. Client project/filter XML parse
4. 관련 Effect pipeline과 ClientFrontendHarness
5. Client x64 Debug build
6. Effect Tool에서 Family별 tree/solo 확인
7. 고정 Active ID + Sample Time + Element ID로 PNG A/B
8. ProjectAudit와 git diff --check
```

자동 검증은 Renderer identity와 데이터 보존을 판정한다. 원작 픽셀 일치는 사용자의 GPU 캡처 전에는
PASS로 기록하지 않는다.

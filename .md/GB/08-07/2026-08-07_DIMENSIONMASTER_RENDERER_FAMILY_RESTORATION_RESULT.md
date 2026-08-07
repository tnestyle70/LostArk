# 2026-08-07 차원술사 Renderer Family 우선 복원 결과서

## 1. 결론

차원술사 복원 단위를 하나의 `Particle` 목록으로 취급하지 않고 실제 렌더러 의미로
분리했다.

```text
Skeletal Model Cue
Static Mesh Effect Instance
Sprite / Procedural Effect Layer
Light / Screen Post / Decal
```

Dimension Summon은 Cascade Material 자동 복원의 성공 사례가 아니다. 원본
`SkeletalMesh + AnimSet + Material Instance`를 캐릭터 모델 변환 경로로 만든 완성형
WModel을 재생하는 사례다.

A `2050210`은 이와 다르다. 정적 Effect Mesh 여러 개를 Cascade가 서로 다른 시각,
위치, 회전, 크기와 수명으로 생성하고 emitter Material/MI가 최종 검격 실루엣을 만든다.

## 2. Renderer Family 전수 조사

기본 11스킬의 현재 정본은 다음과 같다.

| Family | 수량 |
|---|---:|
| Static Mesh Effect Instance | 163 |
| Sprite / Procedural Effect Layer | 418 |
| Light | 23 |
| Screen Post | 38 |
| Decal | 2 |
| bound Skeletal Model Cue | 1 |

기본 11스킬에서 완성형 Skeletal Model Cue로 runtime까지 연결된 것은 T `2050500`
Dimension Summon 1건이다. 다른 Static Mesh 163개를 Summon과 같은 완성형 animated
model로 간주하지 않는다.

정본 감사 파일:

- `Data/Effects/Imported/DimensionMaster/DimensionMaster.renderer-family-audit.json`

## 3. 실제로 발견하고 교정한 원인

원본 UE3 계약은 다음과 같다.

```text
ParticleModuleTypeDataMesh.bOverrideMaterial = true
  -> Cascade emitter Material/MI가 Mesh material을 대체
  -> runtime useModelMaterial = false

bOverrideMaterial = false 또는 class default
  -> Mesh 내장 material 사용
  -> runtime useModelMaterial = true
```

기존 생성기는 이 원본 값을 읽지 않고 Base texture slot 존재 여부로
`useModelMaterial`을 결정했다. 그 결과 현재 canonical Static Mesh 중 35개가 원본
계약과 반대로 기록되어 있었다.

대표 WModel을 실제 조사한 결과 다음 자산은 material section이
`dummy_material_0`이고 Base/Normal/Emissive/ORM texture가 없다.

- `fm_d_crack_032.wmodel`
- `fm_j_cube_01.wmodel`
- `fm_h_swing_02.wmodel`

따라서 이 자산에 `useModelMaterial=true`를 적용하면 원본 검격 Material 대신 dummy
면 전체가 그려져 흰색·보라색 판이 노출될 수밖에 없다.

교정 수량은 다음과 같다.

| 스킬 | 교정 |
|---|---:|
| W `2050120` | 11 |
| E `2050160` | 1 |
| A `2050210` | 20 |
| D `2050240` | 2 |
| T `2050500` | 1 |
| 합계 | 35 |

Q `2050100`의 2개는 병행 Q 3-emitter 작업을 덮지 않기 위해 이번 승격에서 제외했다.
감사 파일에 `STATIC_MESH_CONTRACT_CONTRADICTION=2`로 명시되어 있으며 Q 작업의 다음
materialization에서 같은 공통 규칙으로 교정된다.

교정 뒤 현재 정본 분포는 다음과 같다.

```text
Static Mesh 163
├─ emitter Material override carrier 157
├─ embedded material candidate          4
└─ Q 병행 작업 보류 contradiction       2
```

embedded 후보 4개는 모두 `fm_d_crack_037` 및 Engine default material 계열이다.
Summon처럼 완성된 model material이라는 증거가 없으므로 자동 승격하지 않는다.

## 4. A 2050210과 Summon의 차이

A는 117 Elements이며 다음으로 구성된다.

```text
Static Mesh 48
Sprite      52
Light        4
Screen Post 13
```

Static Mesh 48개는 8개 WModel을 사용한다. 11개 기본 Mesh layer가 다음 네 시각에
위치까지 바꾸어 반복되어 44개가 되고, 일회성 장식 4개가 더해진다.

```text
0.25s / 0.60s / 0.90s / 1.30s
```

각 instance는 `ParticleModuleMeshRotation`, `MeshRotationRate`,
`MeshRotationRateMultiplyLife`의 원본 곡선을 가진다. 이 값은 정적 Mesh를 언제 어느
방향으로 보일지 결정하는 instance simulation이므로 삭제하지 않는다. 다만 Tool과
복원 문서에서는 이를 `Static Mesh Effect Instance`로 표시한다.

A의 검격은 대체로 다음 결합으로 만들어진다.

```text
Mesh geometry/UV
+ emitter Material/MI
+ Base 또는 색 carrier
+ Mask/Opacity
+ Noise/Flow
+ Dissolve
+ Dynamic Parameter와 life curve
+ Emissive/Color
+ Blend/Depth/Cull/Two-sided
```

Diffuse/Normal/Emissive/ORM 네 texture가 모든 검격에 고정적으로 필요한 것은 아니다.
Normal/ORM은 표면 조명 모델에서는 중요하지만, 자체발광 검격 carrier의 외곽은 주로
Mask/Flow/Dissolve와 Blend가 결정한다. Parent Material 증거가 없는 Normal을 Noise로
추측 승격하지 않는다.

## 5. 구현

- 원본 `bOverrideMaterial`을 직접 소비하는 importer 계약 추가
- 기존 Authored를 덮어 재생성하지 않고 해당 Boolean만 교정하는 원자적 repair 추가
- 다른 writer가 source를 변경하면 stale hash로 승격을 거부
- Boolean 외 필드가 바뀌면 교정을 거부
- 기본 11스킬을 Renderer Family로 분류하는 감사기 추가
- A의 반복 Static Mesh group과 source Mesh rotation 곡선 보존 감사
- Renderer Family 감사 JSON을 Client `96.DataFiles`에 등록

## 6. 검증

| 검증 | 결과 |
|---|---|
| repair Python tests | PASS, 2 tests |
| renderer family audit tests | PASS, 3 tests |
| imported document tests | PASS, 13 tests |
| base materialization tests | PASS, 9 tests |
| 비-Q targeted repair | PASS, 5 documents / 35 corrections |
| 기본 11스킬 renderer audit | PASS, 581 layers / Mesh 163 / Sprite 418 |
| staging Assembly/WFX compile | PASS, 16 Effects / 182 Components / 947 Emitters |
| staging compile identity verify | PASS, source Action Cues 526 |
| staging Publisher | PASS, 16 Effects / 182 Components |
| Client project/filter XML + audit JSON parse | PASS |
| `git diff --check` | PASS |

staging Publisher는 별도 `.codex-staging` DataRoot와 Runtime Catalog에 게시했다. Q 병행
작업을 보존하기 위해 canonical Assembly/WFX/Runtime Catalog는 이번 슬라이스에서
교체하지 않았다.

## 7. 아직 완료로 말하지 않는 항목

- Q의 남은 Mesh material contradiction 2개
- A/W/E/D/T canonical Assembly/WFX/Runtime Catalog 재게시
- A 검격의 Parent Material finite shader profile
- T `PlaySkeletalMeshMaterialParam`의 section별 실행
- Tool의 사용자 표시를 Particle 중심에서 Renderer Family 중심으로 재구성
- 고정 sample GPU 캡처에서 흰색·보라색 full-card 제거 확인
- 원작 PNG와 색, 밝기, Mask, Dissolve, Scale의 최종 A/B

이번 변경은 픽셀 복원 완료가 아니다. 다만 Static Mesh를 실제 원본과 반대 material
경로로 렌더하던 구조적 원인을 제거했고, 나머지 Material profile 복원을 올바른
carrier 위에서 시작할 수 있게 했다.

## 8. 다음 순서

1. Q 3-emitter 슬라이스가 끝나면 Q 2개도 같은 계약으로 교정한다.
2. 16 Assembly / 182 WFX / Runtime Catalog를 한 번만 통합 재게시한다.
3. A의 가장 큰 swing Mesh group 하나를 Solo한다.
4. `useModelMaterial=false`와 emitter Material resource가 Runtime Sample에서 일치하는지 확인한다.
5. Mask/Opacity만 먼저 연결해 full-card 0을 확인한다.
6. Noise/Flow, Dissolve, Emissive 순서로 Parent family profile을 닫는다.
7. 한 Parent family가 A의 네 반복 시각에 같이 개선되는지 확인한 뒤 다른 Mesh family로 확장한다.

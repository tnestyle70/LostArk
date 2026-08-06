# 차원술사 T 원본 Effect Source 1차 추출 결과

작성일: 2026-08-06  
대상: `DIMENSIONMASTER`, input slot `T`, skill ID `2050510`  
상태: 원본 시간축과 14개 ParticleSystem graph 추출 완료 / 재귀 Distribution·Curve·Burst·Dynamic Parameter 파싱 완료 / 직접 Material·Mesh·Texture 연결 완료

> 2026-08-06 정정: Animation Tool 가장 아래에서 확인한 원작 동일 Mesh 결합
> 애니메이션은 T body가 아니라 별도
> `DimensionMaster_DimensionSummon.wmodel`이며, 현재 스킬 정본상 F `2050500`
> 포탈 복원의 golden case다. T `2050510`은 Character body의
> `dimensionthrust_01/02`와 별도 Effect graph를 가진다. 상세 파싱 및 현재 Effect
> Detail 대응은 `2026-08-06_EFFECT_CASCADE_PARAMETER_PARSING_RESULT.md`를 정본으로
> 본다.

## 1. 결론

차원술사 T의 Character body 애니메이션은 `CModel`이 소유한다. Effect 문서는 body
Mesh를 다시 만들지 않고 그 위에 겹치는 Particle, Texture/Sprite, Trail,
Distortion, Post Effect만 소유한다. 단, ParticleSystem 내부의
`ParticleModuleTypeDataMesh`는 독립 Effect emitter이므로 추출 대상이다.

현재 수동 문서
`Data/Effects/Authored/effect.dimensionmaster.skill.2050510.effect.json`의 단일
`thought_trail`은 원본 T 전체 구성이 아니다. 원본은 두 Animation clip의 시간축에
여러 ParticleSystem을 겹쳐 재생한다.

## 2. 원본 시간축

제품 binding은 다음 두 clip을 순서대로 사용한다.

```text
0.0000 ~ 0.6000  pc_sp_m_00_sk_sk_dimensionthrust_01
0.6000 ~ 3.0000  pc_sp_m_00_sk_sk_dimensionthrust_02
```

원본 `DimensionMaster.animnotify`에서 Effect 관련 이벤트 25건을 복구했다.

- `PlayParticleEffect`: 18건, graph root 18/18 해결
- 빈 `TrailGhostEffect`: 3건, serialized asset 정보가 없어 미지원으로 보존
- 빈 `Effect`: 1건, serialized asset 정보가 없어 미지원으로 보존
- `ViewShake`: 3건, Effect Document 밖의 presentation event로 분리

핵심 전역 시간은 다음과 같다.

```text
0.0000  Light + SuperAwake
0.0100  DimensionThrust_00_01
0.1000  RGBNoise
0.2007  Hand_00_01
0.3011  Hand_00_02
0.6000  Weapon_01_01
1.5000  RGBNoise + TrailGhost 3건
1.6000  Impact_01_01 + 빈 Effect
1.6022  Exp 3종 + Distortion + ZoomBlur + Light + RGBNoise_Zaxis
1.7024  Light
```

## 3. 해결된 원본 ParticleSystem 14종

```text
FX_PC_SWP_00
  Par_J_SWP_DimensionThrust_00_01
  Par_J_SWP_DimensionThrust_Hand_00_01
  Par_J_SWP_DimensionThrust_Hand_00_02
  Par_J_SWP_DimensionThrust_Weapon_01_01
  Par_J_SWP_DimensionThrust_Impact_01_01
  Par_J_SWP_DimensionThrust_Exp_01_01
  Par_J_SWP_DimensionThrust_Exp_01_02
  Par_J_SWP_DimensionThrust_Exp_01_03

FX_CM_02
  Light.Par_MP_Light_01

FX_PC_WGL_07
  Par_S_SuperAwake_ExMove_01

FX_POST
  FX_Par.Par_J_RGBNoise_01
  FX_Par.Par_J_ZoomBlur_01
  FX_Par.Par_J_RGBNoise_Zaxis_01

FX_CM_01
  Distortion_oneLayer.Par_MP_ConcaveDisOL_Bill_01
```

공용 package 네 개를 추가 복호화한 결과는 다음과 같다.

| Package | Graph object | ParticleSystem | Property error | Missing emitter target |
|---|---:|---:|---:|---:|
| `FX_CM_01` | 25,035 | 417 | 0 | 0 |
| `FX_CM_02` | 7,014 | 194 | 0 | 0 |
| `FX_POST` | 659 | 61 | 0 | 0 |
| `FX_PC_WGL_07` | 2,138 | 36 | 0 | 0 |

T에 실제로 연결된 14개 시스템만 닫아 만든 normalized graph의 실측은 다음과
같다.

| 항목 | 수치 |
|---|---:|
| unique graph node | 688 |
| edge occurrence | 1,131 |
| explicit Material/Mesh/Texture binding occurrence | 123 |
| 아직 불러오지 않은 외부 graph reference occurrence | 1,384 |
| Unsupported/Unresolved unique row | 425 |

외부 reference는 이름으로 기존 Kind에 억지 배치하지 않았다. 다음 단계에서 해당
package의 실제 export class를 열어 graph module을 추가 해결한다. 직접 Material
Instance parameter는 다음 절에서 별도로 해결했다.

## 4. Material과 런타임 리소스 연결

원본 graph가 직접 참조한 고유 Material은 78개다.

- 77개: 정확한 Material/MaterialInstance, parent, texture/scalar/vector parameter 해결
- 1개: `EngineMaterials.DefaultParticle`; 게임 Effect material package가 아닌 Engine
  fallback이어서 현재 Effect material map 밖에 보존
- texture parameter occurrence 148건
- 고유 source texture 93개
- scalar parameter occurrence 863건
- vector parameter occurrence 61건

직접 리소스는 다음처럼 현재 Resources와 연결됐다.

| 종류 | 원본 직접 의존 | Resources 해결 |
|---|---:|---:|
| Effect Mesh | 18 | 18 |
| Effect Texture | 93 | 93 |
| 합계 | 111 | 111 |

기존 Resources에서 빠져 있던 정확 object 두 개를 원본 package에서 단일 추출해
보충했다.

```text
FX_SM_00.fm_d_helix_031
  -> Effect/DimensionMaster/Meshes/fm_d_helix_031.wmodel

FX_TEX_00.fx_a_trail_004
  -> Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_004.dds
```

`fm_d_helix_031`은 같은 pipeline으로 만들어진 기존 `fm_d_helix_018`을 probe해
`--pretransform --no-auto-textures --scale 100` 결과가 기존 WModel과 byte hash까지
일치하는 것을 확인한 뒤 같은 계약으로 조리했다. 이는 runtime pack publish/hash
정책을 되살린 것이 아니라 변환 옵션을 실측한 추출 증거다.

이 Mesh를 참조하는 정확한 source node는
`FX_PC_WGL_07:export:1581 / ParticleModuleTypeDataMesh`이며
`bOverrideMaterial=true`다. 따라서 WModel의 `dummy_material_0`을 최종 Effect
재질로 쓰는 것이 아니라 emitter의 Required Material binding을 사용한다.

## 5. 산출물

Git 관리 대상 compact source receipt:

```text
Data/Effects/Imported/DimensionMaster/skill.2050510.source-receipt.json
```

이 파일에는 Animation 소유권, clip 시간축, 원본 이벤트, 14개 system 요약,
직접 resource binding, package hash, Unsupported/Unresolved 목록이 들어 있다.

대용량 상세 normalized graph는 Resource_LostArk 보고서에 둔다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/DIMENSIONMASTER/
  skill_2050510/skill.2050510.normalized-effect-graph.json
```

공용 package raw graph도 같은 `skill_2050510/particle_graphs/` 아래에 있다.
리소스 payload와 대용량 원본 graph는 Git에 넣지 않는다.

재현 도구:

```text
Tools/LevelPlacementExtractor/build_skill_effect_source_receipt.py
Tools/LevelPlacementExtractor/test_skill_effect_source_receipt.py
```

## 6. 검증

```text
Python unit test                         PASS (5/5)
Python py_compile                        PASS
Source ParticleSystem event resolution  PASS (18/18)
Unique source system resolution          PASS (14/14)
Material parameter resolution            PARTIAL (77/78, Engine fallback 1)
Runtime direct resource resolution       PASS (111/111)
Combined Mesh duplication guard          PASS
  owner = Animation/CModel
  reconstructCombinedMeshAsEffectElement = false
```

## 7. 아직 완료가 아닌 부분

- 428개 외부 graph reference의 실제 package/class materialize
- `EngineMaterials.DefaultParticle`의 Engine fallback 처리 결정
- 원본 `TrailGhostEffect`의 asset 없는 notify payload 추가 근거
- 원본 node를 우리 Mesh/Particle/Decal/Trail/Sprite Element로 변환하는 명시적 표
- `Imported Effect Document` 초안 생성
- 툴 Preview와 실제 skill runtime 재생 검증

이 항목이 닫히기 전에는 차원술사 T Effect 복원이 완료됐다고 기록하지 않는다.

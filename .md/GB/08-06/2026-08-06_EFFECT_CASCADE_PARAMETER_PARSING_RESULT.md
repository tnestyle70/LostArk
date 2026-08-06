# Cascade Effect parameter 파싱 결과

작성일: 2026-08-06  
대상: 차원술사, 창술사, 도화가, 워로드, 발탄 원본 Particle graph  
Golden case: 차원술사 F `2050500` / `업의 경계`

## 1. 결론

Cascade의 Particle graph는 단순히 Mesh와 Texture 이름만 얻는 수준보다 더 깊게
복구할 수 있다. 현재 파서는 다음 데이터를 실제 UE3 tagged property에서 읽는다.

- ParticleSystem -> Emitter -> LOD -> Module 연결
- RawDistributionFloat/Vector의 Distribution 참조와 baked LookupTable
- InterpCurveFloat/Vector의 key time, value, tangent, interpolation mode
- Spawn Burst의 count, countLow, time
- Dynamic Material Parameter의 이름, 평가 방식, 시간별 LookupTable
- Material/Material Instance의 Texture, Scalar, Vector parameter
- TypeDataMesh, TypeDataDecal, Light, Post Effect 같은 원본 module class

다만 이 결과를 현재 `EFFECT_DETAIL_DESC`에 그대로 넣는다고 원작이 자동 완성되지는
않는다. 원본은 다중 key curve, 여러 시각의 Burst, emitter별 loop, 임의 Material
parameter와 Mesh particle을 사용한다. 현재 runtime은 주로 상수, min/max, 선형
Start/End와 표준 Texture slot을 사용한다.

따라서 파싱 결과는 다음처럼 사용한다.

```text
원본 Cascade 수치
  -> EXACT: 현재 필드와 의미가 같아 그대로 초기화
  -> APPROXIMATION: 현재 단순 필드로 축약한 뒤 Effect Detail에서 A/B 튜닝
  -> UNSUPPORTED: 억지 변환하지 않고 원본 node와 이유 보존
```

## 2. 전체 파서 실측

동일 물리 package는 한 번만 계산해 63개 고유 package를 검증했고, 네 class와 발탄
보고서의 99개 graph 파일을 갱신했다.

| 항목 | 결과 |
|---|---:|
| 고유 물리/logical package | 63 |
| 갱신한 상세 graph | 99 |
| Particle graph object | 440,766 |
| 복구한 RawDistribution | 400,608 |
| 복구한 InterpCurve | 1,548 |
| 복구한 curve key | 4,820 |
| 지원 구조의 opaque fallback | 0 |
| tagged property parse error | 0 |
| ParticleSystem의 missing emitter target | 0 |

RawDistribution 수가 이전보다 증가한 이유는 `DynamicParams` 배열 내부의
`ParamValue`까지 재귀적으로 읽기 때문이다. `LODDistances`의 정수 bit pattern을
object reference로 잘못 보던 항목도 제거했다.

## 3. 차원술사 F 기준 실제 결과

### 3.1 Animation과 Effect의 경계

Animation Tool 가장 아래의 다음 모델이 포탈 Mesh 애니메이션 golden case다.

```text
Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel
  Mesh 4개
  Animation 2개
    sk_swp_dms_00_sk_sk_dimensionprison
    sk_swp_dms_00_sk_sk_dimensionprison_1
```

현재 스킬 정본은 `F / 2050500 / 업의 경계`이고 Character body binding은
`pc_sp_m_00_sk_sk_dimensionprison`이다. Summon 모델은 아직 실제 F runtime에서
생성되지 않는다. 최종 복원은 F pivot에서 이 CModel animation을 재생하고 아래
Effect overlay를 같은 시간축에 겹치는 방식이다.

### 3.2 Source graph 규모

| 항목 | 결과 |
|---|---:|
| Effect notify | 15 |
| 해결된 PlayParticleEffect notify | 13 |
| asset 없는 미지원 notify | 2 |
| 관련 ViewShake | 3 |
| 고유 ParticleSystem | 12 |
| Emitter | 88 |
| 고유 graph node | 626 |
| edge occurrence | 972 |
| 파싱한 Dynamic Parameter entry | 140 |
| 파싱한 Burst entry | 25 |
| 현재 Resources에 연결된 Mesh/Texture | 151 / 151 |
| Material parameter set 해결 | 78 / 81 |

Material 미해결 세 건 중 두 건은 `EngineMaterials.DefaultMaterial`과
`DefaultParticle` fallback이다. 나머지 한 건
`fx_k_pa_makeflow_01_01_tr`은 같은 logical path의 서로 다른 물리 package 후보가
두 개라 자동 선택하지 않았다. 이 한 건은 정확 source package 근거 또는 원작 화면
비교가 필요하다.

### 3.3 실제로 읽힌 값의 예

`Par_W_SWP_DimensionPrison_00_Exp`에서 다음 항목이 읽힌다.

- Particle lifetime baked 범위: `0.6 ~ 1.2`초
- Burst: `15 @ 0.0s`, `15 @ 0.3s`, `20 @ 0.5s` 등을 emitter별로 보존
- Start velocity와 acceleration의 축별 baked 범위
- Start size와 Size Over Life LookupTable
- `ring_str`, `uvoffset`, `dissolve`, `coloralphaoffset`, `distortoffset`,
  `distortstrong`, `alphapower` 같은 Dynamic Parameter
- `ParticleModuleTypeDataMesh`와 `EFParticleModuleTypeDataDecal`

`Par_W_SWP_DimensionPrison_00`에서는 다음 Material 동적값도 읽힌다.

- `maintex_tile_pan`: 22개 sample
- `dissolve`: 22~23개 sample
- `uv_noisevelue`
- `uv_sphery_uv_noisepan`
- `alphadissolve[0-1]`, `pan[0-2]`, `edgestr[0-x]`, `disrotion[0-x]`

원본 Curve node도 key 단위로 복구된다. 예를 들어 Alpha curve 중 하나는 정규화
시간 `0.0 -> 0.2 -> 0.6 -> 1.0`에서 값 `0 -> 1 -> 1 -> 0`을 가진다. 현재의
한 번뿐인 Color Multiply Start/End Lerp로는 이 fade-in, hold, fade-out을 동시에
표현할 수 없다.

## 4. 원본, 파싱 결과, 현재 Effect Detail 비교

| 의미 | 원본에서 파싱되는 것 | 현재 Effect Detail | 판정 |
|---|---|---|---|
| Effect 시작 | Animation notify의 global time | `startDelaySeconds` | system 시작은 `EXACT` |
| Particle 수명 | constant/uniform/curve RawDistribution | `lifeTimeSeconds[min,max]` | constant/uniform은 `EXACT`, curve는 `APPROXIMATION` |
| Spawn Rate | 시간 함수 Rate | `spawnRatePerSecond` 한 값 | 상수만 `EXACT` |
| Spawn Burst | 여러 `(count, time)` | 시작 시 `burstCount` 한 번 | time 0 한 건만 `EXACT`, 나머지는 Element 분할 또는 확장 |
| 초기 속도 | 축별 constant/uniform distribution | `initialVelocityMin/Max` | 단순 범위는 `EXACT` |
| 시간별 속도 | Velocity Over Life curve | 초기속도 + constant acceleration | `UNSUPPORTED` 또는 근사 |
| 가속도 | constant/uniform/curve | constant `acceleration` | 상수만 `EXACT` |
| 시작 크기 | 3축 distribution | Particle 2D `startSize` | Sprite 단순 XY는 `EXACT`, Mesh/Z는 근사 |
| Size Over Life | 다중 key/LookupTable | `startSize -> endSize` 선형 | `APPROXIMATION` |
| Color/Alpha | start range와 다중 curve | Color Multiply Start/End | 단순 2-key만 `EXACT`, 대부분 근사 |
| UV pan/tile | Material scalar와 Dynamic Parameter curve | UV Start/Speed/Tile | 단순 pan은 `EXACT`, 임의 parameter는 Template 필요 |
| Dissolve | Material 고유 parameter와 curve | Dissolve start 한 값 | `APPROXIMATION` |
| Emissive/Distortion | Material parameter 이름과 값 | 공통 scalar 한 개씩 | 표준 이름만 근사 연결, Template별 binding 필요 |
| Mesh particle | TypeDataMesh가 particle마다 Mesh 생성 | Particle은 Rect instance | `UNSUPPORTED` |
| Decal emitter | TypeDataDecal의 emitter/projection 의미 | 단일 Decal Element | 다중 방출은 `UNSUPPORTED`, 정적 한 장만 근사 |
| Light/Post | Light TypeData, zoom/rgb/film noise | 대응 Effect Element 없음 | `UNSUPPORTED` |
| Material 입력 | 임의 개수와 의미의 parameter | Base/Noise/Mask/Emissive/Dissolve | 표준 Template 밖은 `UNSUPPORTED` 또는 새 Template |

중요한 점은 `dissolve=1.58` 같은 원본 Dynamic Parameter 값을 현재
`dissolveStartNormalized`에 그대로 넣으면 안 된다는 것이다. 전자는 원본 Material
수식의 입력값이고 후자는 우리 Shader의 정규화된 시작 시각이다. 이름이 비슷해도
같은 의미인지 Shader binding을 확인한 뒤 변환해야 한다.

## 5. F 복원 작업 순서

1. Model View에서 `Dimension Summon` CModel과 두 animation을 재생한다.
2. F body animation 및 skill pivot과 Summon 생성 시각을 맞춘다.
3. 12개 Source ParticleSystem을 system/group 단위로 가져오고 88 Emitter를 하나의
   Particle Element로 뭉개지 않는다.
4. `EXACT`인 notify time, 단순 lifetime/range, resource binding을 먼저 고정한다.
5. 여러 Burst는 시작 시각이 다른 Element로 나누거나 Burst timeline을 추가한다.
6. Color/Size/UV/Dissolve curve는 먼저 Start/End 근사를 만들고 원작과 같은 시간의
   화면을 A/B 비교해 Effect Detail에서 조정한다.
7. Mesh particle, emitted Decal, Light/Post는 `UNSUPPORTED`로 표시한 채 필요한
   runtime 확장을 별도 결정한다.
8. 검수된 결과만 `Data/Effects/Authored/effect.dimensionmaster.skill.2050500.effect.json`
   에 저장한다.

현재 F Authored 문서의 `karma_ring`, `karma_particles` 두 Element는 수동으로 만든
placeholder에 가깝다. 이번 파싱 결과로 자동 덮어쓰지 않는다. 원본 12 system과
현재 두 Element를 나란히 보며 단계적으로 교체해야 한다.

## 6. 검증

```text
test_effect_extraction_tools.py          PASS (9/9)
test_skill_effect_source_receipt.py      PASS (6/6)
LevelPlacementExtractor test discovery   PASS (61/61)
Python py_compile                        PASS
63 package recursive parse              PASS
99 detailed graph stage/validate/commit PASS
43 class skill receipt rebuild           PASS
43 skill source system unresolved        0
43 skill runtime Mesh/Texture unresolved 0
F false LODDistance object references    0
ProjectAudit                              PASS (69 checks)
Effect Tool final bundle audit            PASS
git diff --check                          PASS (line-ending warnings only)
```

상세 F graph:

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/
  DIMENSIONMASTER/all_bound_skills/skills/skill_2050500/
  skill.2050500.normalized-effect-graph.json
```

Git 관리 compact receipt:

```text
Data/Effects/Imported/DimensionMaster/skill.2050500.source-receipt.json
```

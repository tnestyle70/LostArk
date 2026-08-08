# 2026-08-08 차원술사 A 2050210 4검격 복원 구현 계획서

## 1. 결론

2026-08-08 후속 실측에 따라 이 계획의 정본 방향을 수동 조립에서 원본 실행 복구로 전환한다.
차원술사 A `2050210`의 정본은 16개 수동 candidate가 아니라 117개 원본 Element와 각
Element의 `SourceRecipe`, 원본 Material/MI identity, 원본 animation event time이다.

기존에 만든 다음 세 개의 Mesh layer는 삭제하지 않지만 시각 비교용 candidate로만 보존한다.

```text
Body  = 검격의 넓은 백금·보라 면과 Alpha/Mask/Dissolve
Core  = 중심의 강한 백색·연보라 발광
Rim   = 외곽의 짙은 보라색 경계
```

이 candidate의 Body/Core/Rim 수치, 색, 선형 Revolution은 원본값으로 승격하지 않는다.
원본 4타를 먼저 그대로 실행하고, 원본 데이터가 실제로 끊긴 Material 계산식만 부모 family
단위의 finite runtime profile로 복구한다. PNG는 원본 데이터의 대체물이 아니라 결과 검증과
미복구 계산식 튜닝 기준으로만 사용한다.

이번 작업의 완료 기준은 두 단계로 분리한다.

1. 원본 exact gate: Mesh/Sprite, 타이밍, Transform, lifetime, LUT, Dynamic Parameter,
   SubUV, anchor snapshot, texture parameter identity가 원본과 동일하게 실행된다.
2. Material gate: cooked UPK에서 회수 가능한 입력과 그래프 topology를 모두 실행한다.
   cooker가 제거한 수식은 `RECONSTRUCTED_PROFILE`로 유지하며, 증거 없이
   `RUNTIME_EXACT`로 승격하지 않는다.

## 2. 정본과 작업 경계

### 2.1 정본 ID

| 항목 | 정본 |
|---|---|
| Character Class | `DimensionMaster` |
| Input Slot | `A` |
| Skill ID | `2050210` |
| Effect Asset ID | `effect.dimensionmaster.skill.2050210` |
| 원작 PNG | `C:/Users/user/Desktop/로스트아크이펙트이미지/차원술사/DimensionMaster_A00.png` ~ `A04.png` |
| 복원 캡처 | `C:/Users/user/Desktop/로스트아크이펙트이미지/차원술사_복원/08-08/A/` |

`PlayerSkills.json`, Animation/Anchor, 입력 키 계약은 변경하지 않는다. 이번 슬라이스는
presentation Effect만 수정한다.

### 2.2 현재 실행 문서 실측

```text
전체 Elements        117
Particle             100
Mesh-backed           48
Screen Post           13
Light                  4
Material RUNTIME_EXACT 0
```

세부 반복 구조는 다음과 같이 확인됐다.

```text
SwingHit 1회        Mesh 11 + Sprite 8 = 19 layers
SwingHit 4회        19 × 4 = 76 layers
첫 타격 SwingDeco   별도 7 layers와 추가 occurrence
Screen Post         RGB Noise 9 + Zoom Blur 4
마지막 꼬리          1.8535초 Ribbon/Cube 2 layers
```

`Particle 100`은 GPU에 점만 뿌린다는 뜻이 아니다. Cascade emitter가 Mesh, Sprite,
Material, 시간 모듈을 담고 있는 원본 컨테이너라는 뜻이다. 실제 화면의 큰 형상은
`fm_h_swing_*`, `fm_m_trail_*`, `fm_b_ring_*`, `fm_a_broken_*` Mesh가 결정한다.

### 2.3 이번에 보존하는 것

- 원본 Cascade/SourceRecipe와 Imported provenance
- 네 타격의 시작 시각과 파싱된 상대 위치
- 원본 WModel과 확인된 Texture slot asset ID
- 기존 Light, Screen Post, Sprite, Model Cue 문서
- Authored atomic Save/Reload, stale writer 차단, publish 분리 계약

### 2.4 이번에 추측하지 않는 것

- UE3 Parent Material 그래프 전체를 복원했다고 주장하지 않는다.
- 원본의 `diff_str=100`, `distortion=10` 같은 부모 그래프 전용 단위를 현재 standard
  shader 값으로 그대로 복사하지 않는다.
- Imported 최상위 scale `1.1`을 WModel 수동 layer scale로 그대로 복사하지 않는다.
- 원본 PNG에 sample time/camera manifest가 없으므로 최초부터 pixel exact라고 판정하지 않는다.
- 첫 체크포인트에서 Light/Post/전역 scale로 차이를 가리지 않는다.

## 3. 원작 A의 시각 분해

| PNG | 원작에서 읽히는 큰 형상 | 복원 판정 포인트 |
|---|---|---|
| A00 | 얕고 긴 수평 백색 원호, 아래·외곽의 보라 경계 | 첫 검격의 폭, 방향, 흰 코어 연속성 |
| A01 | 더 선명한 초승달, 백금색 중심과 짙은 보라 외곽 | Body와 Rim의 분리 |
| A02 | 원호가 세워지고 우측 파편·백색 충격이 추가 | 3타 방향 변화와 파편 시점 |
| A03 | 세로에 가까운 C형 백색·보라 링 | 4타 직전의 원형 진행 |
| A04 | 큰 원형에 가까운 검격, 강한 백색 코어와 보라 윤곽 | 네 타격 합성, 마지막 Ring/Impact |

따라서 A를 하나의 큰 흰 Mesh로 만들면 실패한다. 동일하거나 유사한 carrier Mesh를
겹치되 Alpha body, additive core, dark violet rim의 역할을 분리해야 한다.

## 4. 파싱으로 확정된 수치와 자산

### 4.1 네 타격의 시간·위치

아래 값은 튜닝 제안이 아니라 원본 실행 문서에서 반복 확인된 값이다.

| Hit | Start Delay | Parent-relative Position |
|---:|---:|---|
| 1 | `0.25 s` | `(0.5, -0.5, -0.9)` |
| 2 | `0.60 s` | `(0.5, -0.5, 0.8)` |
| 3 | `0.90 s` | `(0.5, 0.3, -0.9)` |
| 4 | `1.30 s` | `(0.5, 0.6, -0.8)` |

이 시각은 Swing, Trail, Crack, Ring 계열에서 반복된다. Position은 현재 Player Root
pivot 기준으로 먼저 검증한다. Animation/Anchor 담당의 socket 계약은 수정하지 않는다.

### 4.2 주 검격 Body/Flow의 원본 슬롯 증거

| 역할 | Resources-relative asset ID |
|---|---|
| Body Mesh | `Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel` |
| Flow Mesh | `Effect/DimensionMaster/Meshes/fm_m_trail_002.wmodel` |
| Base | `Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_mirnoise_02.dds` |
| Mask | `Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_auraline_19_ycl.dds` |
| Noise | `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds` |
| Dissolve | `Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds` |
| Source Material | `fx_j_me_linearflow_02_12_tr` / `fx_j_me_linearflow_02_05_tr` |
| Source Render | `alpha_one_sided_depth_read` |
| Emitter Window | `0.10 s` |
| Body Particle Lifetime | `0.50 s` |
| Flow Particle Lifetime | `0.30 s` |
| Source UV Speed | `(0.0, 0.1)` |

`Emitter Window 0.10 s`는 생성 가능한 방출 구간이며 Mesh가 화면에 보이는 최종 수명이
아니다. 수동 MESH의 `Life Time`에는 Particle Lifetime을 옮겨야 한다.

부모 Material 증거의 `mask_density=1.2`, `mask_radius=1.8`, `opacity_str=1.2`,
`opacity_pow=0.6`, `diff_str=5`, noise strength `0.3/0.4`, dissolve hardness `5`는
백색 중심, 어두운 외곽, 중간 정도의 noise, 단단한 dissolve가 있었다는 근거로만 사용한다.
현재 standard shader와 단위가 다르므로 숫자를 직접 복사하지 않는다.

### 4.3 보라 외곽 Rim의 원본 슬롯 증거

| 역할 | Resources-relative asset ID |
|---|---|
| Mesh | `Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel` |
| Base | `Effect/DimensionMaster/Textures/FX_TEX_05/fx_l_environment_001.dds` |
| Mask | `Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_line_01_xcl.dds` |
| Dissolve | `Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds` |
| Source family | blackline aura |
| Source Render | `alpha_one_sided_depth_read` |

다른 분석에서 제안된 `fx_j_normal_02.dds`의 Rim Noise 사용은 현재 canonical A의
ResourceBindings에서 확인되지 않았다. 부모 Material의 별도 증거가 확보되기 전에는 Rim
Noise로 자동 연결하지 않는다.

### 4.4 마지막 Ring과 보조 형상

| 역할 | Resources-relative asset ID | 사용 시점 |
|---|---|---|
| Ring Mesh | `Effect/DimensionMaster/Meshes/fm_b_ring_001.wmodel` | Hit 4 합격 후 |
| Ring Base | `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_atypical_007.dds` | Hit 4 합격 후 |
| Ring Mask/Emissive | `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_fire_003.dds` | Hit 4 합격 후 |
| Crack Mesh | `Effect/DimensionMaster/Meshes/fm_a_broken_012.wmodel` | Material 근거 확보 후 |
| Fragment Sprite Base | `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_fragment_007.dds` | 기존 Imported layer 보존 |

`fm_a_broken_012`는 Dissolve만 확정되고 standard Surface mode에서 안전한 Base가 아직
확정되지 않았다. 흰 카드로 만들지 말고 첫 검격 완료 뒤 source profile을 보존하거나
추가 증거를 확인한 뒤 사용한다.

## 5. 수동 Material 조립식

이 절의 숫자는 파싱된 정답이 아니라 첫 GPU A/B를 시작하기 위한 `TUNING_SEED`다.
캡처 결과에 따라 좁혀야 한다.

### 5.1 `manual.a.hit01.body`

```text
Mesh              fm_h_swing_02
Base              fx_j_mirnoise_02
Noise             fx_d_noise_014
Mask              fx_j_auraline_19_ycl
Dissolve          fx_h_atypical_01_1
Emissive          Empty
Render Profile    Alpha / One-Sided / Depth Read
Scale Start       (0.0341, 0.0341, 0.0341)
Position          (0.5, -0.5, -0.9)
Start Delay       0.25
UV Speed          (0.0, 0.1)
Color Multiply    (1.60, 1.30, 2.40, 0.80)
Color Clip        0.0
Distortion        0.04 tuning seed; shape 판정 시 0.0
Velocity          (0, 0, 0)
```

형상과 방향을 보기 쉽도록 첫 audition에서는 Lifetime을 `0.60~1.00 s`로 늘린다.
실루엣이 확정되면 `0.10~0.18 s` 범위로 되돌리고 원작 템포를 비교한다.

### 5.2 `manual.a.hit01.highlight` 백색 Core

Body와 같은 Mesh/Position/Rotation/Scale/Delay를 사용하는 additive duplicate다.

```text
Base              fx_j_mirnoise_02
Mask              fx_j_auraline_19_ycl
Noise             fx_d_noise_014
Dissolve          fx_h_atypical_01_1
Emissive trial A  fx_j_auraline_19_ycl
Render Profile    Additive / One-Sided / Depth Read
Color Multiply    (1.20, 1.10, 1.40, 0.25~0.40)
Emissive Intensity 1.0 → 1.4 → 1.8 비교
```

`fx_j_auraline_19_ycl`을 Emissive에도 넣는 것은 원본 exact graph가 아니라 연속된 백색
core를 얻기 위한 수동 근사다. 끊어진 선처럼 보이면 trial B로
`fx_j_mirnoise_02`를 Emissive에 넣어 비교하고, 둘 다 카드 외곽을 만들면 Emissive slot을
비우고 additive Base만으로 다시 판정한다.

### 5.3 `manual.a.hit01.rim`

```text
Mesh              fm_h_swing_02
Base              fx_l_environment_001
Mask              fx_j_line_01_xcl
Dissolve          fx_h_atypical_01_1
Noise/Emissive    Empty
Render Profile    Alpha / One-Sided / Depth Read
Color Multiply    (0.45, 0.25, 0.90, 0.45~0.65)
```

Rim은 검격 전체를 덮는 보라 면이 아니라 외곽선으로만 읽혀야 한다. 너무 밝으면 Bloom을
올리지 말고 alpha를 먼저 낮춘다.

### 5.4 `manual.a.hit01.flow` 선택 항목

```text
Mesh              fm_m_trail_002
Base/Noise/Mask/Dissolve는 Body와 동일
Render Profile    Alpha / One-Sided / Depth Read
Lifetime          0.30 s
UV Speed          (0.0, 0.1)
```

Body/Core/Rim만으로 원작 실루엣이 읽히지 않을 때만 추가한다. 처음부터 네 layer를 전부
켜면 어느 layer가 카드나 과노출을 만드는지 알 수 없다.

### 5.5 Distortion, Dissolve, 회전·속도 적용 원칙

- standard shader의 surface UV warp는 `Distortion Intensity × 0.01`이다.
- 따라서 `0.59`는 이미 강한 편이다. `0.00 → 0.02 → 0.04 → 0.06`만 비교하고
  가장 작은 합격값을 사용한다.
- Noise를 바인딩하더라도 Distortion을 0으로 둘 수 있다. Noise R은 Dissolve 보조에도 쓰인다.
- `Dissolve Start`는 Lifetime 정규화 비율이다. `0.55 → 0.65 → 0.75`를 비교한다.
- Rotation은 시작 방향의 degree이고, Revolution은 degree/sec 회전 속도다.
- Velocity는 parent-local 방향과 초당 이동량을 함께 나타낸다. A의 주 검격은 먼저 0으로 둔다.
- 원본 회전 모듈에서 약 `-18°` 시작과 약 `-360°/s` 회전 단서가 있으나 축·순서 변환이
  정본으로 확정되지 않았다. `Revolution=0`에서 Mesh 방향을 먼저 맞춘 뒤 audition seed로만 쓴다.

## 6. G별 구현 순서

### G00. 기준선과 안전 장치

#### 목표

수동 작업이 기존 canonical A와 다른 세션 결과를 덮지 않도록 독립 candidate에서 시작한다.

#### 작업

1. canonical A Authored 문서의 SHA-256과 canonical serialization을 기록한다.
2. candidate ID를 다음으로 고정한다.

```text
effect.dimensionmaster.skill.2050210.a-restoration-candidate
```

3. candidate는 current v12 Effect Document를 그대로 사용한다.
4. 현재 A를 고정 카메라로 먼저 캡처한다.
5. Screen Post OFF, Light 변경 없음, Player Root pivot으로 시작한다.

#### 기준선 캡처

```text
0.25 / 0.30 / 0.35
0.60 / 0.65 / 0.70
0.90 / 0.95 / 1.00
1.30 / 1.35 / 1.40 seconds
```

저장 위치는 `08-08/A/00_baseline`이다. 캡처에는 Effect ID, document SHA, sample time,
camera transform/FOV/resolution, pivot, post 상태를 함께 기록한다.

### G01. Hit 1 Body 실루엣 고정

#### 목표

Material 색보다 먼저 Mesh 방향·스케일·피벗과 Mask가 만든 외곽을 확정한다.

#### 작업

1. Mesh Workbench에서 candidate Effect Name과 `manual.a.hit01.body` Layer Name을 입력한다.
2. 5.1의 Mesh/Base/Noise/Mask/Dissolve를 선택하고 Create Effect한다.
3. 수동 Mesh 기본 `0.01`은 안전값으로만 보존한다. A Body는 원본
   `Particle start size 0.031 × 상위 scale 1.1`을 환산한 `0.0341`에서 시작한다.
4. Distortion/Bloom은 0, Color Clip은 0, Velocity/Revolution은 0으로 둔다.
5. Lifetime을 일시적으로 1초로 늘려 Rotation X/Y/Z를 한 축씩만 바꿔 본다.
6. 원작 A00의 얕은 수평 원호와 맞는 방향을 찾은 뒤 값을 기록한다.

#### 통과 기준

- 불투명 사각 카드가 보이지 않는다.
- 원호의 폭과 곡률이 A00과 같은 계열로 식별된다.
- 캐릭터 pivot에서 갑자기 멀리 날아가지 않는다.
- 방향과 scale을 동시에 바꾸지 않아 어느 값이 원인인지 추적 가능하다.

### G02. Hit 1 Core와 Rim 완성

#### 목표

원작의 백색 중심과 보라 외곽을 각각 독립 조절 가능하게 만든다.

#### 작업

1. `manual.a.hit01.highlight`를 추가하고 Body와 동일 Transform/Timing을 적용한다.
2. Emissive trial A/B와 intensity `1.0/1.4/1.8`을 비교한다.
3. `manual.a.hit01.rim`을 추가하고 alpha만 먼저 조절한다.
4. 각 단계는 Body only, Body+Core, Body+Core+Rim 순으로 캡처한다.
5. 필요할 때만 Flow를 추가한다.

#### 통과 기준

- 중심은 흰색·연보라로 읽히고 전체가 녹색이나 회색으로 물들지 않는다.
- 보라색은 외곽에 남고 검격 전체를 불투명하게 덮지 않는다.
- white core를 위해 full-white Mesh나 blankwhite fallback을 사용하지 않는다.
- additive 중첩으로 내부가 완전히 하얗게 소실되지 않는다.

### G03. Noise와 Dissolve 확정

#### 목표

정지된 페인트가 아니라 흐르고 사라지는 검격으로 만든다.

#### 작업

1. Distortion `0/.02/.04/.06`을 동일 frame에서 캡처한다.
2. 가장 작은 합격값을 Body에 적용하고 Core/Rim에는 필요할 때만 별도로 적용한다.
3. Dissolve Start `0.55/.65/.75`를 비교한다.
4. emitter window `0.10 s`와 Mesh particle lifetime을 혼동하지 않는다. 최종값은
   Body `0.50 s`, Rim/Flow `0.30 s`, Highlight `0.18~0.22 s`에서 시작한다.
5. pop, 역방향 dissolve, 카드 경계가 생기면 바로 직전 합격 상태로 롤백한다.

### G04. 4타로 확장

#### 목표

완성된 Hit 1 조립식을 네 타격으로 확장하되 원본 시간·위치를 보존한다.

#### layer ID

```text
manual.a.hit01.body/highlight/rim[/flow/afterimage]
manual.a.hit02.body/highlight/rim[/flow/afterimage]
manual.a.hit03.body/highlight/rim[/flow/afterimage]
manual.a.hit04.body/highlight/rim[/flow/afterimage]
```

#### 작업

1. 각 hit는 4.1의 Start Delay와 Position을 사용한다.
2. Material 값은 Hit 1 합격값에서 시작한다.
3. Rotation만 Hit 2 → Hit 3 → Hit 4 순으로 Solo하여 원작 원호 진행 방향에 맞춘다.
4. 네 타격을 동시에 보면서 alpha와 lifetime만 줄여 과노출을 제거한다.
5. Hit 4가 합격한 뒤에만 `manual.a.hit04.ring`을 추가한다.

회전값은 PNG만 보고 한 번에 하드코딩하지 않는다. 각 hit의 Solo GPU 결과로 확정하여
`parsed`가 아니라 `project-tuned`로 기록한다.

### G05. 기존 Cascade 보조 layer와 결합

#### 목표

수동 Mesh가 큰 형상을 책임지고, 기존 Cascade 데이터가 파편·연기·충격·조명을 보조한다.

#### 작업 순서

1. Ring
2. 기존 Crack source profile
3. Fragment/Impact Sprite
4. Smoke
5. Light 4개
6. Screen Post 13개

각 그룹을 한 번에 하나씩 ON/OFF한다. 현재 수동 Mesh Workbench에 Sprite 생성 기능이 없으므로
Fragment/Smoke를 Mesh로 위조하지 않고 기존 Imported layer를 보존한다.

### G06. Candidate의 결정적 canonical 병합

#### 목표

Effect Tool preview에서 승인된 layer만 canonical A에 재현 가능하게 병합한다.

#### 계획 파일

| 파일 | 역할 |
|---|---|
| `Tools/EffectPipeline/promote_dimensionmaster_a_candidate.py` | candidate 검증, staged merge, SHA guard, atomic promote/rollback |
| `Tools/EffectPipeline/test_promote_dimensionmaster_a_candidate.py` | 정상·중복·잘못된 ID·stale canonical·중간 실패 rollback |
| `Data/Effects/Imported/DimensionMaster/DimensionMaster.2050210-a-manual-promotion.receipt.json` | 승인된 candidate/canonical SHA와 교체 layer 기록 |

#### 병합 계약

- candidate layer ID는 `manual.a.` prefix만 허용한다.
- canonical의 Light/Post/Model Cue/Action Cue/SourceRecipe를 보존한다.
- 기존 source layer를 숨길 때는 명시적인 replacement ID 목록만 허용한다.
- canonical SHA가 G00 기준과 달라졌으면 자동 중단한다.
- staged 문서를 parse/validate/preview한 뒤에만 atomic 교체한다.
- 실패 시 canonical과 현재 runtime catalog를 모두 유지한다.

현재 Effect Tool의 `Promote to Authored Skill`은 Imported 문서 전용이므로 수동 candidate에
억지로 재사용하지 않는다.

### G07. Assembly/WFX/Runtime publish

#### 목표

Effect Tool 저장 성공과 실제 A 키 런타임 적용을 분리하여 검증한다.

#### 순서

1. 승인된 canonical A를 isolated staging root에 배치한다.
2. `build_effect_components.py`로 Assembly/WFX candidate를 생성한다.
3. `Publish-Effects.ps1 -Mode Validate`로 staging catalog를 검증한다.
4. 다른 세션의 Effect 파일 소유권과 SHA가 변하지 않았는지 재확인한다.
5. 승인된 A 산출물과 runtime catalog를 atomic publish한다.
6. Client를 완전히 종료한 뒤 Debug Client를 다시 시작한다.
7. DimensionMaster A 키로 `2050210`이 재생되는지 확인한다.

현재 builder가 전체 effect catalog를 재컴파일하므로 dirty shared tree에서 즉시 publish하지 않는다.
staging과 소유권 checkpoint 없이 전체 182 WFX를 덮는 작업은 금지한다.

### G08. 최종 A/B와 성능 검증

#### 캡처 폴더

```text
08-08/A/00_baseline
08-08/A/01_hit01_shape
08-08/A/02_hit01_material
08-08/A/03_four_hit_sequence
08-08/A/04_runtime
```

최종 승인 전까지는 설명형 파일명을 사용한다.

```text
DimensionMaster_A_HIT01_shape_t0.300.png
DimensionMaster_A_HIT01_core-rim_t0.300.png
DimensionMaster_A_SEQUENCE_t1.350_post-off.png
DimensionMaster_A_RUNTIME_t1.350_post-on.png
```

승인된 최종 캡처만 `DimensionMaster_A00~A04` 대응표와 함께 기록한다.

## 7. 사용자의 인게임 검증 절차

### 7.1 첫 검격 검증

1. Client를 `Client/Default` 작업 디렉터리에서 실행한다.
2. DimensionMaster와 Effect Tool을 연다.
3. candidate Data File을 Load한다. 첫 생성 전에는 Effect Name에 candidate ID를 입력한다.
4. `manual.a.hit01.body`를 Solo하고 Lifetime을 1초로 둔다.
5. Rotation을 한 축씩 조정하여 A00의 수평 원호와 맞춘다.
6. Body scale은 source 환산값 `0.0341`에서 시작하고 `0.031/0.0341/0.037`을 비교한다.
7. Body 합격 캡처를 보낸다.
8. Core와 Rim을 순서대로 추가하고 같은 sample/camera로 다시 캡처한다.

### 7.2 캡처를 통해 판정할 수 있는 정보

| 캡처 | 판정 가능한 것 | 아직 판정하면 안 되는 것 |
|---|---|---|
| Body only | Mesh 선택, 회전축, scale, Mask 외곽 | Bloom, 최종 색, Post |
| Body+Core | 백색 중심 연속성, additive 과노출 | 최종 파편 밀도 |
| Body+Core+Rim | 보라 외곽 폭과 alpha | 4타 전체 타이밍 |
| Distortion 4종 | Noise warp 강도와 카드 경계 | 원작 parent graph exact 여부 |
| 4타 Solo | 각 타격 방향·pivot | 합성 과노출 |
| 4타 전체 | 시작 시각, overlap, 원형 진행 | Light/Post 정합성 |
| Post OFF/ON | Material 문제와 화면 후처리 문제 분리 | Camera가 다른 PNG pixel 오차 |

### 7.3 반드시 같이 남길 메타데이터

```text
Active Effect ID
Document SHA
Selected layer ID
Sample Time
Camera Transform / FOV / Resolution
Character Class / Pivot
Screen Post ON/OFF
Render Profile
Mesh/Base/Noise/Mask/Emissive/Dissolve asset IDs
Color/Emissive/Distortion/Dissolve/Transform values
```

이 정보가 있어야 다음 수정이 “더 밝게” 같은 감상이 아니라 Core alpha, Rim alpha,
Distortion, Rotation 중 정확히 어느 축의 문제인지 좁혀진다.

## 8. 완료 기준

### 8.1 시각 기준

- 네 검격이 `0.25/0.60/0.90/1.30 s` 순서로 식별된다.
- A00의 얕은 수평 원호에서 A04의 원형에 가까운 검격으로 진행한다.
- 중심은 백색·연보라, 외곽은 짙은 보라로 분리된다.
- 불투명 사각 카드, 보라 상자, normal/data texture Base 출력이 0이다.
- Distortion은 형상을 보조하고 화면 전체 noise를 만들지 않는다.
- Dissolve가 lifetime 안에서 작동하며 순간 pop으로 끝나지 않는다.
- 마지막 Ring/Fragment/Smoke가 주 검격 실루엣을 가리지 않는다.

### 8.2 데이터·런타임 기준

- canonical A 117 Elements와 SourceRecipe 보존 PASS
- source Material named texture/scalar/vector/switch 보존 PASS
- source event `0.250/0.600/0.900/1.300초`와 snapshot anchor PASS
- candidate Save → Reload canonical 동일성은 회귀 검증으로만 PASS
- canonical A의 unrelated source data 보존 PASS
- Assembly/WFX stage PASS
- Effect Publisher validate/publish PASS
- Client x64 Debug build PASS
- Effect Tool/ClientFrontendHarness PASS
- A 키 인게임 재생과 고정 sample 캡처 PASS
- `git diff --check` PASS
- ProjectAudit 결과 기록; unrelated shared-tree 실패는 별도 분리

### 8.3 완료라고 부르지 않을 것

- Hit 1 Body만 보이는 상태
- Effect Tool preview만 성공하고 runtime publish하지 않은 상태
- four-hit timing은 맞지만 흰 카드가 남은 상태
- Post를 켜야만 Material 결함이 가려지는 상태
- PNG 카메라 조건이 다른데 pixel exact라고 주장하는 상태

## 9. 실패 시 전환 기준

다음 세 번의 명확한 checkpoint를 거쳐도 첫 검격이 원호로 식별되지 않으면 원인을 분리한다.

1. `fm_h_swing_02 + Base + Mask`에서도 외곽이 틀림: Mesh 방향/모델 변환 문제
2. Base+Mask는 맞지만 Core/Rim이 실패: standard shader profile 또는 texture role 문제
3. Hit 1은 맞지만 4타가 틀림: per-hit Rotation/Pivot/Timing 문제

1번이면 다른 Swing Mesh `fm_h_swing_01/05`를 같은 slot 조합으로 A/B한다. 2번이면 A 전용
finite Material profile을 구현한다. 3번이면 Material을 더 만들지 않고 타격별 Transform만
교정한다. 이 세 분기 전에는 117 Elements 수작업 재제작으로 전환하지 않는다.

## 10. 이번 계획의 첫 실전 체크포인트

이번 작업에서 가장 먼저 닫을 결과는 다음 한 장이다.

```text
effect.dimensionmaster.skill.2050210
원본 SwingHit 1의 Mesh 11 + Sprite 8
Sample Time 0.30 s
Screen Post OFF
원본 source event/transform/LUT/runtime sample 표시
미지원 Material layer는 fail-closed
```

이 한 장에서 Geometry/Timing/Anchor와 Material을 분리한다. 전자가 맞고 후자만 틀리면
Body/Core/Rim을 다시 손으로 조립하지 않고 해당 parent Material profile만 교정한다. 첫 타격의
parent family가 합격하면 동일 identity를 쓰는 Hit 2~4와 다른 스킬에 같은 실행식을 확대한다.

## 11. 애니메이션 보정 속도와 Effect 시간축 계약

A의 네 검격은 애니메이션 길이를 기계적으로 같은 4등분으로 나누지 않는다. 원본
`DimensionMaster.animevents`의 `pc_sp_m_00_sk_sk_willowrend`에는 주 검격 Effect가
`0.250 / 0.600 / 0.900 / 1.300초`에 시작한다고 기록돼 있고 candidate의 네 Group도 이
source time을 보존한다.

Effect Tool과 제품 런타임은 다음 계약을 사용한다.

```text
animation source time = current track ticks / ticksPerSecond
wall time              = animation source time / authored playRate
effect sample time      = animation source time
```

따라서 Animation 담당자가 `playRate`를 올리면 같은 `0.250초` source event가 실제 화면에서는
더 빨리 도착하고, `playRate`를 낮추면 더 늦게 도착한다. Effect 문서의 네 Group start delay를
그때마다 파괴적으로 다시 저장하지 않는다. Tool의 Play/Pause/Restart/Loop/Sample Time은
바인딩된 Animation과 함께 움직이고, Sample Time은 wall-clock 초가 아니라 source animation
timeline 초로 표시한다.

제품의 `CCharacter::Update_EffectCues`도 현재 animation track을 `ticksPerSecond`로 나눈
source millisecond가 cue를 통과했을 때 Effect를 생성한다. Effect Tool은 별도 시간 체계를
만들지 않고 이 제품 계약을 미러링한다. 여러 clip으로 구성된 스킬은 앞 clip의 source duration
또는 `playMs`를 누적하고, 현재 clip source time을 더해 하나의 연속 Effect timeline으로 만든다.

위치 관계는 다음과 같다.

- Character와 Effect는 같은 GameObject가 아니라 같은 world의 별도 객체다.
- A의 원본 주 검격은 `root + follow=false`다. 각 source event가 도착한 프레임의 Character
  root transform을 snapshot하고 이후에는 그 world transform을 유지한다.
- 각 Element의 Position/Rotation/Scale은 이 root 아래의 local transform이다.
- Weapon Socket/Model Bone을 선택한 layer만 해당 bone transform을 다시 resolve한다.
- 투사체나 `SNAPSHOT` layer는 생성 시 transform을 고정하고 이후 Character를 따라가지 않는다.

수동 검증에서는 Timeline `0.250 / 0.600 / 0.900 / 1.300초`를 차례로 scrub하여 각 Group이
동일한 animation pose에서 시작하는지 확인하고, 이어서 Play로 실제 보정 속도에서 네 검격이
각 event 프레임의 root 위치를 snapshot한 뒤 캐릭터를 잘못 따라오지 않는지 확인한다.

## 12. 2026-08-08 원본 Material 직접 파싱 실측과 구현 순서

원본 package `ZHJ4TC4PCK4PR4J22HIXEYUXBU.upk`를 프로젝트의 UE3 tagged-property parser로
직접 열어 A 대표 부모를 조사했다.

| Parent | Expressions | 살아 있는 expression | 원본 named texture 입력 |
|---|---:|---:|---|
| `fx_j_pa_linearflow_02_tr` | 344 | parameter/switch 105 | `diff_tex`, `a_mask_tex`, `b_mask_tex`, `a_noise_01_tex`, `b_noise_01_tex`, `diff_noise_tex`, `dissolve_tex` |
| `fx_j_pa_blacklineaura_01_tr` | 235 | parameter/switch 105 | `mask_a_tex`, `mask_b_tex`, `diffuse_tex`, `flow_tex`, `dissolve_tex` |

현재 UModel props에는 parameter/switch 객체와 값은 남지만 대부분의 Add/Multiply/Lerp/Panner
노드와 최종 Emissive/Opacity 연결이 cooker에 의해 null 처리돼 있다. 반면 원본 UPK의 각
`MaterialExpressionTextureObjectParameter` export에는 parameter name, group, default texture가
남아 있다. 따라서 기존 5-slot 축소는 원본 입력을 유실한 계약이다.

구현은 다음 순서로 닫는다.

1. 원본 UPK에서 parent output reference, expression 배열의 null/non-null, 살아 있는 parameter와
   static-switch edge, named texture/scalar/vector default를 직접 추출한다.
2. child MI override를 이름 기준으로 합성하되 parent default와 group provenance를 보존한다.
3. 실행 문서에 모든 named source texture 입력을 손실 없이 보존한다. 수동 Mesh의
   Base/Noise/Mask/Emissive/Dissolve 5-slot 계약은 변경하지 않는다.
4. A의 `linearflow_02`와 `blacklineaura_01` 전용 runtime profile이 명시적으로 요구하는 named
   texture와 scalar/vector만 stage한다. 누락 입력은 흰색 fallback이 아니라 fail-closed다.
5. geometry/timing/anchor gate를 먼저 통과한 뒤 Material profile을 Solo A/B한다.
6. cooked package에서 사라진 연산 topology가 남아 있는 shader cache를 추가 확보하거나 검증된
   수식을 만들기 전까지 semantic status는 `RECONSTRUCTED_PROFILE`을 유지한다.

이 경계 때문에 현재 `RUNTIME_EXACT=0`을 단순히 수치 튜닝으로 1로 바꾸지 않는다. 먼저 원본
입력 손실 0과 source execution 동일성을 닫고, 그 다음 실제 계산식 증거가 확보된 parent만
`RUNTIME_EXACT`로 승격한다.

## 13. 2026-08-08 구현 체크포인트

이번 슬라이스에서 다음 항목을 구현했다.

- canonical A 117 Elements와 SourceRecipe를 유지한 채 `fx_j_pa_linearflow_02_tr`의 원본 named
  texture 입력을 문서와 런타임까지 보존했다.
- `diff_tex`, `diff_noise_tex`, `a_mask_tex`, `a_noise_01_tex`, `b_mask_tex`,
  `b_noise_01_tex`, `dissolve_tex`가 하나라도 빠지면 흰색 fallback 없이 stage를 거부한다.
- Publisher와 C++ runtime dependency 수집기가 named source texture를 동일하게 계산한다.
- A의 원본 source event `0.25/0.60/0.90/1.30초`를 유지하고, Animation binding의 `playRate`를
  Effect playback rate에도 전달해 보정 속도 변경 시 wall-clock drift가 생기지 않게 했다.
- 16 Assembly / 182 Components / 947 Emitters를 재컴파일하고 Runtime Catalog를 재게시했다.

자동 검증은 Client Debug build와 ClientFrontendHarness `failures 0`, Effect Tool final audit,
Effect pipeline, Python 34 tests까지 통과했다. 전체 ProjectAudit에는 공유 작업트리의
`projects.data-source-visibility: expected=550 project=547 filters=547` 한 건이 남았다.

아직 완료하지 않은 항목은 수동 GPU A/B다. 또한 parent graph는 344 expression entry 중
110개만 non-null이고 unresolved input edge가 45개이므로 Material 의미는 계속
`RECONSTRUCTED_PROFILE`이며 `RUNTIME_EXACT=0`이다. 이 상태를 원작 픽셀 완료로 부르지 않는다.

## 14. canonical A 검증 성능 게이트

정본 117 Elements를 검증할 수 없는 6 FPS 상태를 Material A/B보다 먼저 닫는다.

1. Data Files Load는 리소스 stage만 수행하며 명시적 Play 전에는 preview를 숨기고 평가하지 않는다.
2. Follow root와 preview time을 한 번의 playback update로 반영해 같은 프레임을 두 번 재구축하지 않는다.
3. Renderer는 authored Element 순서를 유지하되 Element마다 전체 evaluated 목록을 다시 찾지 않고
   정렬된 contiguous range를 선형 순회한다.
4. `Mesh Emitters` 범위를 별도로 제공해 48개 mesh-backed Cascade layer의 형상·타이밍을
   52개 Sprite와 Screen Post 13개 없이 먼저 검증한다.
5. `Solo Element`는 선택 emitter 하나만 stage하며 Complete, Mesh, Sprite, Solo 결과를 서로
   완료로 혼동하지 않는다.

수동 완료 기준은 Load 직후 idle FPS 회복, Mesh Emitters 네 타격 재생, Solo emitter의
상호작용 가능한 프레임 유지다. Complete Effect의 최종 FPS는 실제 GPU 측정 전에는 PASS로 기록하지 않는다.

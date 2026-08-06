# 차원술사 2050240 A 경계 돌파 복원 구현 계획

## 1. 목표

차원술사 기본 상태의 `A / 2050240 / 경계 돌파`를 원본 Animation Notify, Cascade,
Material Instance, Mesh/Texture 계약에서 기존
`CEffectCatalog -> CEffectPresentationService -> CEffectObject -> CEffectDocumentRenderer`
경로로 실행한다. 사용자가 46개 Particle layer를 다시 조립하거나 수치를 손으로 재입력하지 않는다.

이번 구현 단위는 A의 활성 Particle 46개가 참조하는 원본 Material 전체를 손실 상태까지 명시한
typed profile로 전환하고, Particle/Material runtime을 실제 월드 검증이 가능한 상태로 닫는 것이다.
2026-08-07 GPU 캡처에서 확인된 형상 붕괴는 Material 튜닝보다 앞선 실행기 결함으로 판정했다.
따라서 cooked `FRawDistribution` payload 해석, SourceRecipe/Detail 값 소유권, SubUV atlas 합성을 먼저
교정한 뒤 같은 장면을 다시 캡처한다. 이 세 계약이 통과하기 전에는 밝기나 크기를 Authored 값으로
손보지 않는다.
`Light 2 + Screen Post 3` typed presentation channel은 이미 합의한 다음 단계로 유지하며, 이번 완료와
섞어서 A 전체가 100% 완료됐다고 보고하지 않는다. PNG `DimensionMaster_A00~A04.png`는 원본
수치의 입력이 아니라 동일 캐릭터·카메라·timeline의 GPU 결과를 판정하는 마지막 A/B 자료로만 쓴다.

## 2. 완료 용어

원본 데이터가 존재하는 것과 우리 GPU가 같은 의미로 실행하는 것을 분리한다.

| 상태 | 의미 | 완료 판정 |
|---|---|---|
| `SOURCE_EXACT` | 원본 package/export/property에서 값과 출처를 손실 없이 추출 | Source Data 완료 |
| `RUNTIME_EXACT` | 원본 의미를 우리 runtime과 shader가 동일하게 실행 | 해당 기능 완료 |
| `RECONSTRUCTED_PROFILE` | 조리 과정에서 graph topology가 제거되어 보존된 값과 화면 근거로 식을 재구성 | 근사 유지 |
| `UNSUPPORTED` | 문서는 보존하지만 runtime 의미를 실행하지 못함 | 미완료 |
| `MISSING_RESOURCE` | 필요한 Mesh/Texture/Material 입력을 stage하지 못함 | 미완료 |

`RECONSTRUCTED_PROFILE`을 `RUNTIME_EXACT`로 승격하려면 원본 uncooked graph, compiled shader 또는
원본 renderer oracle처럼 식 전체를 판정할 수 있는 추가 정본이 필요하다. A00~A04 다섯 장의 GPU
A/B는 시각 회귀 자료이지 입력 공간 전체의 의미 동등성을 증명하는 자료가 아니다.

## 3. 현재 실측 정본

```text
skillId                              2050240
inputSlot                            A
displayName                          경계 돌파
body clips                           telekinesisthrust_01 -> _04

PlayParticleEffect source notify     26
enabled base notify                  13
disabled FX-07 notify                13
disabled notify runtime execution     0

active source emitter partition      42
inactive FX-07 emitter partition      35
runtime Elements                     51
Particle / Light / Screen Post       46 / 2 / 3

source-material pending emitter      10
source-material runtime occurrence   14
unique source material                5
effect.standard Particle occurrence 32
standard unique Particle material    21
parameter-name heuristic mapping     72
active Particle material identity    26
active parent/profile graph group     21

active Particle source module        518 / 28 classes
Particle DynamicParameter module      33
Particle CameraOffset module          27
SubUV module occurrence               2
deferred ScreenPost Dynamic/Camera      3 / 3
```

기존 계획의 `107 Elements / 77 emitter 실행`은 폐기한다. 77은 활성 42와 비활성 FX-07 35를
합친 원본 증거 수다. 비활성 graph는 lossless source evidence에는 보존하지만 기본 A runtime에는
절대로 넣지 않는다.

이미 구현된 경계는 다음과 같다.

- Notify enabled typed field를 파싱하고 활성 13개만 materialize한다.
- timeline에 없는 ghost ParticleSystem을 만들지 않는다.
- Notify 위치와 source socket 위치에 UE unit `0.01`을 각각 자신의 경계에서 정확히 한 번 적용한다.
- `WP_SWM_M_1`, `FX_State_01`, `b_ROOT`를 실제 캐릭터 bone/socket에 연결하고 follow 시 매 프레임
  matrix를 추적한다. 실패를 root identity로 조용히 대체하지 않는다.
- CameraOffset은 world Z가 아니라 inverse-view camera forward로 적용한다.
- DynamicParameter는 CPU 평가와 GPU instance 전달까지, SubUV는 단일 atlas frame UV 적용까지
  연결돼 있다.
- Effect Tool의 매 프레임 대형 JSON 직렬화 비교는 cache로 교체했다. Debug build는 통과했지만
  수정 후 profiler 재측정은 아직 수동 미검증이다.

## 4. Material 원본 증거와 한계

A의 pending emitter 10개, runtime occurrence 14개는 다음 5종 source material을 사용한다.

1. `bfx_d_pa_circ_01_01_dt_ad`
2. `fx_j_pa_dot_ad_01`
3. `fx_d_pa_ring_11_09_ts_tr`
4. `fx_c_pa_aura_02_tr`
5. `fx_mm_onelayerdistortion_02_01_ad`

나머지 Particle 32개 `effect.standard` occurrence도 완료된 원본 Material이 아니다. 21종 source
material의 parameter 이름을 Base/Noise/Mask/Emissive/Dissolve로 72번 추정한 축약 경로다. 따라서
5종만 가시화하고 `A Material 완료`로 판정하지 않는다. 활성 A Particle의 material identity 26종을
동일 부모 graph 기준 21개 profile group으로 묶어 같은 evidence/runtime 계약으로 분류한다.

원시 UPK에서 다음은 자동 복구 가능하다.

- MI parent와 package/export identity
- blend mode, lighting model, two-sided, depth/distortion 상태
- instance override 및 parent default scalar/vector/texture/static switch
- referenced texture와 parameter 이름
- native tail과 expression occurrence의 provenance/hash

그러나 현재 staged cooked package에는 일반 연산 graph가 제거된 부분이 있다.

```text
circle       Expressions 45 중 non-null 16
ring         Expressions 100 중 non-null 35
aura         Expressions 12 중 non-null 2
distortion   Expressions 144 중 non-null 10
dot          Expressions 7 중 non-null 0
```

남아 있는 노드는 주로 parameter expression이며 Multiply/Add/Lerp, output 연결과 일부 static branch가
`0/null`이다. 따라서 부모/파라미터를 정확히 읽는 것과 원래 pixel 식을 정확히 아는 것은 다르다.
native tail hash나 음수 정수 reference scan은 provenance이지 실행 의미 decoder가 아니다. 정확한
dependency/semantic 판정에는 package version에 맞는 `FMaterialResource / UniformExpressionSet`
decoder가 필요하다. 현재 source pack만으로 21개 profile graph 식 전체를 `RUNTIME_EXACT`라고 선언하지
않는다.

직접 확인된 pending profile dependency는 다음과 같다.

```text
ring         fx_bg_waterspray_01, fx_d_noise_009
distortion   fx_a_noise_002
aura         fx_a_glow_009, fx_a_cloud_026
circle/dot   tagged referenced texture 없음
```

`fx_a_glow_009`, `fx_a_cloud_026`은 현재 DimensionMaster runtime resource에 정상 staged copy가
없다. 다른 class 폴더나 0-byte 파일을 재사용하지 않고 원본 texture package에서 자동 cook한다.

## 5. 구현 구조

```text
UE3 Material / Material Instance
        |
        +-- Source Evidence Contract
        |     parent/render state/parameter/texture/expression coverage
        |     null topology evidence/native-tail hash
        |
        +-- Source Material Profile Registry
              stable profile ID
              required resources
              typed scalar/vector/static-switch payload
              DynamicParameter channel semantics
              SubUV mode
              shader pass/blend/depth policy
                    |
                    v
Effect v11 document -> validate -> stage -> commit
                    |
                    v
DocumentRenderer -> particle shader profile -> GPU
```

Source Evidence는 재추출 가능한 불변 정본이고, Source Material Profile은 우리 runtime 실행 계약이다.
PNG 기반 미세 수정은 profile override로 분리하여 Imported 재생성이 덮어쓰지 않게 한다.

## 6. 구현 단계

### G40. A 구조·성능 baseline 고정

- A를 `enabled notify 13 / disabled 13 / active partition 42 / Elements 51`로 고정한다.
- 비활성 source payload와 module order/duplicate occurrence는 evidence에만 보존한다.
- Render frame에서 `CEffectDocumentCodec::Serialize` 호출이 0인지 source harness로 고정한다.
- 새 profiler capture에서 `EffectTool.ModelViewWindow`, CPU frame, draw call을 기록한다.
- profiler 확인 전에는 3 FPS 해결을 자동 PASS로 적지 않는다.

### G41. Source Material Evidence Contract

- UE3 Material output/input tagged struct를 typed decoder에 추가한다.
- MI parent를 같은 package의 local Material export까지 resolve한다. 현재
  `parentSourcePhysicalPackage=null`을 정상값으로 인정하지 않는다.
- parent chain, render state, default/override parameter, referenced texture, static switch,
  expression non-null/null coverage와 native-tail hash를 lossless JSON으로 생성한다.
- package version별 `FMaterialResource / UniformExpressionSet` decoder로 native dependency와 uniform
  expression을 해석한다. 해석하지 못한 tail은 hash와 raw evidence만 보존하고 semantic coverage를
  `UNKNOWN`으로 둔다.
- exact source texture를 DimensionMaster resource domain에 stage하고 절대 경로나 다른 class resource의
  우연한 상대 경로에 의존하지 않는다.
- 중복 parameter, 끊긴 parent, 잘못된 export index, missing texture는 publish 전에 실패시킨다.

산출물:

```text
Data/Effects/Imported/DimensionMaster/ActionSource/
  DimensionMaster.A.source-material-contract.json
  DimensionMaster.A.source-material-contract.receipt.json
```

### G42. Effect v11 Source Material 문서 계약

- `EFFECT_MATERIAL_DESC`에 optional typed source material payload를 추가하고 format version을 11로
  올린다. v3~v10 reader 호환은 유지한다.
- payload는 stable `profileId`, semantic status, named texture/scalar/vector/static-switch parameter,
  render state, DynamicParameter channel binding, SubUV mode를 소유한다.
- Codec과 publisher는 profile ID, finite numeric value, duplicate parameter, safe resource asset ID,
  required texture slot과 semantic status를 검증한다.
- load는 `parse -> validate -> stage -> commit`을 지키고 profile/resource stage 실패 시 기존 preview를
  유지한다.
- Effect Detail은 선택한 source material의 parent, render state, 원본 parameter, graph coverage,
  runtime profile과 `SOURCE_EXACT / RECONSTRUCTED_PROFILE` 상태를 함께 표시한다.

### G43. A 21개 Profile Graph Registry와 자동 materialization

- 활성 Particle 46개의 material identity 26종을 parent graph 기준 21개 profile group에 자동
  연결한다. 기존 standard material 21종의 72건 parameter-name heuristic은 명시적 source profile로 교체하고 target
  gate에서 0으로 만든다.
- pending 10 source emitter와 반복 notify로 생성된 14 runtime occurrence도 같은 profile/parameter
  payload를 상속한다.
- circle/ring/distortion은 남아 있는 parameter와 texture evidence를 우선 사용한다.
- dot/aura도 별도 profile을 제공하되 graph topology가 없는 식은
  `RECONSTRUCTED_PROFILE`로 유지한다.
- source material이 알려진 profile이면 Renderer의 현재 skip을 제거한다. 알려지지 않은 profile은
  render-time `S_FALSE`로 숨기는 것이 아니라 document stage를 실패시키고 stable error/source path를
  반환하여 기존 preview를 유지한다.
- Base/Noise/Mask/Emissive/Dissolve 이름 추정은 source profile의 증거로 사용하지 않는다.

### G44. GPU Material·DynamicParameter·SubUV 실행

- profile별 required texture와 scalar/vector 값을 particle shader에 전달한다.
- profile별 blend/depth/two-sided/distortion pass를 선택한다.
- Particle 범위 DynamicParameter 33개(pending profile 5, 기존 standard profile 28)의 4채널은
  profile registry의 typed channel binding에 따라서만 소비한다. deferred ScreenPost 3개는 이번
  Particle gate에 섞지 않는다.
  전역으로 `x * emissive` 같은 임의 공식을 넣지 않는다.
- `psuvim_linear_blend`인 SubUV 2개는 현재 floor 단일 frame을 두 frame atlas sample과 원본 blend
  alpha로 확장한다. 두 occurrence의 8x4 atlas, `RandomImageTime=1`,
  `AllowImageFlipping=true`, `SquareImageFlipping=true`, loop와 last-frame policy까지 검증한다.
- Particle CameraOffset 27개는 이미 연결된 camera-forward 경로를 targeted snapshot으로 회귀
  검증한다. deferred ScreenPost 3개는 별도 channel로 남긴다.
- mesh renderer material override와 orientation/axis policy가 profile pass까지 유지되는지 확인한다.
- 활성 Particle source module 518 occurrence/28 class를 최신 executor capability에서 다시 분류한다.
  stale receipt의 partial/unsupported 수치를 그대로 쓰지 않는다. source에서 찾은 `accounted`, 실제
  state/shader 결과에 반영한 `executed`, `unsupported`, `silent ignored`, `RUNTIME_EXACT`,
  `RECONSTRUCTED_PROFILE`을 별도로 집계한다.

### G45. Runtime/Tool 검증

- All Effects에서 A 전체 Assembly를 load한다.
- Data Files에서 A Component/WFX를 개별 load·solo·source start seek한다.
- Dimension Master 캐릭터, 실제 body clip, 실제 bone/socket으로 검증한다. 다른 class 캡처는 A 시각
  증거로 인정하지 않는다.
- emitter별로 position/velocity/size/rotation/color/SubUV/DynamicParameter/material profile을 snapshot한다.
- 고정 카메라와 원본 notify time에서 `DimensionMaster_A00~A04.png`를 수동 A/B한다.
- 밝기·노출·profile 식의 마지막 차이만 Authored override로 기록하며 source baseline을 수정하지 않는다.

### G48. Cooked FRawDistribution 실행 계약 교정

- UE3 cooked `LookupTable`의 첫 두 float는 전체 분포의 range cache이며 평가 sample이 아니다.
- 실제 sample payload는 index 2부터 시작한다. 정본 entry stride는
  `componentCount * (Op이 Random/Extreme이면 2, 아니면 1)`이고 Vector3를 4 float로 임의 정렬하지
  않는다. 원본 `LookupTableChunkSize`와 `LookupTableNumElements`가 있으면 이 정본 shape와 정확히
  일치해야 하며, 더 큰 stride도 padding으로 추측하지 않고 fail-closed한다.
- `LookupTableTimeScale`과 `LookupTableStartTime`은 시간 index 계산에만 사용하며 range cache와
  혼동하지 않는다.
- `FRawDistributionVector.Type`의 하위 3비트는 UE runtime `LockFlag`다. 변환기는 이를
  `randomLockAxes`로 보존하고, 실행기는 UE처럼 축별 난수 세 개를 소비한 뒤 XY/XZ/YZ/XYZ 잠금을
  적용한다. A의 `Type=4` 13 occurrence를 실데이터 회귀로 고정한다.
- C++ evaluator와 Python 변환기의 축약 preview가 같은 helper 계약을 사용하도록 맞춘다.
- A의 실제 `particlespriteemitter_17.StartSize` table
  `[0, 500 | 500, 0, 0 | 500, 0, 0]`은 `(500, 0, 0)`으로 평가되고 프로젝트 단위에서
  `(5, 0, 0)`이 된다. `PSA_Square` renderer가 이를 최종 `5 x 5` quad로 투영한다.
- 잘못된 header 소비는 size만의 문제가 아니므로 position, velocity, color, rotation,
  DynamicParameter를 포함한 A 전체 distribution shape를 다시 검증한다.

### G49. SourceRecipe와 Authored Detail 소유권 분리

- 원본 Cascade `ParticleModuleColor`, `ColorOverLife`, `ColorScaleOverLife`는 SourceRecipe가 단독으로
  평가한다.
- `Detail.color.multiply`와 `Detail.linearLerp.colorMultiply`는 사용자의 후단 override다. 변환기는
  source color를 이 필드에 다시 복사하지 않고 identity/disabled로 생성한다.
- Playback의 `source color * authored override` 합성은 유지한다. 이렇게 해야 원본 색을 한 번만
  실행하면서 이후 Effect Detail multiplier 튜닝도 보존할 수 있다.
- 현재 A의 Authored와 Imported `detail`은 51/51 동일하여 이번 자동 재생성에서 잃을 사용자 override가
  없음을 migration gate로 고정한다. 이후 별도 override 저장 계약을 만들기 전에는 이 동등성 검사를
  통과한 문서만 자동 교체한다.

### G50. Source SubUV와 공통 UV 합성 교정

- SourceRecipe의 `ParticleModuleSubUV`가 atlas frame, 다음 frame, blend, flip을 소유한다.
- `Detail.UV`는 후단 panner/start/wave override만 소유한다. 변환기는 source SubUV가 있으면
  `sequence=false`, `tileColumns=tileRows=1`, `tileIndex=0`으로 생성한다.
- shader에서는 instance SubUV transform 뒤에 공통 UV panner를 적용하되 atlas tile scale을 다시
  곱하지 않는다.
- A의 SubUV 2개가 8x4 atlas를 한 번만 사용하고 유효 tile이 `1/8 x 1/4`인지 snapshot으로 고정한다.

### G51. A 실데이터 회귀·빌드·수동 판정

- synthetic distribution test와 A 실제 emitter test를 함께 둔다. synthetic만 통과하고 실제 문서가
  다시 깨지는 상태를 허용하지 않는다.
- A Imported/Authored를 같은 변환기로 재생성하고 Effect Component/Assembly/Runtime Catalog까지
  publish하여 중간 단계의 identity를 검증한다.
- Python unit, C++ frontend harness, Effect pipeline, Debug Client/HLSL build, startup smoke,
  `git diff --check`, ProjectAudit 순서로 실행한다.
- 자동 검증 뒤 고정 카메라 A 캡처를 새로 얻어 형상·방향·시간을 먼저 판정한다. 그 결과에서 남는
  차이만 21개 공유 Material profile과 Light/Post 단계로 넘긴다.

## 7. 자동 검증

1. source material contract 및 versioned material-resource parser unit test
2. local parent resolve, duplicate parameter, missing export/resource 실패 test
3. A active/disabled notify 및 42 active partition accounting test
4. Particle material identity 26/26, profile graph 21/21, pending emitter 10/10,
   pending runtime occurrence 14/14 coverage test
5. cooked LookupTable range-cache skip, entry stride, time interpolation, random/extreme test
6. A 실제 StartSize/Color/Position distribution 평가 snapshot test
7. Source color와 Authored color override가 각각 한 번만 적용되는 test
8. SubUV dual-frame interpolation 및 atlas transform 단일 적용 snapshot test
9. unknown profile 및 stage 중간 실패 rollback test
10. Effect pipeline publish/materialization test
11. active Particle module 518 occurrence/28 class runtime coverage test
12. targeted x64 Debug Client build와 shader compile
13. startup smoke, ProjectAudit, 관련 `git diff --check`

광범위 ProjectAudit의 unrelated failure를 A PASS나 A 실패로 섞지 않는다. 이번 범위와 직접 연결된
Effect audit 결과만 별도로 기록한다.

## 8. 완료 판정

### Particle/Material runtime 검증 진입 gate

```text
enabled / disabled notify             13 / 13
disabled notify runtime execution      0
active source partition               42 / 42
runtime Elements                       51
Particle / Light / Screen Post         46 / 2 / 3
source material emitter coverage       10 / 10
source material runtime occurrence     14 / 14
Particle material identity             26 / 26
Particle parent/profile graph          21 / 21
legacy parameter-name heuristic          0
accounted Particle module occurrence  518 / 518
executed Particle module occurrence   518 / 518
unsupported execution                   0
silent ignored module                   0
silent white/pink fallback               0
malformed cooked lookup payload           0
source color duplicated into Detail       0
source SubUV duplicated into Detail       0
per-frame document serialization         0
unresolved external module               0
Debug Client + shader compile          PASS
save/reload                            identical
```

### 상태별 보고

| 영역 | 완료 조건 |
|---|---|
| Source Data | Particle material identity 26/26과 profile graph 21/21의 parent/parameter/render state/texture/expression/tail coverage provenance |
| Document Contract | v11 parse/validate/stage/commit 및 rollback PASS |
| Tool UX | stable material/profile 선택과 원본/runtime/coverage 표시 |
| Runtime Execution | Particle module 518/518, pending 14/14 실행, DynamicParameter 33 typed 소비, SubUV linear blend/flipping |
| Material | missing 0, silent fallback 0, 각 profile의 semantic status 명시 |
| GPU A/B | 새 profiler와 A00~A04 수동 결과 기록 |

동일 A scene의 새 profiler는 per-frame Serialize 0과 기존 `ModelViewWindow` 평균 약 287ms 대비
최소 90% 감소를 함께 만족해야 성능 gate를 통과한다.

`RECONSTRUCTED_PROFILE`이 남아 있어도 A를 실행·비교할 수 있는 Particle/Material runtime slice는
완료할 수 있다. 그러나 이를 `원본 graph EXACT`, `A Material EXACT` 또는 `A 전체 100% 완료`로
부르지 않는다.
Light 2와 Screen Post 3의 typed presentation 및 최종 pixel 동등성은 별도 완료 gate다.

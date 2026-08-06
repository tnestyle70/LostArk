# 차원술사 2050240 A 경계 돌파 Particle/Material 복원 결과

## 1. 결론

차원술사 A의 46개 Particle을 사용자가 손으로 다시 조립하는 방식은 사용하지 않았다. 원본
Notify/Cascade/Material provenance를 51개 실행 Element와 21개 공유 Material profile로 자동
materialize하고, 기존 Effect runtime과 Effect Tool이 그 계약을 소비하도록 확장했다.

이번 변경으로 닫힌 범위는 A의 Particle/Material 문서·실행 파이프라인이다. 조리된 source package에서
부모 Material의 일반 연산 graph topology가 제거된 21개 profile은 정직하게
`RECONSTRUCTED_PROFILE`로 유지한다. 따라서 이번 결과를 원본 Material graph EXACT 또는 A 전체
픽셀 동일로 보고하지 않는다. 합의한 `Light 2 + Screen Post 3` typed presentation과 동일 장면 GPU
A/B는 다음 실행 검증 범위다.

## 2. 구현 결과

### Source evidence와 자동 materialization

- A 기본 분기는 enabled Notify 13개만 실행하고 disabled FX-07 Notify 13개는 evidence에만 보존한다.
- 실행 문서는 `51 Elements = Particle 46 + Light 2 + Screen Post 3`이다.
- Particle source module은 518 occurrence, 28 class를 원본 순서와 중복 occurrence 그대로 보존한다.
- Particle Material identity 26종을 parent 기준 21개 stable profile group으로 묶는다.
- 46개 Particle 모두 v11 `sourceProfile`을 갖는다.
- 32 occurrence는 공통 reconstructed standard shader, 14 occurrence는 circle/dot/ring/aura/
  one-layer-distortion 전용 profile을 사용한다.
- 원본 aura texture `fx_a_glow_009`, `fx_a_cloud_026`을 export/cook했으며 resource missing은 0이다.

### Effect v11 계약

- `SOURCE_EXACT / RUNTIME_EXACT / RECONSTRUCTED_PROFILE / UNSUPPORTED /
  MISSING_RESOURCE` 상태를 문서에 저장한다.
- profile ID, runtime shader profile, parent Material, scalar/vector/static switch,
  Dynamic Parameter 4채널 의미와 SubUV policy를 typed payload로 저장한다.
- codec과 publisher가 stable ID, finite value, 중복 parameter, semantic status를 검증한다.
- v3~v10 문서는 migration을 위해 읽을 수 있지만, profile 없는 legacy source material은 흰색 fallback으로
  렌더링하지 않고 fail-closed로 유지한다.

### GPU 실행

- Dynamic Parameter를 Particle instance에서 Pixel Shader까지 전달하고 typed semantic에 따라 opacity,
  emissive, dissolve, UV pan, distortion, radial size에 적용한다.
- A의 Dynamic Parameter module 33 occurrence 중 실제 semantic을 가진 32 Element를 profile과 연결했다.
- SubUV 2 Element는 현재/다음 atlas frame과 blend alpha를 전달하며 deterministic horizontal/vertical
  flipping을 적용한다.
- circle/dot/ring/aura/distortion profile의 전용 HLSL 계산과 exact texture binding을 추가했다.
- CameraOffset은 기존 world Z가 아니라 camera-forward 경로를 사용한다.

### Effect Tool과 성능 경계

- 선택한 Element에서 source Material parent, 상태, profile, Dynamic Parameter semantic, SubUV,
  scalar/vector/static switch를 읽기 전용으로 확인할 수 있다.
- source Material resource slot은 고정 pending 카드가 아니라 현재 profile의 Mesh/Texture binding을 사용한다.
- `CEffectDocumentCodec::Serialize`를 이용한 runtime equivalence 비교는 load/save/promote/commit 시점의
  `Refresh_RuntimeEquivalence()`에서만 수행한다. Effect Tool Render 함수는 cached bool만 읽는다.
- 과거 265 Component 고정 하네스를 현재 publisher 정본인 180 Component로 교정했다.

## 3. 자동 검증

### Python

```text
python -m unittest
  test_build_effect_source_material_contract
  test_extract_umodel_material_dependencies
  test_effect_extraction_tools

17 tests / PASS
```

고정된 A 계약:

```text
Elements                         51
Particle                         46
enabled source profiles          46
reconstructed profiles           46
stable parent/profile groups      21
specialized shader occurrences    14
dynamic semantic Elements         32
SubUV Elements                     2
Particle source modules          518 / 28 classes
```

### C++ 하네스

```text
ClientFrontendHarness --effect-executor-fast
failures 0

ClientFrontendHarness --effect-runtime-fast
failures 0
```

검증된 실행 계약:

- 30/60/144 FPS deterministic playback
- source module order와 duplicate occurrence 보존
- seeded distribution, local/world, bone/socket, event, vector field synthetic execution
- DimensionMaster authored 11문서 stage
- A v11 serialize/parse/serialize identity
- runtime 15 Effects와 180 WFX Components stage
- All Effects Assembly와 Data Files Component hierarchy 소비

### Publisher와 빌드

```text
Test-EffectPipeline.ps1                         PASS
Publish-Effects.ps1 -Mode Validate              15 Effects PASS
Test-EffectToolFinal.ps1                        PASS
ClientFrontendHarness x64 Debug build           errors 0
Client x64 Debug build + HLSL compile           errors 0
targeted git diff --check                       PASS
관련 JSON parse                                 PASS
```

## 4. 완료와 미완료

| 영역 | 현재 판정 |
|---|---|
| A 분기/Notify/51 Element 문서 | 완료 |
| source Material evidence와 v11 profile 저장 | 완료 |
| resource export/cook와 runtime stage | 완료 |
| Dynamic Parameter/SubUV GPU 경로 | 구현·빌드·계약 하네스 완료 |
| Particle module source accounting | 518/518 완료 |
| 각 518 occurrence의 원작 GPU 의미 동등성 | 수동 runtime 판정 전 미완료 |
| Material graph | 46/46 `RECONSTRUCTED_PROFILE`; `RUNTIME_EXACT` 0 |
| 동일 A 장면 profiler 재측정 | 수동 미검증 |
| A00~A04 고정 카메라 GPU A/B | 수동 미검증 |
| Light 2 / Screen Post 3 typed presentation | 다음 단계 |

현재 사용자가 직접 해야 하는 작업은 46개 Particle의 lifetime/velocity/scale 재입력이 아니다. 다음 실행에서
Dimension Master를 선택하고 A를 재생해 profile별 Solo와 A00~A04를 비교한 뒤, 차이가 남는 공유
Material profile의 식 또는 마지막 시각 override만 조정하면 된다.

## 5. 다음 실행 검증

1. Debug Client에서 `Dimension Master`를 선택한다.
2. All Effects에서 A 전체 Assembly를 load하고 Data Files에서 A Component를 개별 audition한다.
3. 같은 A 장면에서 profiler를 다시 저장해 `EffectTool.ModelViewWindow`, CPU frame, draw call을 이전
   `287.35 ms`와 비교한다.
4. 원본 notify time에서 A00~A04 고정 카메라 프레임을 비교한다.
5. 분홍 사각형, 흰 fallback, 잘못된 root pivot, 과대/과소 scale을 emitter/profile stable ID와 함께
   기록한다.
6. 그 결과가 통과한 뒤 Light/Camera/Screen Post/Sound typed presentation 단계로 이동한다.

## 6. 2026-08-07 실행기 교정 결과

### 원인과 반영

GPU 캡처에서 보인 가늘고 긴 초록 막대와 과도한 색 증폭은 수동 scale 튜닝 대상이 아니었다. 원인은
다음 네 실행 계약의 누락 또는 중복이었다.

1. UE3 cooked `FRawDistribution.LookupTable`의 첫 두 range-cache 값을 실제 sample로 읽었다.
2. `FRawDistributionVector.Type & 0x07`의 random axis lock을 변환 문서에 전달하지 않았다.
3. 원본 Cascade Color를 SourceRecipe와 Detail에 동시에 복사해 두 번 곱했다.
4. 원본 SubUV 8x4와 Detail 8x4를 연속 적용해 UV를 `1/64 x 1/16`까지 다시 잘랐다.

다음과 같이 교정했다.

- cooked payload는 index 2부터 읽고, Float stride 1/2와 Vector3 stride 3/6을 사용한다.
- 명시된 chunk/numElements가 정본 shape와 다르면 stage를 fail-closed한다.
- Op 4는 Op 3으로 clamp하지 않고 unsupported로 거부한다.
- Type 4는 UE와 같이 난수 X/Y/Z를 세 번 소비한 뒤 XYZ가 X 난수를 공유한다.
- SourceRecipe는 원본 Color/SubUV를 소유하고 Detail은 identity 후단 override로 생성한다.
- Source SubUV transform은 순수 실행기 함수로 분리했다. `8x4, frame 10.5`는
  `scale=(1/8,1/4), blend=0.5`로 평가되고 renderer의 공통 UV scale은 `(1,1)`이다.
- Sprite size X/Y는 UE의 width/height이며 음수 크기는 절댓값을 사용한다. `PSA_Square`는 최종 Y=X다.

### A 실데이터 계약

```text
Elements                         51
Particle                         46
source module occurrences       518 / 28 classes
cooked distribution lookup      604
malformed cooked lookup           0
raw Type 4 / XYZ lock             13 occurrences
source Color + Detail duplicate    0
source SubUV + Detail duplicate    0
SubUV                              2 elements
Material profile                  46 occurrences / 21 groups
```

대표 StartSize도 원본 수치로 평가된다.

```text
particlespriteemitter_17 = (500,   0,   0) -> project 5.0 -> Square 5 x 5
particlespriteemitter_32 = (250,-100,   0) -> abs Rectangle 2.5 x 1.0
particlespriteemitter_64 = ( 60, 180,   0) -> Velocity 0.6 x 1.8
```

### 재생성·게시 결과

```text
Authored/Imported precondition    11/11 detailDiff=0
Runtime Effects                   15
WFX Components                   180
Emitter Elements                1076
Source Action Cues                501
Assembly compile identity        true
Runtime Catalog publish          PASS
```

`build_dimensionmaster_base_effects.py`는 문서를 정상 생성했지만 전역 receipt에 아직 다른 module과
Material/Presentation 미지원이 남아 있어 설계대로 exit 1과 `runtimeExecutionComplete=false`를 반환했다.
이를 A 실행기 완료나 전체 차원술사 EXACT 완료로 오해하지 않는다.

### 검증

```text
Python 관련 unit tests                    31 PASS
Effect executor fast harness              failures 0
Effect runtime fast harness               failures 0
Effect pipeline tests                     PASS
Effect publish validate                   15 Effects PASS
Effect publish                            15 Effects / 180 Components PASS
Effect Tool final harness                 PASS
ClientFrontendHarness x64 Debug build     errors 0
Client x64 Debug build                    errors 0
Client startup smoke                      12초 생존 PASS
Component/Assembly verify-existing        identity true
관련 JSON parse                           PASS
git diff --check                          PASS
```

전체 `ProjectAudit`의 이펙트 항목은 통과했다. 공유 작업트리의 기존
`projects.data-source-visibility: expected=473, project=225, filters=225` 한 건 때문에 전체 명령의
exit code만 1이며 이번 이펙트 실행기 실패는 아니다.

### 현재 완료 경계

자동 검증으로 닫힌 것은 Particle의 원본 수치 해석과 실행 경로다. 다음 수동 검증에서 A의 크기, 형상,
방향, pivot, Color, SubUV가 정상인지 같은 장면으로 확인한다. 이 검증이 통과하면 남은 큰 화면 차이는
21개 공유 `RECONSTRUCTED_PROFILE`의 Material 식으로 좁혀진다. 다만 Material을 닫은 뒤에도 A 전체
완료에는 Light 2, Screen Post 3, profiler/3 FPS 원인 확인과 고정 카메라 A/B가 남는다.

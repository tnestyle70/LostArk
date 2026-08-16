# 2026-08-15 G3 generic authoring family과 저작 소유권 계약

이 문서는 4직업 Track A candidate를 Effect Tool에서 실제로 튜닝 가능하게 만들기 위한 두 계약을
고정한다. 하나는 **원본 Material family를 사용자가 다룰 수 있는 소수의 authoring family로 접는 것**,
다른 하나는 **compiler가 소유하는 값과 artist가 소유하는 값을 나눠 re-materialize에도 사용자의
수정이 살아남게 하는 것**이다.

> **2026-08-15 Product admission 정책 정정**: 이 문서의 `AUTHORING_APPROXIMATE` 전면 Product
> 거부 문구는
> [`FOUR_CLASS...IMPLEMENTATION_PLAN.md` §13](../08-14/2026-08-14_FOUR_CLASS_VALTAN_EFFECT_RESTORATION_AND_RAID_PERFORMANCE_IMPLEMENTATION_PLAN.md#13-product-approved-approximate-admission과-first-canary-transaction)의
> cue-scoped 정책으로 대체한다. 명시적 사용자 승인, authored SHA, binding tuple, provenance,
> rollback receipt가 모두 일치한 cue만 `PRODUCT_APPROVED_APPROXIMATE`로 같은 runtime/renderer를
> 사용할 수 있다. Hard/fail-closed/unsafe element는 계속 Product 금지다.

전제는 다음과 같다.

- 리소스 추출을 더 늘리는 단계가 아니다. 현재 병목은 `EVIDENCE -> COMPILER/AUTHORING`이다.
- 원본 family 112개를 Winters shader 112개로 만들지 않는다.
- family 이름 문자열로 표현을 추정하지 않는다. 분류축은 실제 캡처된 evidence만 사용한다.
- 이 문서 범위에서 `Data/Effects/Authored/*.unified.effect.json` 101개를 다시 쓰지 않는다.
  G3 코드를 Codex의 admission/renderer/playback/Tool 변경과 통합하고 Client Debug를 빌드한 뒤, 사용자가
  기존 101 baseline 대표 스킬을 육안 검증한다. 방향 승인 뒤 denominator 재기준선과 함께 한 번만 write한다.

## 통합 입력과 중지 경계

G3는 다음 Codex 소묶음 위에 통합한다.

- grouped texture sampling UV와 carrier edge-feather UV 분리
- source image flip을 signed geometry가 아니라 Current/Next SubUV에 적용하는 단일 Playback/Renderer 권위
- Full / Authoring Approximate / Hard 3-way exactness와 cue-scoped Product admission
- visible exact `epet_spawn`, same-document route, 최대 4,096 queue 계약
- Lance cooked-partial `dissolve` arithmetic을 `unbound` authoring approximate로 내리는 경계
- repo-local four-class material evidence provenance

현재 authored 101은 baseline Full 3,481 / fail-closed 1,007 / approximate 0으로 유지한다. no-write
projection 2,793 / 722 / 973을 이 문서 구현 도중 쓰거나 `EXPECTED_*`로 고정하지 않는다. G3 통합 범위의
종료 증거는 schema/Codec/Tool/reimport focused test와 Client Debug 빌드 준비까지이며, Client/UI 실행과
visual fidelity 판정은 사용자 경계다.

---

## 1. 분류축 실측

입력은 `Data/Effects/Imported/FourClassCombat/FourClassCombat.source-material-evidence.json`의
parent `renderState`와 authored 문서 4,810 element다. parent evidence render state는 901개
material에 대해 색인됐고, element 기준 4,012/4,810(83%)이 evidence blendMode를 직접 가진다.

```
evidence blendMode   BLEND_Translucent 2375 / BLEND_Additive 1544 / BLEND_Masked 93 / 미보유 798
evidence twoSided    False 3268 / True 744 / 미보유 798
evidence noDepth     False 3994 / True 18 / 미보유 798
subUVMode            none 4272 / psuvim_linear_blend 276 / ..._random_flip_square 262
```

evidence blendMode가 없는 798개는 compiler가 이미 기록한 `material.renderProfile`로 채운다.
이 필드 자체가 evidence에서 파생된 값이며 family 이름 파싱이 아니다.

```
renderProfile  alpha_two_sided_depth_read 2441 / additive_two_sided_depth_read 1621 /
               alpha_one_sided_depth_read 393 / additive_one_sided_depth_read 353 /
               opaque_back_depth_write 2
```

lane 조합은 실제 runtime asset이 존재하는 슬롯만 센 값이다. 상위 분포는 다음과 같다.

```
1328  base
1060  (없음)
 720  base + mask + noise
 618  base + noise
 262  base + dissolve + noise
 205  base + mask
 190  base + emissive + mask + noise
 122  base + dissolve
 101  base + dissolve + mask + noise
  72  base + dissolve + emissive + mask + noise
```

## 2. authoring family 13개

축은 `rendererShape × blendClass × subUV` 셋뿐이다. 결과는 13개이며 상위 6개가 96%를 덮는다.

| authoring family | element | 비고 |
|---|---:|---|
| `SPRITE_ADDITIVE` | 1,551 | |
| `SPRITE_TRANSLUCENT` | 1,424 | |
| `MESH_TRANSLUCENT` | 889 | |
| `SPRITE_TRANSLUCENT_SUBUV` | 335 | atlas 재생 |
| `MESH_ADDITIVE` | 225 | |
| `SPRITE_ADDITIVE_SUBUV` | 196 | atlas 재생 |
| `MESH_MASKED` | 78 | |
| `DECAL_TRANSLUCENT` | 77 | 기존 Decal 경로 유지 |
| `SPRITE_MASKED` | 15 | |
| `MESH_MASKED_SUBUV` | 5 | |
| `MESH_TRANSLUCENT_SUBUV` | 2 | |
| `DECAL_ADDITIVE` | 2 | |
| `MESH_UNKNOWN` | 1 | evidence blendMode와 renderProfile 모두 미확보. 계속 hard lock |

`twoSided`와 `disableDepthTest`는 family 축이 아니라 family 안의 **render state 필드**로 둔다.
같은 `SPRITE_ADDITIVE`라도 two-sided 여부는 element마다 evidence 값을 그대로 쓴다. 이걸 family로
쪼개면 13개가 52개로 늘어나기만 하고 사용자가 다룰 단위는 늘지 않는다.

Distortion과 Procedural은 **별도 family로 만들지 않는다.** 실측상 distortion은 `noise` lane의
유무와 `renderProfile`로 이미 표현되고, procedural은 lane이 하나도 없는 상태(1,060개)라 family가
아니라 §5의 hard-lock 사유다. 이름만 보고 `Generic Distortion` family를 미리 만들면 소비자가 없는
인터페이스가 된다.

## 3. authoring family가 노출하는 편집 표면

family는 shader가 아니라 **사용자가 무엇을 고칠 수 있는지의 계약**이다. 각 family는 아래 표면만
노출하고, 그 뒤에 기존 `runtimeShaderProfileId`와 evaluator가 연결된다.

```
SPRITE_* 계열
  Resources   Base / Mask / Noise / Dissolve / Emissive   (해당 lane이 있을 때만)
  Color       tint, alpha
  Transform   position, rotation, scale, revolution
  UV          pan, tiling                                  (subUV면 atlas frame)
  RenderState twoSided, depthTest                          (evidence 기본값)

MESH_* 계열
  Geometry    WModel
  Resources   Base / Mask / Noise / Dissolve / Emissive
  Color       tint, alpha
  Transform   position, rotation, scale, revolution, preScale
  UV          pan, tiling
  RenderState twoSided, depthTest
```

lane이 없는 슬롯은 표면에 나타나지 않는다. 빈 슬롯을 만들어 흰 텍스처를 유도하지 않는다.

## 4. compiler-owned / artist-owned 소유권 계약

### 4.1 현재 계약의 결함

`CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage`의 현재 보존 목록은 다음과 같다.

```
artist-owned (보존)   strElementId, strDisplayName, strGroupId, bVisible,
                      ActionCueAttachment, TransformInheritance, Detail
                      + Decal에 한해 Base binding과 Material 전체
compiler-owned (갱신) SourceRecipe, ResourceBindings, Material
```

즉 **Particle에는 artist가 소유하는 material/resource 필드가 하나도 없다.** 사용자가 approximate
sprite의 DDS를 바꾸거나 material 파라미터를 조정해도 다음 re-materialize가 전부 되돌린다.
Decal만 예외로 열려 있다. 현재 no-write 722개 approximate를 열어도 이 상태로는 튜닝 결과가 남지 않는다.

### 4.2 고정할 소유권

| 필드 | 소유 | 근거 |
|---|---|---|
| `sourceNode`, `sourceRecipe`, source module/distribution | compiler | 원본 occurrence 정본 |
| `material.sourceMaterialPath`, `sourceProfile`, `renderProfile` | compiler | source evidence |
| `material.execution.failClosed` / `authoringApproximate` | compiler | admission 판정 |
| `resources[*]` 기본값 | compiler | exact source binding |
| `strElementId`, `displayName`, `groupId` | artist | 기존 유지 |
| `visible` | artist | 기존 유지 |
| `detail` (transform / color / size / lifetime 등) | artist | 기존 유지 |
| `actionCueAttachment`, `transformInheritance` | artist | 기존 유지 |
| **`authoringOverrides.resources[slotId]`** | **artist (신규)** | 사용자가 다시 바인딩한 DDS/WModel |
| **`authoringOverrides.scalars[name]` / `.colors[name]`** | **artist (신규)** | 사용자가 조정한 material 파라미터 |

### 4.3 authoringOverrides 계약

```
element.authoringOverrides = {
  "resources": [ { "slotId": "<compiler가 이미 아는 슬롯>", "assetId": "<Resources 상대 경로>" } ],
  "scalars":   [ { "name": "<parent가 선언한 파라미터명>", "value": <float> } ],
  "colors":    [ { "name": "<parent가 선언한 파라미터명>", "value": [r,g,b,a] } ]
}
```

불변식은 다음과 같다.

1. 블록이 비면 직렬화하지 않는다. 기존 문서와 byte 호환이다.
2. `slotId`는 compiler가 그 element에 이미 부여한 슬롯이어야 한다. 새 슬롯을 만들 수 없다.
   없는 슬롯을 만들면 lane 없는 곳에 텍스처를 끼워 넣는 우회가 되기 때문이다.
3. `assetId`는 기존 `Is_SafeResourceAssetId` 계약을 그대로 통과해야 한다. 절대 경로, drive,
   `..` 탈출은 거부한다.
4. `name`은 그 element의 `sourceProfile`이 실제로 선언한 파라미터여야 한다. 선언되지 않은 이름을
   만들어 넣을 수 없다.
5. reimport는 compiler 산출물을 먼저 새로 만든 뒤 override를 **마지막에 다시 적용**한다.
   override가 가리키는 슬롯/파라미터가 새 compiler 산출물에서 사라지면 그 override 항목만
   드롭하고 사유를 남긴다. 문서 전체를 실패시키지 않는다.
6. override가 하나라도 있으면 그 element의 fidelity는 `ARTIST_TUNED`로 표시한다.
   override는 exactness 주장이 아니다. `sourceExactGraph`, `graphProvenance`는 바뀌지 않는다.
7. override는 admission을 바꾸지 않는다. `authoringApproximate` element는 override를 넣어도
   계속 product publish에서 거부된다. Decal Base 바인딩이 fail-closed를 해제하는 기존 동작은
   Decal 고유 계약으로 유지하고 Particle로 확대하지 않는다.

### 4.4 fidelity 표기

Effect Tool은 element마다 다음 중 하나를 배지로 보여준다. 이것은 **표시 계약**이며 admission
판정과 분리된다.

```
EXACT        typed material execution이 enabled
RECONSTRUCTED  compiled sourceProfile을 사용하고 fail-closed가 아님
APPROXIMATE  material.execution.authoringApproximate
ARTIST_TUNED authoringOverrides가 비어 있지 않음 (위 셋 위에 덧붙는 표시)
INCOMPLETE   fail-closed이며 approximate 자격도 없음
```

## 5. G4로 넘길 hard-lock 분류

다음 세션은 lock을 "리소스가 없어서"로 뭉뚱그리지 않고 아래 축으로 먼저 나눈다.

| 층 | 사유 | 현재 수 | 해결 경로 |
|---|---|---:|---|
| RESOURCE | `.wmodel` 자체가 없음 | 110 | 추출/변환 트랙 |
| EVIDENCE | package는 있으나 exact named texture 불확실 | 과거 단독 alias 671, 최종 재집계 대기 | evidence resolution |
| COMPILER | compile 대상에서 누락 | 64 | seed/compile 대상 선정 |
| RUNTIME | Cascade module 미지원 | 199 (Ribbon 81 제외 118) | module 실행기 |
| AUTHORING | parent가 texture를 아예 선언하지 않음 | 650 | procedural evaluator. **추가 DDS 추출로 풀리지 않는다** |
| 복원 불가 | material expression graph가 cooked 단계에서 소실 | 전역 | family evaluator 재구성만 가능 |

650개는 `sourceProfile.textures`가 0개이고 parent가 `fx_d_pa_flare_02_ad`,
`fx_e_pa_mask_01_ad`, `fx_k_pa_velflow_01_tr` 같은 절차적 재질이다. 여기에 `white.dds`를 넣지
않는다. 표현할 증거가 없으면 잠근 채로 두고 사유를 명시한다.

## 6. 이 문서 범위에서 하지 않는 것

- `Data/Effects/Authored/*.unified.effect.json` 101개 write
- materializer `EXPECTED_*` denominator 재기준선
- Codex의 101 Stage/Draw baseline에 영향을 주는 모든 변경
- family evaluator 신규 구현. 13개 authoring family는 표현 계약이고, 전용 evaluator 승격은
  사용자가 화면을 보고 지목한 family부터 진행한다

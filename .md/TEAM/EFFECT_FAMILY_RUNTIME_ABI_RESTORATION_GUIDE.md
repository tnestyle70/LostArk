# Effect Family Runtime ABI 복원 가이드

초기 기준일: 2026-08-22

문서 성격: 계속 갱신하는 Effect 렌더링 구조·복원 공정 정본

최종 화면 판정자: 사용자

모델·소품을 Effect나 World Object로 추출·변환할 때도 [오브젝트 재질·환경 입력 공통 절차](../GB/렌더링이펙트복원V2.md#오브젝트-추출에서-재질환경-입력을-보존하는-공통-절차)를 따른다. geometry와 일반 texture 슬롯, native material descriptor, IBL/BRDF 및 장면 입력은 각각 실제 소비자까지 연결해야 한다. 이 문서의 carrier/ABI 검사를 WModel 변환 성공으로 대신하지 않는다.

이 문서는 Effect를 구성하는 element가 Winters Engine에서 실제 pixel이 되기까지의 소유권을
정의한다. 날짜별 PLAN/RESULT는 당시의 증거와 실행 로그를 보존하고, 이 문서는 그 증거에서 현재
유효한 구조와 다음 복원 순서만 유지한다.

## 1. 결론

사용자가 제시한 챔피언 추가 비유는 **구조적으로 맞다**. Effect도 안정적인 ID와 데이터로
구성 요소를 조립하고, 런타임이 각 element에 맞는 geometry/simulation과 material program을
연결해 화면에 출력한다.

다만 다음 두 가지는 교정해야 한다.

1. `element 하나 = HLSL 하나`가 아니다. 서로 다른 **exact shader program/permutation**에는
   번역된 HLSL이 필요하지만, 같은 program과 ABI를 쓰는 여러 element·skill·class는 이를
   재사용한다.
2. HLSL만 연결하면 끝나는 것도 아니다. HLSL은 pixel 계산식이고, 그 계산식에 올바른 입력을
   전달하는 runtime ABI와 renderer adapter가 함께 닫혀야 한다.

정본 흐름은 다음과 같다.

```text
Skill / Pattern cue
  -> Effect asset
    -> stable Element / Occurrence
       -> Composition       언제, 어디에, 얼마나 오래 생성하는가
       -> Carrier           어떤 geometry와 simulation으로 그리는가
       -> Material Descriptor
                            어떤 texture/channel/CB/sampler/state를 쓰는가
       -> Material Program  그 입력으로 색·coverage·굴절을 어떻게 계산하는가
       -> Renderer Adapter  VF/pass/RT/scene input에 어떻게 안전하게 배선하는가
    -> pixel / decal / trail / screen presentation
```

한 문장으로 줄이면 다음과 같다.

> Effect 문서는 무엇이 언제 어디서 생성되는지를, material family descriptor는 그 element가
> 어떤 입력과 상태로 어떤 pixel을 만드는지를, renderer adapter는 그 descriptor를 GPU에
> 안전하게 전달하는 방법을 소유한다.

## 2. 챔피언 데이터 비유의 정확한 범위

| 챔피언 구성 관점 | Effect 대응 |
|---|---|
| champion stable ID | effect ID, element stable ID, source occurrence ID |
| 스킬·애니메이션·수치 조립 | cue, timeline, transform, attachment, source recipe |
| 공용 gameplay system | 공용 particle/decal/trail/screen carrier와 scheduler |
| 스킬별 동작 데이터 | material descriptor, module curve, render profile |
| 재사용 가능한 코드 | material HLSL program, typed packet/layout, renderer adapter |
| 제품 승인 | source catalog admission, source validator/runtime 소비와 사용자 화면 승인 |

이 비유는 물리 폴더나 권위가 같다는 뜻은 아니다. 전투 command와 damage는 Shared/Server 계약을
따르지만 Effect pixel은 Client presentation이다. Effect를 추가할 때마다 Shared에 새 renderer
class를 만드는 것이 아니라, 기존 public cue와 Effect 데이터가 Client의 typed rendering 경로를
소비하게 한다.

## 3. 원작과 같은 방식으로 복원한다고 말할 수 있는 범위

LostArk의 원본 엔진 source를 보유하고 있지는 않으므로 내부 구현이 줄 단위로 같다고 단정하지
않는다. 현재 확보한 cooked UE3 자료가 증명하는 dataflow는 다음과 같다.

```text
Cascade emitter / module
  -> carrier와 child Material Instance 선택
  -> child MIC override + parent Material + effective static parameter set
  -> platform용 FMaterialShaderMap 선택
  -> ShaderMap 내부 VertexFactory / pass program 선택
  -> texture·sampler·constant·scene input 배선
  -> render state와 render target으로 출력
```

Winters의 source-exact 복원은 이 dataflow를 다음처럼 명시적으로 재구성한다.

```text
Authored effect / element
  -> carrier
  -> exact family + permutation descriptor
  -> translated HLSL program
  -> shared renderer adapter
  -> render output
```

따라서 “유리 SpriteParticle용 원본 material program이 있었고, Winters도 같은 역할의 program과
배선을 복원한다”는 설명은 맞다. 더 정확히는 원본 HLSL source를 되찾는 것이 아니라, cooked
ShaderMap/DXBC의 연산과 ABI를 읽을 수 있는 Winters HLSL과 typed descriptor로 번역하는 것이다.

## 4. 반드시 분리해야 하는 네 축

### 문서별 bloom 기여

V1 authored Effect root의 optional `bloomIntensity`는 그 문서 전체가 만드는 bloom 기여의 배율이다.
유한한 0~16만 허용하며 생략 기본값은 1.3이다. stable `effectAssetId`가 다른 Full Restore의 단계별·
clip별·통합 버전은 각각 독립 값을 저장한다. 같은 skill ID나 공통 source group을 이유로 합치지 않는다.
`Effect Tool V1 → Effect Detail → Skill Bloom Intensity`에서 현재 문서 전체 값을 즉시 조절하고
`Save Changes`로 저장한다. 편집은 같은 문서를 사용하는 preview의 draw scalar만 갱신하며 시계와
리소스를 다시 준비하지 않는다. 다른 문서의 값과 Rendering Benchmark의 전역 품질 값은 보존한다.

원본 HDR 색·emissive와 bloom 기여는 별도 출력이다. SceneHDR의 RT0 색, RT1 distortion에 이어
RT2에 문서 intensity를 적용한 bright-pass 기여를 모으고 기존 blur/final 합성에서 소비한다. 전역 bloom
enable·threshold·soft knee·scatter는 공통 품질 계약이다. 비Effect 기여는 전역 intensity를 사용하고,
V1은 문서 intensity를 사용한다. 최종 합성에서 전역 intensity를 다시 곱하지 않는다. 문서 intensity를 원본 RGB에
곱하거나 최종 전체 화면 intensity를 스킬마다 바꾸지 않는다. SceneColor를 사용하는 화면 왜곡도
다른 문서의 가중치를 잃지 않도록 bloom 입력을 함께 운반해야 한다.

draw별 threshold는 어두운 반투명 출력 여러 개가 겹쳐 임계치를 넘는 경우 기존 전체 화면 threshold와
다를 수 있다. 원본 HDR 보존, 문서별 가중치 독립성과 사용자 화면 판정은 각각 별도로 확인한다.
구현과 실행 검증의 현재 상태는 [스킬별 Bloom 결과](../GB/09-12/2026-09-12_EFFECT_PER_SKILL_BLOOM_RESULT.md)를 따른다.

현재 복원 단위는 다음 네 축의 곱이다.

```text
CarrierVariant × MaterialVariant × RenderVariant × CompositionVariant
```

### 4.1 Composition

- stable occurrence ID
- spawn time, life, loop, burst
- transform, anchor, attachment, action-facing policy
- particle module, source recipe, curve

Composition은 “언제 어디에 무엇을 만든다”를 소유한다. 계산식이나 GPU register를 소유하지 않는다.

### 4.2 Carrier

- SpriteParticle
- MeshParticle
- LocalDecal / projected decal
- Trail / Ribbon
- standalone Mesh / Sprite
- Light / ScreenPost
- CModel과 animation owner

Carrier는 geometry, vertex input, instance data, simulation과 draw scheduling을 소유한다. Sprite에
decal texture를 넣어도 carrier는 Sprite이므로 바닥 projection decal이 되지 않는다.

### 4.3 Material program과 descriptor

Material program은 UV, coverage, radiance, emissive, dissolve, distortion, refraction을 계산하는
HLSL 식이다. Descriptor는 그 식에 들어가는 exact texture, channel, CB lane, sampler와 policy를
소유한다.

같은 식에서 texture와 scalar만 달라지면 HLSL을 새로 만들지 않고 descriptor만 추가한다.
식이 다르면 같은 carrier adapter 위에 새 program/opcode를 추가한다.

### 4.4 Render variant와 adapter

- actual VertexFactory input signature
- pass와 shader stage 조합
- RT0 또는 MRT topology
- scene depth/color 같은 scene input
- blend, cull, depth/stencil, coverage/discard
- resource stage/commit/rollback과 state 복구

Adapter는 renderer가 descriptor와 program을 GPU draw로 바꾸는 경계다. `Translucent`,
`Additive`, `Masked`, one/two-sided는 중요한 render state이지만 그 자체가 material family는 아니다.

## 5. 용어

| 용어 | 이 문서의 의미 |
|---|---|
| Material | 원본 parent equation과 static permutation의 기반 |
| MIC / child | parent를 재사용하면서 texture·scalar·vector·static switch를 override하는 instance |
| Material Map | 일반 texture map이 아니라 compiled `FMaterialShaderMap`. 문맥상 texture map이면 `texture lane`이라고 쓴다 |
| ShaderMap | base Material GUID, effective static set, platform으로 선택되는 cooked program 집합 |
| DXBC | cooked program의 실행 bytecode와 signature. 수식·register 사용의 oracle |
| translated HLSL | DXBC 연산을 Winters가 컴파일하고 유지할 수 있게 다시 쓴 읽을 수 있는 식 |
| family | 같은 의미의 equation/ABI를 공유하는 재사용 집합. parent 이름 하나만으로 exact identity가 되지는 않음 |
| permutation | static set, platform, VF/pass로 갈리는 실제 program 선택 |
| descriptor | exact CB/SRV/sampler/VF/scene/RT/state 배선 자료 |
| packet/layout | C++가 shader에 전달하는 검증된 메모리·register 계약 |
| adapter | carrier draw를 특정 layout/pass/RT/state에 연결하는 공용 실행 경계 |
| Runtime Carrier | v15 authored 문서 안에서 occurrence selector와 typed packet/history를 소유하는 inline projection 입력. GPU HLSL program이 아님 |
| occurrence | effect 문서 안의 stable element instance |

## 6. V0에서 V1로 간다는 표현의 정확한 뜻

`V0`와 `V1`은 현재 JSON schema나 C++ enum의 공식 세대명이 아니다. 이 문서에서는 사용자가
말한 진행 방향을 설명하는 **architecture label**로만 쓴다. 실제 구현에는 다음 세 층이 공존한다.

| 성숙도 | 역할 | 현재 의미 |
|---|---|---|
| `FAMILY_LITE / compatibility` | 넓은 범위를 일단 보이게 하는 호환 경로 | 공용 UV/pan과 base/noise/mask/emissive slot을 사용하는 grouped/generic 근사 |
| `TYPED_FAMILY / reconstructed` | 명시적인 packet과 수식을 쓰는 재구성 경로 | RuntimeMaterialV2, StandardColorV1, Artist typed opcode처럼 의미가 닫혔지만 일부 sampler/VF/source-exact 증거가 남을 수 있음 |
| `SOURCE_EXACT / evidence` | 원본 program과 ABI 패리티를 닫은 fidelity 경로 | ShaderMap/DXBC/native wire/VF/pass/state와 수치 A/B를 통과한 exact tuple. Product·사용자 승인은 별도 축 |

위 표는 빠른 설명용 maturity ladder다. 실제 ledger는 다음 다섯 축을 분리한다.

```text
runtime architecture : V0_COMPATIBILITY | V1_TYPED_CANDIDATE
                       | V1_TYPED_PRODUCT | V1_TYPED_PRESENTATION_PRODUCT
disposition          : LIVE_PRODUCT | AUDITION_ONLY | LEGACY | RETIRED
                       | DISABLED_WITH_RECEIPT
fidelity evidence    : SOURCE_EXACT | BOUNDED_TRANSLATED
                       | PROJECT_RECONSTRUCTED | NOT_APPLICABLE
runtime admission    : NOT_ADMITTED | TOOL_ONLY | PRODUCT_ADMITTED
visual review        : NOT_REQUIRED | USER_REVIEW_PENDING | USER_APPROVED | USER_WAIVED
```

사용자의 표현으로는 `FAMILY_LITE`가 V0, typed program/layout/descriptor/adapter가 V1이다.
`SOURCE_EXACT`는 V1 안의 fidelity 등급이지 V1의 유일한 완료 형태가 아니다. 원본 증거가 없는
element도 typed ABI, fail-closed Product admission과 사용자 승인을 닫으면
`V1_TYPED_PRODUCT + PROJECT_RECONSTRUCTED`로 완료할 수 있다. Light나 ModelCue처럼 material equation이
없는 presentation은 `V1_TYPED_PRESENTATION_PRODUCT + NOT_APPLICABLE`로 닫는다. 중간의 typed family 성과를 V0로 되돌려
부르거나, 코드의 `STANDARD_COLOR_V1`을 전체 V1 architecture 이름으로 사용하지 않는다.
`USER_WAIVED`는 review가 닫혔다는 뜻일 뿐 visual fidelity PASS가 아니다. “완벽 복원”은 required
composition이 사용자의 `USER_APPROVED`를 받은 경우에만 선언한다.

마이그레이션 동안 세 경로는 공존한다.

- 마이그레이션 중인 element는 자기 V1 replacement가 승인될 때까지 FAMILY_LITE를 유지할 수 있다.
- channel/coverage 의미가 확정된 표준 Sprite/Decal/Trail은 StandardColorV1을 쓸 수 있다.
- 고유 equation은 RuntimeMaterialV2의 검증된 opcode/layout이 맞으면 이를 재사용한다.
- Glasshole처럼 packet 크기, scene input, sampler/register와 MRT 계약이 다르면 별도 typed layout과
  adapter가 필요하다.
- exact replacement를 stage한 뒤 오류가 나면 generic으로 조용히 fallback하지 않고 fail closed 한다.

목표는 모든 family를 하나의 mega packet에 넣는 것이 아니다. 목표는
`compiled program/layout ID + typed, fidelity-qualified descriptor + 공용 carrier adapter`이며, 같은 layout을 쓰는
program끼리 renderer code를 재사용하는 것이다.

V1은 현재 V0 문서를 제자리 변환하는 일만 뜻하지 않는다. V0 손튜닝 중 사각 카드, 잘못된 alpha/UV,
미지원 carrier 때문에 source element를 제거했다면 다음 3-way 복구가 필요하다.

```text
원본 source occurrence 전체
+ 현재 V0 user-tuned document
+ 복원된 V1 program/layout/descriptor
= Tool-only source restoration candidate
```

기존에 남아 있는 행은 사용자가 맞춘 transform, timing, attachment와 composition을 보존하고 material
계약만 선택적으로 바꾼다. 원본에는 있지만 V0에 없는 행은 family가 닫힌 뒤에만 stable source-derived
ID로 Tool Solo 후보를 다시 만든다. 사용자가 포함을 승인해야 Product에 삽입하며, 거부하거나 의도적으로
지운 행은 retirement/disabled receipt를 남긴다. 따라서 현재 live element 수는 V0 기준선이지 최종 V1
분모가 아니다. 기존 generated restoration ledger는 복원 분석 당시의 inventory evidence일 뿐 현재 Product
membership이나 runtime 정본이 아니다.

## 6a. 확정된 V1_COMPLETE 판정 기준 (2026-08-22 사용자 결정)

사용자가 완료 이름이 흔들리지 않도록 다음 두 이름을 분리하고, 판정은 `V1_COMPLETE`로만 한다.

```text
V1_COMPLETE
= 올바른 carrier
+ family별 RT0 Base HLSL
+ texture / channel / scalar / DynamicParameter 배선
+ blend / depth
+ attachment / timing
+ Effect Tool 편집·저장
+ 사용자 육안 승인
```

```text
NATIVE_PARITY   (별도 backlog, V1 완료 조건 아님)
= native VertexFactory / BasePass 선택
+ 원본 ShaderMap permutation과 exact child cooked variant
+ 원본 MRT 0/2/3/4/5 전체 의미
+ Distortion 이외의 scene feedback
+ hardware sampler 전수 parity와 raw VS spatial A/B
```

- `V1_COMPLETE` 판정에 `NATIVE_PARITY` 항목을 요구하지 않는다. 도화가 F도 typed RT0 semantic
  replay golden이지 native VF/pass 전체를 닫은 사례가 아니며, 그 수준이 기준선이다.
- 원본 evidence가 없는 element는 `PROJECT_RECONSTRUCTED`, parent 증거로 식을 재구성한 element는
  `BOUNDED_TRANSLATED`로 기록한다. 증거 없이 `SOURCE_EXACT`로 올리지 않는다.
- 13절의 `G0~G9` 표준 공정은 그대로 쓴다. 다만 `G2`의 DXBC/native wire와 `G5`의 raw VS A/B가
  아직 없는 family는 `NATIVE_PARITY` backlog로 남기고, parent material evidence
  (`effect-family-manifest.v1.json`의 texture/scalar parameter와 blend/twoSided/masked)로 RT0 Base를
  닫아 `V1_COMPLETE` 판정을 진행한다.
- 이 결정은 완료 기준을 낮추는 것이 아니라 도화가 F가 실제로 통과한 기준으로 되돌린 것이다.
  `NATIVE_PARITY`는 폐기가 아니라 별도 후속 연구 항목으로 유지한다.

## 7. 기존 translucent 경로가 하던 일과 부족했던 이유

기존 grouped translucent는 많은 SpriteParticle과 MeshParticle을 빠르게 보이게 한 중요한 기반이다.
그러나 family별 원본 식을 다음과 같은 공용 의미로 축약한다.

- 하나의 공용 UV scale/pan
- base texture와 공용 color
- base alpha, mask red, emissive/base luminance 순의 coverage 추정
- 공용 noise/dissolve
- 공용 additive/alpha state

이 경로에서는 grayscale DDS 자체가 색인지, alpha mask인지, dissolve인지, emissive 강도인지 알 수
없다. 같은 회색 값도 다음 배선에 따라 완전히 다른 pixel이 된다.

```text
gray -> coverage/clip       사각형 경계를 자름
gray -> radiance            회색 또는 tint된 빛이 됨
gray -> emissive multiplier bloom 전 밝기를 증폭
gray -> dissolve threshold  시간에 따라 소멸 경계를 만듦
gray -> UV/distortion       다른 texture를 휘게 함
```

따라서 `t0=Base`, `base.a=alpha`, `mask.r=coverage`를 전 family의 기본 공식으로 쓰면 검격은
사각형으로 보이고, glass는 색·굴절이 사라지며, dragon flow는 잘못된 위치에서 UV가 시작할 수
있다. Typed descriptor는 각 lane에 다음을 명시하고 source evidence가 모두 닫힌 occurrence에만
`SOURCE_EXACT` fidelity를 부여한다.

- texture register와 sampler register
- source channel
- sRGB 또는 linear sample 의미
- UV source, scale/pan/rotation/time seed
- CB group/lane과 default 값
- particle color/dynamic parameter 의미
- radiance, coverage, emissive, dissolve의 결합 순서

## 8. HLSL, runtime ABI, adapter의 관계

HLSL은 조리법이고 runtime ABI는 재료와 도구를 정확한 자리에 놓는 계약이다.

| 층 | 닫아야 하는 질문 |
|---|---|
| HLSL equation | 어떤 sample과 산술로 최종 radiance/coverage/distortion을 만드는가 |
| Constant ABI | 어느 scalar/vector가 b0/b1/b2의 어느 lane에 들어가는가 |
| Resource ABI | 어느 DDS/scene target이 어느 t register에 들어가는가 |
| Sampler ABI | filter, address U/V/W, mip/bias와 s register가 무엇인가 |
| VF ABI | ParticleColor, DynamicParameter, UV, tangent가 어느 semantic으로 오는가 |
| Pass ABI | 어떤 VS/PS 조합과 scene input을 쓰는가 |
| Output ABI | RT0/MRT, coverage/discard, metadata 출력을 어떻게 처리하는가 |
| Render state | blend, cull, depth/stencil을 어떻게 적용하고 복구하는가 |
| Runtime safety | stage 실패, missing lane, shader compile 실패 때 무엇을 rollback하는가 |

DXBC에서 식만 번역하고 `t0`에 관습적으로 base texture를 꽂으면 정확한 복원이 아니다. 반대로
descriptor가 있어도 program 식이 grouped approximation이면 source-exact가 아니다. 두 층과 adapter가
한 exact tuple로 닫혀야 한다.

현재 Product runtime이 JSON에서 임의의 HLSL source나 shader graph bytecode를 받아 즉석 컴파일하는
구조도 아니다. bounded typed backend는 등록된 `backend + opcode + Layout`을 C++가 packet으로 pack하고
기존 Mesh/Sprite/Decal/Trail carrier shader가 작은 evaluator 집합을 dispatch한다. SOURCE_EXACT 주경로는
원본 bytes/hash가 봉인된 cooked pixel DXBC를 `CreatePixelShader`로 여는 방식이다. 169 번역 HLSLI를
한 mega-switch에 넣지 않으며, HLSLI 하나를 include한 build-time 단일-program HLSL permutation은 원본
DXBC와 output/ABI 동등성을 별도 봉인하기 전까지 `BOUNDED_TRANSLATED`다. 어느 경로든 Product가 소비하는
Program/Layout은 build와 harness로 봉인된 ID여야 하며 JSON이 shader 경로나 macro를 저작하지 않는다.

### Typed V1 LIGHT 계약

V1 `detail.light`는 다음 profile을 기존 `CEffectPlayback -> Try_BuildEffectLightDesc ->
CPresentation_Manager::Add_TransientLight -> CLight` 경로로 소비한다. Effect Tool에서 밝기나 색을
수정해도 저장된 광원 종류를 유지한다.

| `profileId` | Engine 종류 | 추가 저장 입력 |
|---|---|---|
| `light.point.reconstructed.v1` | POINT | 없음 |
| `light.spot.reconstructed.v1` | SPOT | `direction`, `innerConeDegrees`, `outerConeDegrees` 필수 |
| `light.directional.reconstructed.v1` | DIRECTIONAL | `direction` 필수 |

`direction`은 유한한 영벡터가 아닌 **element local 방향**이다. 실제 element transform,
`SourceTransformTrack`, occurrence root를 합성한 basis로 한 번 변환하고 최종 Light descriptor에서
정규화한다. UE3 원본 전방·FRotator·cm 단위의 변환은 기존 source transform 경로가 소유하며,
이미 변환된 방향에 별도 90도 보정을 더하지 않는다. SPOT은 `0 <= inner <= outer < 90`과
`outer > 0`을 요구한다. 알 수 없는 profile이나 잘못된 방향·각도는 POINT로 대체하지 않고 거부한다.

모든 종류의 optional `specularIntensity`는 유한한 0 이상이며 생략 기본값은 0이다. 양수이면
최종 diffuse RGB, 즉 원본 밝기·색 곡선까지 평가한 광색에 이 배율을 곱해 specular RGB를 만든다.
원본 근거가 있는 광원만 명시적으로 켠다. 기본값 0은 기존 POINT의 specular 0과 저장 형식을
유지하며, 임의로 다른 문서나 맵 광원에 specular를 더하지 않는다. 광원 수신 범위와 shadow·light
shaft·named channel의 지원 여부는 이 profile 이름만으로 복원됐다고 간주하지 않는다.

Engine `LIGHT_DESC`의 x64 ABI는 108 bytes로 유지된다. Client의 `EFFECT_LIGHT_DETAIL_DESC`는
72 bytes, `EFFECT_EVALUATED_LIGHT`는 96 bytes이며 이들을 포함한 문서·프레임의 C++ 크기는
기존 POINT 전용 구조와 다르다. 헤더가 바뀌면 codec, playback, renderer, tool 및 검증 실행파일의
모든 소비자를 같은 헤더로 다시 컴파일한다. 새 probe만 컴파일한 뒤 예전 Client OBJ와 링크한
결과를 ABI·재생 검증으로 사용하지 않는다.

검증은 실제 codec 저장 왕복과 잘못된 입력 거부, 원본 시계의 위치·방향·RGB 곡선, occurrence
회전의 단일 적용, 되감기 재현을 포함한다. 기존 POINT는 새 필드를 생략한 문서와 동일한 평가
입력에서 최종 `LIGHT_DESC`를 이전 구현과 바이트 단위로 비교한다. 이 수치·호환성 검증은
GPU 표시나 사용자의 최종 화면 판정을 대신하지 않는다.

### Source Transform의 정적 Mesh와 재질 곡선

원본 `StaticMeshComponent`가 사용하는 native program은 generator의 `sourceTransformMesh`와
`ARTIST_PROGRAM_DESC::bSourceTransformMesh`로 승인한다. 이 경로는 `MESH`, 비활성 SourceRecipe,
실제 `sourceTransformTrack.nodes`, 하나의 meshModel을 요구하며 particle program과 구분한다.
원본 texture·scalar·vector·static switch와 native packet 계약도 기존 검사를 그대로 통과해야 한다.

Optional `sourceTransformTrack.materialParameterTracks`는 `name`, `kind`(`SCALAR`/`VECTOR`),
`keys`를 저장한다. key의 값과 tangent는 각각 1/3성분이며 기존 `constant`/`linear`/`cubic`
분포를 사용한다. staging은 등록된 native program의 정확한 이름·형·row/lane에만 연결하고,
누락·중복·미지원 program·비유한 값은 거부한다. draw는 기존 Frame 시계에 source origin을 한 번
더해 prepared packet의 복사본만 갱신한다. vector는 XYZ만 바꾸고 W를 유지하며 곡선 없는
element의 기존 packet은 바꾸지 않는다.

각 node의 optional `useQuaternionInterpolation`은 기본 false다. true는 원본 Matinee의
quaternion 보간 근거가 있는 node에만 기록하고, 기존 false node의 Euler 보간과 직렬화를 유지한다.
헤더 변경 뒤 모든 codec·playback·renderer 소비자는 같은 ABI로 다시 컴파일한다.

## 9. 새 HLSL과 새 adapter를 만드는 기준

| 실측 결과 | 구현 선택 |
|---|---|
| equation과 layout이 같고 texture/scalar만 다름 | 기존 HLSL·adapter 재사용, descriptor만 추가 |
| equation이 다르고 carrier/VF/pass/layout은 같음 | 같은 adapter에 새 HLSL program 또는 검증된 opcode 추가 |
| equation은 같지만 packet/register layout이 다름 | 새 typed layout 또는 compatible layout adapter 추가 |
| VF/pass/scene input/RT topology가 다름 | 새 technique/adapter 추가 |
| Sprite를 projected decal로 바꿔야 함 | LocalDecal carrier/projection과 decal VF/pass부터 복원 |
| source evidence가 아직 없음 | FAMILY_LITE 또는 PROJECT_RECONSTRUCTED로 명시하고 source-exact 표기 금지 |

Program identity에 skill ID, class 이름, 파일명 또는 occurrence ID를 넣지 않는다. Exact identity는
material/static permutation과 실제 VF/pass/program·binding closure가 소유한다. Occurrence ID는
어떤 verified descriptor를 소비하는지 선택할 뿐이다.

현재 공용 golden의 class-neutral 예시는 다음과 같다. Descriptor와 Binding만 occurrence/domain을
가질 수 있다.

```text
Program: effect.program.runtime-material-v2.opcode-6.v1
Layout : effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1
Descriptor: effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1
```

현재 compiled S6/M3/D14 allowlist는 `(backend, opcode)`마다 정확히 한 ABI receipt만 허용한다. 같은
Program에 두 번째 호환 Layout fingerprint가 필요해지면 C++/source validator/harness의 versioned receipt 집합을
먼저 확장한 뒤에만 public Layout을 추가한다. opcode는 backend 안에서 append-only로 배정하며 다른
세션이 번호를 수동 예약하거나 기존 번호를 재사용·재정렬하지 않는다.

## 10. 도화가 F가 증명한 것

도화가 F는 이 계획의 `V1_GOLDEN_CONTROL`이다. 성공 요인은 “전용 shader 파일이 있었다” 하나가 아니다.

1. stable occurrence를 식별했다.
2. 원본 texture와 material 역할을 연결했다.
3. ShaderMap/DXBC와 VF/pass 후보를 조사했다.
4. exact 또는 bounded equation을 읽을 수 있는 HLSL로 만들었다.
5. occurrence별 strict registry와 typed opcode를 만들었다.
6. source recipe, carrier, attachment, timeline을 함께 연결했다.
7. 사용자가 실제 화면에서 손튜닝하고 결과를 높은 품질로 평가했다.

현재 Product cue는 legacy `effect.artist.skill.31470`이 아니라
`effect.artist.skill.31470.unified`를 가리킨다. 이 문서는 17 elements이고 particle 14, decal 2,
trail 1로 구성된다. legacy evidence registry의 35행과 현재 Product 17행을 같은 분모로 세지 않는다.
과거 35행 기준으로 semantic replay는 7, bounded explicit replay는 22였고 native selection
admission은 0이었다. 사용자 평가는 중요한 visual evidence지만 formal source-exact gate와 같은
뜻은 아니다. 즉 도화가 F는 높은 품질과 재사용 가능한 공정을 증명했지만, 35행 전부의 native
ABI를 완전히 복구했다는 뜻은 아니다.

그러므로 도화가 F를 일반화한다는 것은 스킬마다 C++ renderer와 HLSL 묶음을 복사하는 것이 아니라,
그때 성공한 `occurrence registry + typed equation + exact resource role + carrier/attachment + user tuning`
공정을 family registry와 shared adapter로 승격하는 것이다.

관련 증거:

- [Artist 31470 F 원본 복원 결과](../GB/08-13/2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_RESULT.md)
- [Artist 31470 F V5 family/transform/material 결과](../GB/08-13/2026-08-13_ARTIST_31470_F_V5_FAMILY_TRANSFORM_MATERIAL_RESULT.md)
- [Artist 31470 F Track A ShaderMap/DXBC runtime 결과](../GB/08-10/2026-08-13_ARTIST_31470_F_TRACK_A_MAIN_SHADERMAP_DXBC_RUNTIME_RESULT.md)

## 11. Glasshole02가 증명한 것과 아직 남은 것

Glasshole02는 차원술사 W의 기존 occurrence
`authored.source-particle.40e1b48e2f0f88dcfeff1549` 하나를 exact canary로 선택했다. 저장 문서에
별도 solo element를 만든 것이 아니다. Tool의 Solo Element는 기존 문서를 바꾸지 않는 preview
filter이며, translated canary는 이 stable occurrence에만 default-off로 replacement된다.

현재 Glasshole02 경로는 다음 층을 실제 코드로 연결했다.

- 읽을 수 있는 translated equation: `Shader_Ue3Glasshole02.hlsli`
- material constants `b0[22]`, scene constants `b2[4]`
- source DDS 7개와 engine scene depth 1개, `t0..t7`
- sampler slot `s0..s7`
- particle carrier varying을 원본 PS input으로 바꾸는 runtime shader
- alpha/two-sided/depth-read state
- exact effect+occurrence gate, stage/rollback, Tool default-off canary

이것이 과거의 `DXBC -> equation -> ????? -> Winters renderer`에서 `?????`를 처음 구체적인 packet과
adapter로 닫은 사례다.

그러나 아직 다음을 완료로 쓰지 않는다.

- source-exact hardware sampler 7/7
- raw source VS와 현재 adapter의 full numeric/spatial A/B
- 원본 MRT 0/2/3/4/5 전체 의미와 현재 RT0 runtime의 동등성
- Product runtime admission
- 사용자 visual admission

따라서 Glasshole02는 `SOURCE_EXACT 후보 Tool canary`이지, 이미 Product에 완전 복원된 Glass family가
아니다. 하나의 canary가 같은 coarse sprite+MRT bucket 전체를 자동으로 exact하게 만들지도 않는다.
공용 scheduling skeleton은 재사용할 수 있지만 family별 program, CB 크기, SRV wire, sampler와
descriptor tuple은 각각 증명해야 한다.

관련 증거:

- [Missing Effect Family ABI 복원 계획](../GB/08-22/2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_IMPLEMENTATION_PLAN.md)
- [Missing Effect Family ABI 복원 결과](../GB/08-22/2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_RESULT.md)

### Valtan Effect V2 source와 게시된 binding의 분리

Composition binding 저작 정본은 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`이다.
Source Save는 이 owner의 exact baseline CAS와 storage schema만 확정하며 제품 재생을 활성화하지 않는다.
엄격한 Valtan Publish가 검증한 같은 형식의 생성물은
`Data/Valtan/Published/BOSS_VALTAN.effectv2bindings.json`에 둔다. `CEffectV2Catalog`의 authoring snapshot과
runtime snapshot은 독립이며 제품 `CEffectV2Runtime::Ensure_Bindings`는 게시된 snapshot만 소비한다.
명시적 local preview가 주입하는 authoring snapshot은 계속 현재 draft를 재생한다.

Sound도 `Data/Valtan/Published/Valtan.patternsoundcues.json`을 제품 입력으로 사용한다.
Source Reload/Save 또는 dirty snapshot 무효화를 Product admission으로 간주하지 않는다.
게시 오류·Product parse 오류는 이전 admitted snapshot을 유지하며 생성물을 source로 역복사하지 않는다.

## 12. Valtan에 적용할 때의 현재 경계

이 가이드의 초기 main 기준선 `61930d4a`에는 Valtan V0 graph와 첫 V1 연구 수직 슬라이스가 함께
있다. V0 graph의 material fidelity는 주로 FAMILY_LITE다. PR #145로 Ground Decal, Masked Dissolve,
Crack Translucent 세 family·9 occurrence의 bounded RT0 Tool canary가 병합됐지만 기본 OFF 비영구
preview다. Product, actual VF/pass, exact sampler, scene CB, MRT/coverage와 visual admission은 모두
false이므로 V1 Product coverage는 아직 `0/3,683`이다.

따라서 다음 방향은 맞다.

- ground decal은 LocalDecal carrier/projection과 verified, fidelity-qualified decal material
  program/descriptor를 함께 복원한다.
- 나머지 missing Sprite/Mesh family는 stable occurrence를 하나씩 canary로 골라 program과 ABI를 닫는다.
- 동일한 program/layout/descriptor closure가 구조적으로 증명된 cohort에만 registry row로 확장한다.

그러나 “발탄 ground decal과 다른 두 family용 HLSL 세 개를 반드시 새로 만든다”라고 미리 고정하지
않는다. 실측 후 기존 program/layout을 재사용할 수 있으면 descriptor만 추가하고, equation이 다르면
새 HLSL을, carrier/VF/pass가 다르면 새 adapter를 추가한다. Draw 제출과 rollback proof는 중요한 자동
증거지만 material exact나 사용자 visual PASS를 대신하지 않는다.

관련 증거:

- [Valtan Effect Family Completion 결과](../GB/08-21/2026-08-21_VALTAN_EFFECT_FAMILY_COMPLETION_RESULT.md)

## 13. source-exact 복원 표준 공정

### G0. 대상과 occurrence 고정

- 실제 제품 cue/effect join을 확인한다.
- 현재 Product 문서의 stable element ID를 고정한다.
- 원본 emitter/module과 현재 element의 수를 별도로 기록한다.
- carrier, texture, mesh, timeline, attachment를 element 표로 만든다.

### G1. child/parent/permutation 선택

- child MIC override를 수집한다.
- parent Material GUID를 연결한다.
- effective static parameter set과 platform을 확정한다.
- 이름이 같다는 이유만으로 다른 ShaderMap을 선택하지 않는다.

### G2. ShaderMap, DXBC, native wire

- 실제 VF/pass program을 선택한다.
- VS/PS signature와 DXBC identity를 봉인한다.
- native binding wire로 CB, SRV, sampler register를 연결한다.
- texture expression 나열 순서를 register 순서로 간주하지 않는다.

### G3. descriptor closure

- texture bytes와 color space를 검증한다.
- scalar/vector default와 CB AST/lane을 확정한다.
- sampler filter/address/mip/bias를 확정한다.
- ParticleColor/DynamicParameter와 UV/time semantic을 확정한다.

### G4. renderer closure

- actual carrier/VF/pass를 확정한다.
- scene depth/color와 RT/MRT를 연결한다.
- blend/cull/depth/coverage를 연결한다.
- stage 실패와 state leak에 대한 rollback을 검증한다.

### G5. program translation과 parity

- DXBC 연산을 typed HLSL로 번역한다.
- WARP 또는 focused harness로 수치 A/B를 수행한다.
- UV, depth, camera, time, dynamic parameter를 바꾼 spatial A/B를 수행한다.
- translation receipt와 source oracle identity를 함께 보존한다.

### G6. 한 occurrence canary

- 기존 occurrence 하나만 exact tuple에 연결한다.
- Tool default-off, fail-closed replacement로 stage한다.
- Debug/Release build와 first-draw 수치 증거를 닫는다.
- canary를 Product 전체에 바로 확장하지 않는다.

### G7. 사용자 화면 검증

- 사용자가 Tool에서 ordinary/translated A/B를 직접 본다.
- 경계, 색, UV 흐름, timing, depth, camera 의존 결함을 occurrence 단위로 기록한다.
- 사용자의 서면 판정 전에는 visual PASS로 올리지 않는다.

### G8. Product admission

- `EffectCatalog.json`의 exact source row와 authored v15 `runtimeExtensions.runtimeCarriers`에 typed
  program/layout/descriptor identity와 별도의 fidelity status를 등록한다.
- `Validate-EffectSources.ps1`, Save transaction의 next-spawn 교체와 다음 Client source reload를 검증한다.

### G9. verified cohort 확장

- program/layout/descriptor closure의 구조적 동등성이 증명된 occurrence에만 데이터로 확장한다.
- 같은 closure를 재사용해도 fidelity는 occurrence별 source/reconstruction evidence로 유지한다.
- 다른 CB/SRV/sampler/VF/pass tuple은 새 canary로 남긴다.

## 14. 완료 증거 사다리

| 단계 | 완료 증거 | 아직 의미하지 않는 것 |
|---|---|---|
| element inventory | cue/effect/occurrence/carrier join | pixel 정확성 |
| texture parity | source/runtime DDS byte identity | channel·sampler 정확성 |
| DXBC 확보 | exact program identity와 bytecode | runtime에서 같은 출력 |
| translated equation | structural/numeric replay | 올바른 packet과 carrier |
| ABI descriptor | CB/SRV/sampler/VF/pass/state closure | 사용자 화면 승인 |
| Tool canary | stable occurrence의 typed fail-closed draw | Product 전체 admission |
| Product admission | source catalog와 다음 spawn/다음 Client가 같은 typed selector 소비 | source-exact fidelity 또는 다른 family의 정확성 |
| 사용자 승인 | 대상 화면의 visual fidelity 판정 | 증거가 다른 occurrence로 자동 전파됨 |

`첫 pixel`, `draw admitted`, `texture 7/7`, `DXBC translated`를 각각 복원 완료와 혼동하지 않는다.

## 15. 현재 코드·데이터 소유권

| 정본 | 소유 내용 |
|---|---|
| `Data/Effects/Authored/*.effect.json` | stable element, composition, carrier/resource/material 선택과 v15 inline runtime carrier/history |
| `Data/Effects/Contracts`와 Imported receipts | source identity, exact variant, evidence와 admission 상태 |
| `Data/Effects/EffectCatalog.json` | Product EffectAssetId와 exact authored 상대 경로 admission |
| `Data/Effects/EffectResourceTree.json` | 저작 라이브러리 분류·표시 이름. runtime 보스·관문 선택이나 Product admission을 소유하지 않음 |
| `Client/Bin/Resources/Effect`와 필요한 Character model | Product 문서가 참조하는 Drive-owned DDS/WModel binary; Git 추적 금지 |
| `Effect_AuthoringDocument.h` | element, renderer, resource, material execution descriptor schema |
| `Effect_VisualProgramCorpus.*` | 같은 authored document pointer에서 만드는 transient `ADAPTER_PACKET_V1` projection. disk corpus를 읽거나 쓰지 않음 |
| `Effect_MaterialTemplate.h` | profile ID, typed constant/packet 구성 계약 |
| `Shader_EffectCommon.hlsli` | compatibility/grouped 공용 계산 |
| `Shader_EffectStandardColorV1.hlsli` | 명시적 radiance/coverage/dissolve 표준 ABI |
| `Shader_EffectUe3MaterialFamilies.hlsli` | 검증된 RuntimeMaterialV2 fixed opcode dispatch |
| family별 translated `.hlsli` | exact 또는 bounded material equation |
| carrier/technique `.hlsl` | VF input, pass, RT와 render state 연결 |
| `Effect_DocumentRenderer.*` | packet stage, adapter scheduling, draw, rollback, diagnostics |
| Effect Tool | occurrence 선택, solo/filter, explicit canary와 authored Save/next-spawn transaction |

Generated runtime Effect 문서는 존재하지 않는다. authored 문서와 source catalog를 직접 고치고
`Validate-EffectSources.ps1`로 검증한다. Save activation 실패는 compare-and-swap으로 authored 파일과
prepared target을 이전 상태로 되돌린다.

Kouku 저작 브라우저는 Catalog와 실제 Authored 헤더, 저장된 Tree 참조를 함께 표시한다.
Catalog 미등록 원본도 canonical 저작 경로에서 열 수 있으나 이 탐색을 admission으로 취급하지 않는다.
Play All의 보스는 sourceModelPreview 또는 실제 Composition 사용 관계로 선택하고, 공유 원본의
모호한 문맥은 사용자의 명시적인 Model View 선택으로 해결한다. Tree의 첫 분류를 actor로 해석하지 않는다.

현재 contract 파일의 역할도 구분한다.

| 현재 contract | 현재 역할과 한계 |
|---|---|
| `effect-family-manifest.v1.json` | discovery/inventory와 blocker 집계. runtime admission 정본이 아님 |
| `ue3-material-family-registry.v1.json` | profile/carrier/render profile/texture role을 가진 thin registry. 아직 full runtime ABI descriptor가 아님 |
| `ue3-exact-cooked-shader-variants.v1.json` | exact program candidate와 evidence 저장소. sampler·raw VS·Product 상태가 남을 수 있음 |
| occurrence별 canary receipt | 한 stable occurrence의 exact/candidate 경계와 admission 상태 봉인 |

향후 admitted runtime descriptor registry는 shader asset/permutation, CB AST/lane, native t/s wire,
sampler, VF/pass, scene input, RT와 state를 빠짐없이 소유해야 한다. 기존 thin registry에 이 의미를
소급해서 부여하지 않는다.

## 16. 다음 진행 방향

1. Glasshole02의 sampler, VS/spatial A/B, RT/MRT 경계와 사용자 Tool A/B를 닫는다.
2. Glass 전용 hard-coded packet 선택을 `program/layout ID + descriptor registry`로 일반화한다.
3. 같은 adapter skeleton을 쓰되 exact tuple이 다른 mesh MRT 대표 family 하나를 canary로 닫는다.
4. occurrence 수가 많은 sprite RT0와 mesh RT0 대표를 각각 하나씩 닫는다.
5. LocalDecal projector/VF/pass와 material equation을 분리해 Valtan ground decal, 워로드 Full Barrel,
   도화가 R 같은 cohort에 재사용한다.
6. Dragon/MakeFlow는 UV/time/dynamic parameter와 mesh carrier를 먼저 닫고, CModel 용 animation과
   분리한다.
7. Screen shard, attractor, scene feedback은 material family만으로 해결되지 않는 presentation
   capability이므로 별도 carrier/adapter 수직 슬라이스로 진행한다.
8. 각 canary가 사용자 승인을 받은 뒤 program/layout/descriptor의 구조적 동등성이 증명된 cohort만
   catalog에 확장하고, fidelity는 occurrence별 evidence로 유지한다.
9. family 하나가 닫힐 때마다 현재 V0 matched 행과 함께, 그 family 문제 때문에 과거 삭제된 source
   occurrence를 Tool-only 후보로 복구해 사용자가 Product 포함 여부를 결정한다.

이 순서는 family 수를 억지로 줄이는 것이 아니라, renderer scheduling은 공용화하고 원본 program과
descriptor의 차이는 데이터와 translated HLSL로 보존하는 방향이다.

## 17. 금지할 지름길

- texture만 바꿔 Sprite를 LocalDecal로 간주하지 않는다.
- `t0=Base`, `t1=Normal`, `t2=Mask`를 근거 없이 가정하지 않는다.
- grayscale DDS를 자동으로 alpha나 color로 판정하지 않는다.
- `Translucent`, `Additive`, `Masked` suffix만으로 family를 정하지 않는다.
- parent 이름 하나나 coarse adapter bucket을 exact ABI라고 부르지 않는다.
- 모든 family를 한 mega packet에 억지로 넣지 않는다.
- skill/class/filename switch로 renderer를 복제하지 않는다.
- draw proof나 자동 screenshot을 사용자 visual PASS로 승격하지 않는다.
- canary candidate sampler/default를 source-exact 값이라고 기록하지 않는다.
- exact replacement stage가 실패했을 때 generic fallback으로 결함을 숨기지 않는다.

## 18. 관련 구현 계획

- [4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획](../GB/08-22/2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)
- [Character Source-Exact Effect Conquest Master 계획](../GB/08-21/2026-08-21_CHARACTER_SOURCE_EXACT_EFFECT_CONQUEST_MASTER_IMPLEMENTATION_PLAN.md)
- [Animation/Effect/Character Preview Tool 인계](ANIMATION_TOOL_OWNER_HANDOFF.md)

이 문서의 구조나 admission 기준이 바뀌면 이 문서를 갱신한다. 개별 family의 명령 로그, hash,
일시적인 실패와 실행 결과는 해당 날짜의 PLAN/RESULT에만 기록한다.


### 고정 장면 캡처 프로필의 현재 저작 계약 (2026-09-14)

`detail.screenPost.profileId=screen.scene-collapse.capture.v1`은 시작 시점의 resolved HDR/bloom pair를 고정하고 바깥을 검정으로 그린다. optional `captureShrinkSeconds`의0은 기존 Timing 수명을 사용하며, 양수는 Timing 수명 이하의 수축 구간이다. 나머지 구간은 끝 상태를 유지한다.

`screen.scene-capture.cube.v1`은 live scene 위에서 고정 이미지를 축소하며 `captureTargetModelCueId`가 필수다. target은 같은 문서의 visible ModelCue이고 ScreenPost 끝은 그 cue 시작과 일치해야 한다. 현재 endpoint는 실제 설치 CModel의 첫 pose bounds만 지원한다. 다른 profile의 target ID, 잘못된 끝 시간과 잘못된 수축 구간은 Codec이 거절한다. Tool profile 전환은 이전 target과 별도 수축 시간을 정리하며 저장·재로드가 같은 계약을 사용한다. 원본 SourceMaterial ID와 renderer carrier는 새 profile ID로 위장하지 않는다.

### ModelCue의 선택적 skeletal shadow

`modelCues[].castsShadow`는 optional bool이며 누락 시 false다. opaque/masked CModel surface만 허용하고 afterimage와 별도 Effect material/translucent carrier는 거부한다. codec 저장·재로드와 prepared-resource 비교가 이 값을 보존한다. visible cue의 유효 시간에만 기존 SHADOW group으로 제출하며, surface와 shadow는 동일 `Sample_ModelCuePose`, occurrence RootWorld, source material parameter track, CModel 본/재질을 소비한다. ModelCue 전용 animated pass15는 기존0..14를 이동하지 않고 마지막에 추가되며 기존 캐릭터 shadow pass1은 바꾸지 않는다.

shadow raster 위치는 별도 light View/Proj로 계산하지만 native coverage의 View/Proj와 camera position은 실제 scene camera를 사용한다. source mask/dead dissolve를 diffuse alpha 임계값으로 대체하지 않는다. 현재 opt-in 대상은 Guardian ALT V 원본49420 notify024의 용 모델5개 material-part cue다. 원본 EF skeletal component의 generic CDO가 shadow=true를 상속한다는 데이터와 override 필드 부재를 근거로 선택했으며, 원본 native actor 생성 코드 전체를 확인한 것으로 주장하지 않는다. 후속 projectile의 DragonDecal은 별도 source occurrence다.

### Presentation provider의 실패 격리와 texture coverage

`Submit_FrameProviders`에서 provider가 `LOCAL_PROVIDER_CONTRACT`와 isolated failure를
명시하면 그 provider가 이번 제출에 추가한 light/post/overlay 및 channel 통계만 rollback한다.
실패 provider는 `Finalize_PresentationSubmission(false)`를 한 번 받고, 다른 정상 provider는
채널 검증 후 `true`를 받는다. frame의 `bCommitted=true`와 `iProviderFailures>0`는 함께
가능하며 반환 `S_FALSE`는 일부 provider 격리를 표시한다. 전역 실패·budget 초과·불명확한
failure scope는 전체 frame rollback과 실패 HRESULT를 유지한다.

Overlay coverage 검증은 실제 GPU SRV 형식을 기준으로 한다. BC1/BC1_SRGB는 불투명 또는
1-bit alpha를 공급하므로 A coverage를 지원한다. 색 공간 일치, 다른 형식의 channel 존재,
양수 footprint와 alpha 범위 검사는 그대로 유지한다. 정상 capture의 소비 여부는 다른
provider와 함께 제출한 프레임으로 검증한다.


### External Composition capture window

The existing `CEffectPresentationService::Seek_WorldRoot` accepts an optional playback end age in seconds (zero preserves the authored duration). Only externally sampled occurrences may supply a positive end. Pending spawns retain it in their descriptor; active objects forward it to their renderer. Scene-collapse clamps its authored shrink interval to the remaining owning box time. Frozen Color/Bloom and authored particle clocks remain unchanged; cube/model-cue capture keeps its original contract. Both ordinary sequence sampling and live geometry sampling supply the owning box duration. Non-finite/negative values fail without mutating the occurrence.


### Native Sprite의 양면 저작 옵션

Authored `detail.sprite.twoSided`는 기본false인 carrier 옵션이다. 현재 Artist registry의 검증된 native SourceRecipe Sprite에 한해 Alpha/Additive One Sided의 blend·depth를 유지하고 양면 pass로 그린다. 원본 material.renderProfile과 native program identity는 변경하지 않으며 다른 carrier·compiled adapter·source-contract 문서에는 허용하지 않는다. 원본의 two-sided 근거로 취급하지 않고 사용자 저작 변경으로 구분한다. false는 JSON에서 생략한다.


### 화면 왜곡 MRT의 수신 표면 보호

`Target_Distortion`의 RGBA16_FLOAT는 signed 화면 UV offset 두 쌍을 누적한다. RG는 기존 일반 왜곡이고 BA는 actor와 깊이 경계를 보호하는 왜곡이다. 현재 BA writer는 검토한 쿠크 native2461/2587/3682뿐이다. native source 식을 바꾸거나 JSON renderProfile을 새 값으로 위장하지 않는다. installer의 명시 routing도 같은 범위를 유지한다.

V1 공통 Opaque/Alpha/Additive의 RT1은 RGBA 전체에 One+One을 적용한다. SceneColor와 Bloom의 원래 blend는 유지하며 RingFill/LinearReveal coverage와 distortionScale은 offset 네 채널에 적용한다. 일반 writer의 BA=0은 이미 누적한 BA를 지우지 않는다. compiled material adapter의 fixed-function 검사도 동일 RGBA write mask를 요구한다. V2의 별도 기존 RG 경로는 유지한다.

Engine SceneResolve가 Depth와 PickPos를 명시적으로 바인딩하고 BA를 소비한다. depth marker0/5의 skinned bit8 또는 marker5의 source skin/equipment program1~24,26~29만 actor로 해석한다. 다른 map marker payload를 actor ID로 읽지 않는다. 현재 픽셀과 RG 적용 전후·BA를 합친 bilinear 샘플의 실제 footprint를 확인해 actor/깊이 불연속을 넘는 BA를 차단한다. 경사면의 NDC 깊이 기울기는 허용하고 HDR와 가중 Bloom은 같은 UV를 쓴다. BA=0이면 기존 RG resolve와 동일하다.

이 계약은 불투명 G-buffer에 기록된 actor와 표면 경계를 보호한다. 동일 native map program을 쓰는 unskinned prop 내부와 G-buffer를 쓰지 않는 투명 객체를 종류별로 완전히 제외하는 계약은 아니다. 설치된 Engine/Client 셰이더와 바인딩 코드는 함께 갱신해야 하며 소스 반영·GPU 수치 검증·제품 빌드·사용자 화면 판정을 구분한다. 개별 검증 근거는 09-14 Sequence Implementation RESULT G50에 둔다.

### Native Trail의 coverage와 거리 UV

연결된 정점·인덱스만으로 재질의 연속성을 판정하지 않는다. 원본 vertex factory의 전달 성분과
최종 pixel program의 실제 소비 성분을 함께 확인한다. 현재 검토된 positive-tiling adapter는
최종 업로드한 Trail의 U 시작·끝에서 정규화한 coverage UV와 기존 거리/tiling UV를 구분한다.
native2379는 xy에 coverage, zw에 거리 UV를 전달한다. native2346/2836/3007은 xy에 거리,
zw에 coverage를 전달한다. 이 대응은 회수한 pixel program 소비식에 따른 adapter이며,
회수하지 못한 원본 UE3 CPU의 vertex-buffer packing을 복원했다고 주장하지 않는다.

Engine의 VTXEFFECT_TRAIL 저장 구조는 유지한다. CPU는 매 draw에 g_TrailSourceUVTransform을
다시 바인딩하며, StandardColor·RuntimeV2·다른 native profile의 UV 의미를 바꾸지 않는다.
명시 tiling=0의 geometry/VS UV 계약은 유지하지만, native2346의 최종 color/distortion
consumer가 UV1을 버리던 결함의 수정까지 bit-identical하다는 뜻은 아니다. 오래된 CSO의
필수 uniform 누락을 무시하지 않으며 C++와 Trail CSO는 같은 빌드에서 갱신해야 한다.

### 명시적으로 저작한 Sprite 공통 원형 마스크

optional detail.sprite.ownerRadialMask는 enabled, centerXZ, radius, feather를 저장한다.
기본값은 꺼짐이며 생략한다. 현재 허용 범위는 native3171 source-particle Sprite의
기존 one-sided alpha/depth-read carrier다. 다른 carrier·mesh model·action-cue attachment와
지원하지 않는 backend는 검증에서 거절한다. 원본 shader 기능으로 분류하지 않는다.

마스크 기준은 각 입자 중심이 아니라 effect 원점 XZ다. ParticleSystem uniform scale,
yawOffset, Frame.RootWorld를 한 번 합성한 역행렬로 world position을 변환한다. 개별 입자의
StartSize/detail scale로 공통 원이 커지지 않는다. 비유한·비affine·특이 변환은 거절한다.
coverage는 native 계산 뒤 RT0 alpha와 RT1에 적용하고, 그 결과로 Bloom을 만든다.
straight RGB는 유지하며 지원하는 carrier의 매 draw에서 enabled를 초기화한다.
수치 검증·설치·사용자 화면 판정의 증거는 09-17 세이튼 카드 RESULT G29에 둔다.

### V1 ScreenPost의 수동 강도 보간과 색수차

V1 `detail.screenPost`는 `screen.chromatic-aberration.reconstructed.v1`을 기존 Engine `CHROMATIC_ABERRATION_RECONSTRUCTED` 프로필로 제출한다. 기존 프로필 번호는 유지한다. optional `intensityLerp`의 기본값은 false, `intensityEnd`는 0이다. 보간을 켜면 `intensity`에서 `intensityEnd`까지 기존 Timing lifetime의 정규화 시간으로 선형 보간한다. 종료 강도는 유한한 비음수여야 한다. 기존 source dynamic intensity가 있으면 그 값이 우선하고 alpha-over-life는 기존대로 곱한다. 필드가 없는 문서의 재생·직렬화는 유지한다.

V2의 화면 효과를 V1으로 옮길 때 프로필 이름만 복사하지 않는다. 실제 parser, lifetime, 강도 곡선, Engine 제출 프로필을 함께 연결하고 원래 요소와 occurrence를 보존한다. 현재 추가 계약은 단순 선형 보간이며 V2의 keyed/smoothstep 곡선 전체를 이식한 계약은 아니다.

### CubeSample의 optional project clarity

`effect.ue3.cubesample-01-scene.v1`은 기존 sourceProfile.scalars의 `project_clarity_strength`를 선택적으로 소비한다. 생략/0은 기존 shader와 alpha/clip을 유지하며 값은 유한한 0~1이다. codec 공통 material 검증과 staging이 범위를 검사한다. 기존 `g_SourceScalars0.w`를 사용하므로 CBuffer/schema/profile ID와 carrier ABI는 바뀌지 않는다. 이 필드는 원본 MIC에서 회수한 값이 아닌 프로젝트 저작 보정이다.

활성화한 재질만 배경 transmission을 제한하고, 원본 particle의 정규화 chroma를 흡수와 자체 면 발광에 적용한다. 원래 caustic·edge는 유지하며 자체 발광을 배경 highlight 제한에 넣지 않는다. 밝은 배경의 중간 alpha coverage를 보정하되 fade0/1과 원본 clip을 유지한다. 자체 발광은 SceneColor/SceneBloom/black 평가에 동일하게 들어가므로 기존 per-effect Bloom 추출을 소비한다. 배경 전달의 gain·alpha도 세 평가에서 고정하며 억제된 배경 Bloom을 새로 만들지 않는다. 강도1의 보정분은 입력 SceneBloom을 선형으로 전달한다. 연결 범위는 차원술사 Q 두 저작 문서의 해당 CubeSample 요소이며 다른 동일 재질은 기본0이다. 실제 Q 색·원본 텍스처·현재 tone을 포함한 수치 검증과 사용자 화면 판정은 별도다.

### Native ScreenPost source material curves

Admitted Artist/Kouku SCREEN_POST carrier도 기존 SourceTransformTrack의 materialParameterTracks binding을 검증한다. Build_NativeScreenPost는 문서 sample time과 source time origin으로 곡선을 평가한 parameter array를 snapshot에 복사한다. 정적 material 배열만 전달하면 원본 opacity/type 변화가 유실된다. Full-screen post의 LocalVF UV0와 viewport aspect를 실제 원본 VS/PS 식별자로 검토하며 일반 Particle/mesh 프로그램에 보정을 퍼뜨리지 않는다.

### ModelCue optional castsShadow

ModelCue의 optional boolean `castsShadow` 기본값은false다. true는 기존 CModel의 opaque/masked
surface에만 허용하며 afterimage, Effect-native material, translucent surface는 validation에서
거부한다. 같은 evaluated frame의 pose/root와 source material tracks를 surface/shadow가 공유한다.
animated mesh pass15는 기존0~14의 인덱스를 보존한다. native coverage에는 scene View/Proj와
camera를 유지하고 SV_POSITION에만 별도 light View/Proj를 사용한다. 별도 원본 decal element는
이 모델 shadow 정책으로 대체하지 않는다. legacy default·malformed type·compiled cohort의
pass/입력 계약과 최종 화면 판정은 서로 구분해 검증한다.


### 정적 ambient의 화면 밖 시계 정책

Map world placement의 LEVEL_ACTIVE + SOURCE_LOOP만 offscreen pause를 요청할 수 있다.
서비스는 외부 owner/anchor/control이 없고 전체 source sprite bound를 증명한 occurrence에 한해
최종 camera에서 simulation과 draw 제출을 생략한다. particle/RNG 상태는 보존하지만 hidden wall
time은 누적하지 않으므로 재진입 시 연속 재생과 ambient 위상이 달라질 수 있다. 기존 draw-distance
Stop/respawn, Server lifecycle, externally sampled trigger marker의 시간 계약은 유지한다.

bound helper는 stationary affine root와 rigid camera를 요구한다. root 변이·non-rigid camera·
미지원 source module·mesh/trail/light/screen/control은 fail-open한다. 새 source 기능이나 sprite VS
변형을 추가할 때 이 admission을 함께 검토하고 모르는 변형을 기존 sphere에 조용히 포함하지 않는다.
상세 지원 범위와 검증 증거는 [공통 최적화 RESULT](../GB/09-22/2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_RESULT.md)의 G07에 둔다.


### Portable movie particle의 타일·충돌·이벤트

기존 authored particle carrier는 SubUVSelect의 relative-age vector X/Y tile 선택과 Collision의
FreezeRotation 완료를 소비한다. atlas dimensions는 양의 정수, SubImageSelect는 3성분을 요구한다.
FreezeRotation은 이동·수명을 보존하고 후속 sprite/mesh 회전과 collision만 정지한다.
수신자 없는 원본 generator는 bounded local no-op으로 보존하며 연결된 cycle과 queue 초과는
계속 거부한다. codec와 playback 모두 sourceMaterialSlots를 포함한 element 실행 판정을 쓴다.
실제 원본·PhysX·다섯 무비 증거는 [무비 RESULT](../GB/09-25/2026-09-25_FOUR_CLASS_SELECTION_MOVIES_IMPLEMENTATION_RESULT.md)의 G05/G07을 따른다.

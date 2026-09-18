# 발탄·쿠크 Trail/Ribbon 원본 입력과 렌더링 복원 결과

## G00. 반영 범위와 실행 경계

확인된 누락은 native2379의 UV1, baked AnimationTrail의 원본 TilingDistance,
CascadeRibbon SpawnPerUnit의 출생 위치와 loop 거리 나머지, primitive Cylinder의 부호 제한이다.
원본 UPK/DXBC와 실제 설치 모델 경로를 대조해 기존 소비자를 수정했다. 100배 tick·전역 emissive
증가·원형 검정 sprite 강제 변경은 적용하지 않았다. 두 CPP의 필요한 컴파일과 Trail 전체 FXC
컴파일은 격리 산출물로 검증했다. Client/Server EXE와 설치 CSO는 이번 작업에서 교체하지 않았다.
Client 31396과 Server 34628은 사용자 실행 상태로 관찰됐으며 종료·조작하지 않았다.

사용자가 편집한 피자 19요소와 다른 라이브 Data는 쓰지 않았다. 등록을 기다리는 새 이펙트는
이번 변경에 없다. 다음 사용자 빌드와 재시작에서 수정된 runtime/shader를 소비해야 한다.
화면의 최종 두께·밝기·형상은 사용자 검증 전이며 원작 전체 시각 일치로 기록하지 않는다.

## G01. 발탄 native2379의 누락된 UV1 입력

Full Restore420609 stage008의3개와 stage009의1개 AnimationTrail은 native2379를 사용한다. 원본 material PS `34c7dac3b50dcd40b8c18be67bbf7d5a`, distortion PS `602579e9d74c2744acfda3bca9b987bb`의 TEXCOORD0.zw는 emissive/noise/dissolve에 쓰인다. 각각의 source VS `f6b274c2c28e4b45b0c2762be4e095fb`/`91ccb94877dac34e988dd1d7bf625e2c`는 네 UV 성분을 그대로 전달하지만, 현재2379 adapter는 zw를0으로 채웠다.

`Shader_EffectKoukuNativeGroup2368.hlsli`의 두 입력만 `float4(input.uv,input.uv1)`로 바꿨다. generator의 검증된 PS/VS allowlist에도 AnimationTrail 두 pass를 추가했다. 원본PS 연산, MIC scalar/vector/texture, ParticleColor/DynamicParameter, emissive·alpha·blend·HDR 정책은 바꾸지 않았다. 원본 source 재질이 없는 상태가 아니었다.

실제 원본PS DXBC와 현재/수정 함수에 같은 source DDS·MIC parameter packet과 대표 particle 입력을 공급한 격리 WARP64×64 비교9조건(3개 시간·색·dynamic × UV1길이1/4/100)에서 수정 후 RT0의 모든 성분 최대오차0, nonfinite0을 확인했다. 수정 전 원본과 최대오차는119.560181이며 age.5 fixture의 alpha 합은0이었다. 수정 후 같은 입력은 원본과 같이13.0099522~14.4575264로 공간별 알파를 유지했다. 이는 입력 누락에 따른 사라짐·저대비의 수치 근거이며 전체 화면 원작 일치 판정이 아니다.

material/distortion 재생성 모두 기존 설치 파이프라인 결과와 함수 원문이 일치했다. distortion은 기존 `prepare_distortion`의 독립 discard→0 accumulation adapter를 그대로 적용해 비교했다. 전체 `Shader_VtxEffectTrail.hlsl` FXC fx_5_0 컴파일과 Python compile, 변경 파일 `git diff --check`가 성공했다. 기존 Common X4000 및 무관native3328 X4008 경고는 남았다. 결과는 `out/TrailRibbonRestoration20260917/material/{source-proof,warp-result,regeneration,verified}.json`, `trail-fxc.log`다. distortion 원본DXBC의 별도 WARP pixel parity는 실행하지 않았다.

발탄 제품 `carrier-v1.attack.four-slash.active.clip-01/02`는 각각7/2개의 standard particle/screenPost 요소이며2379 Trail을 포함하지 않는다. 이번 재질 수정의 직접 대상은 Full Restore다. 쿠크2836/3007은 이미UV1이 연결돼 있어 해당 native재질을 임의로 흰색·고휘도로 바꾸지 않았다. 원작100cm TilingDistance가 발탄 detail에 누락된 별도 결함은G03에서 처리한다. 실제 vertex UV축·궤적 검증은G02/G03과 구분한다.

라이브 Effect JSON과 설치CSO는 이 작업에서 변경하지 않았다. 제품 설치·Client 실행·사용자 최종 화면 판정은 대기다.

## G02. 원본 거리 생성과 실제 본 궤적

쿠크 ritual hand/staff의 원본 UPK를 기존 parser로 다시 읽어 SpawnPerUnit·TypeData·Spawn 5개 export를 대조했다. 두 Ribbon의 SpawnRate=0, SpawnPerUnit=75cm당1개, 최대50점, tessellation5cm, tiling500cm는 실제 binary 추출값과 같다. 현재 코드는 한 tick에서 필요한 개수만 계산한 뒤 모두 현재 본 위치에 생성해 중복점을 만들었으며, 매1초 emitter loop에서 이동 나머지도 버렸다.

`Effect_Playback.cpp::Step`은 portable world-space CascadeRibbon의 거리 생성만 이전·현재 실제 본 표본 사이의 거리 분율에 배치한다. 기존 `Spawn_Particles` world override를 재사용하므로 source module·난수·수명·폭 계산 경로는 그대로다. Ribbon의 loop 경계는 이동 나머지와 이전 원점을 유지한다. `Consume_SourceSpawnPerUnit`의 private optional out 인자는 소비한 이동 기여량만 전달한다. 공개 저장 필드·Shared·Data·Renderer·shader·60Hz는 바꾸지 않았다. 같은 CPP의 기존 sprite axis와 별도 Cylinder 변경은 보존했다.

실제 설치 MN_RPCT_05 CModel(.017 preScale)과 production source-anchor sampler를 기존 probe로 재사용했다. 동일 staff 궤적에서 red 중복60/1425·white98/2570 구간이 모두0이 됐고, 최대 인접 간격은1.816333274m에서0.750000538m가 됐다. 원본75cm 초과 구간은 red603·white1030에서모두0으로 줄었다. 최대 폭1.36m/.339592m와 최대 점20/30은 유지했다. staff·hand·staff yaw90도 모델 표본/rewind 검사를 통과했다. 이는 CPU playback 수치이며 화면의 두께·색·연속성에 대한 사용자 판정을 대신하지 않는다.

실제 Codec/Stage/Playback focused fixture는135개 검사0실패다. 같은 fixture에 기존 Playback OBJ를 연결하면132개 검사54실패로 재현된다(기존4점과 수정5점으로 반복 검사 수가 다름). 1m/s의1초 loop carry, 90m/s에서 한 tick 복수 출생, missing-anchor 입력 거절과 기존 clock 보존, Seek 재현을 확인했다. 일반 Sprite11개의 모든 출력 위치는 수정 전후 동일하다. 원본 capacity를 넘는 입력을 입자 상한 증대로 우회하지 않는다. 실제 Playback TU의 격리 컴파일과 해당 diff check를 통과했다. 기존 include의 C4828 경고는 남아 있다.

발탄 stage008/009의 baked4개는 원본 Trails 객체의 binary를 다시 읽어 properties와 설치 history 배열 전체가 같음을 확인했다. 표본수101/89/48/130, 시작epoch0, 끝span .2666664/.1999998/.1999998/.5333328초와 occurrence clamp가 일치한다. 최대 표본 간격16.66665/8.33344/16.66665/16.66665ms이며 이 계층에서 tick100배가 필요한 누락 근거는 없었다. baked history를 새 궤적으로 교체하지 않았다. UV 입력·타일링은 G01/G03에서 따로 다룬다.

검증 묶음은 `out/TrailRibbonRestoration20260917/geometry/verified.json`, 전후 거리 측정은 `distance-comparison.json`, 원본 근거는 `native-ribbon-fields.json`과 `valtan-baked-epochs.json`이다. 제품 전체 빌드·Client/UI 실행·라이브 Data 쓰기·GPU 최종 판정은 하지 않았다.

## G03. baked AnimationTrail의 원본 100cm 타일링

발탄 stage008/009의 네 baked trail은 retained TypeDataAnimTrail에 tilingdistance=100을
보존했지만 detail.trail.tilingDistanceWorldUnits는 없었다. 기존 Codec의0 기본값이 렌더러의
U=pointIndex 분기를 선택해 표본마다 텍스처가 반복됐다. 원본 history는 수정하지 않았다.

`Effect_DocumentCodec.cpp`는 animationTrailBakedEdgeV1이고 JSON에 해당 detail 필드가
없는 경우만 원본 숫자를 읽어1m로 복원한다. 명시0·명시2m는 그대로이고 sourceRecipe가
disabled인 baked carrier도 근거를 읽는다. source module/값이 없으면 기존0을 유지하며
음수·범위 초과·잘못된 타입·중복 값/모듈은 OutDocument commit 전에 거절한다. 저장 시 기존
serializer가 복원값을 기록하므로 새 schema는 없다. 현재 디스크 JSON은 변경하지 않았다.

`build_kouku_gate1_full_restore.py`와 이를 사용하는 `build_kouku_action_effect_groups.py`도
같은 retained literal을 읽어 생성 시 누락을 예방한다. 일반 Ribbon/Beam의 기존 별도 tiling
수입 경로는 유지했다. 이 변경은 Renderer의 거리/tiling 수식·UV 축·geometry를 바꾸지 않는다.

실제 두 문서의 Load/Validate_Drawable/Stage/Roundtrip와 명시값 보존, 잘못된 입력의 rollback
14 fixture에서54검사0실패다. 현재 Authored 전체의 baked20요소를 추가 조사했고 원본 값은
모두 유효했다. 전체 문서 확대 검사는100검사 중1개 기존 실패가 남았다. 별도 구형
`effect.valtan.pattern.420633.active`의 `valtan.420633.notify004.emitter5259`가 material
variant/carrier/named inputs admission에서 거절되는 문제이며, 수정 전 Codec을 정확한
baseline bytes로 다시 컴파일해도 같은 오류다. 이번 four-slash420609의 두 문서는 통과했다.
이 기존 재질 오류를 새 타일링 수정의 성공으로 숨기거나 validator를 완화하지 않았다.

Codec TU 컴파일·probe 링크와 세 Python 문법 검사를 통과했다. 기존 Engine 헤더 C4828,
DirectXTK PDB LNK4099 경고가 있다. 증거는 `out/TrailRibbonRestoration20260917/tiling/`
의 `verified.json`, `baked-trail-inventory.json`, `codec-probe.log`, `all-baked-probe.log`,
`before-all-baked-probe.log`다. 실제 Valtan 문서와 사용자 피자 문서의 작업 전후 bytes가 같다.

## G04. 피자 검정 부분의 원본 근거와 남은 화면 확인

검정 layer의 원본 자료는 존재한다. native3171 dark-aura는 시작8×8m가 수명에 따라
24×16m로 늘어나는 바닥 sprite이며 별도 analytic UV alpha를 쓴다. native3280 flow는
폭2m·길이6.5~7m인 velocity-facing sprite다. `fx_d_noise_002`는 흐름 입력이고 원형 경계에는
`fx_m_ring_001_cl` 및 radial UV alpha 계산이 관여한다. 원본 emitter1은 반원 제한, emitter31은
CircleSurface의 half-mode/split6 방향을 갖는다. 개별 sprite의 타원과 여러 입자의 합성 외곽은
구분해야 한다. 모든 검정 영역을 잘린 mesh 하나 또는 둥근 sprite 복제만으로 설명하지 않는다.

primitive Cylinder가 원본 positive/negative XYZ를 무시하던 결함을 기존 Spawn 함수에서
수정했다. 36조건9,216표본의 제한 위반4,608→0이며, 무제한2,304표본과 다른 검정 층156표본은
수정 전후 동일하다. 사용자19요소에서는 원본 emitter1이 이미 없고 emitter31의 복제 둘이
들어 있으므로 이 코드 수정이 사용자의 현재 타원 배치를 자동으로 원형으로 만드는 것은 아니다.
사용자 복제·크기·회전은 보존했다. 원본16요소 비교본은 `out/PizzaBlackLayer20260917/`
의 `source-reference.effect.json`이며 설치 후보로 등록하지 않았다. 상세 원본·실제 occurrence
근거는 세이튼09-17 결과 G20 검정 레이어 후속과 같은 폴더의 `evidence.json`에 둔다.

독립 리뷰에서 Codec의 missing/explicit 구분·transactional 실패, shader 수식 보존,
SpawnPerUnit의 다른 family·count·RNG·loop/reset 경로에 차단할 회귀를 찾지 못했다.
출생 시각·회전은 기존 tick 값을 유지하므로 완전한 원본 CPU runtime 재현으로 확대하지 않는다.
이번 변경의 `git diff --check`는 통과했다. Client/UI 실행 또는 화면 판정은 하지 않았다.


## G05. 점선 리본의 구조 원인과 coverage/detail UV 계약

### 1. Trail의 본질과 이번 결함

Trail은 시간에 따른 위치 기록을 두 edge 또는 폭을 가진 연속 면으로 연결한 것이다.
원본 쿠크 CascadeRibbon은 거리 기반 생성과 수명·폭을, 발탄 AnimTrail은 실제 animation의
first edge/control/second edge 표본을 갖는다. 면이 연결돼 있어도 각 pixel의 alpha가 길이
방향으로 0이 되면 화면은 작은 가로 무늬·점선으로 보인다. 따라서 정점 수를 늘리거나
60Hz를 바꾸는 것만으로 이번 mask 좌표 오류를 해결할 수 없다.

현재 Engine의 `VTXEFFECT_TRAIL`은 52 bytes이고 UV는 float2 한 쌍이다. `Render_Trails`는
실제 history로 두 vertex씩 만들고 triangle list index를 생성한다. `CVIBuffer_DynamicTrail`
upload/bind/DrawIndexed까지 연결돼 있으며, 검토한 native Ribbon은 control point를 독립
sprite로 그리는 경로가 아니다. 기존 문제는 Trail PS의 `uv1=runtimeUV.yx`가 실제
`runtimeUV=(거리/타일길이, 폭)`에서 길이를 texture V로 보내는 데 있었다.

### 2. Unreal 원본에서 확인한 것과 아직 회수하지 못한 것

[UE3 ParticleSystemReference](https://docs.unrealengine.com/udk/Three/ParticleSystemReference.html)의
Ribbon/AnimTrail TypeData는 거리 타일링용 두 번째 UV set과 geometry tessellation을 구분한다.
[UE3 AnimTrails](https://docs.unrealengine.com/udk/Three/AnimTrails.html)는 animation sampling과
두 edge/control point를 설명한다. 이 공식 설명만으로 cooked shader의 packed xy/zw가 어느
Material TexCoord index인지까지 정해지는 것은 아니다.

회수한 material VS `f6b274c2c28e4b45b0c2762be4e095fb`는 `mov o2.xyzw,v3.xyzw`, distortion
VS `91ccb94877dac34e988dd1d7bf625e2c`는 `mov o1.xyzw,v3.xyzw`다. 실제 프로그램은
`out/KoukuAllEffects20260912/native/full_programs/`와
`out/FullMapRestoration20260915/ValtanActor/Whirlwind/native/full_programs/`에 있다.
원본 CPU vertex-buffer bytes·head/tail 방향은 회수하지 못했다. UPK material graph의 주요
expression 연결도 stripping돼 있어 source CPU packing과 완전히 동일하다고 주장하지 않는다.

WaterRibbon PS `59a22eeec5a51f439595f929dddfe8bf`는 packed zw를 mask와 end taper에 사용한다.
실제 red DDS `fx_c_line_002_cl`의 R 평균은 U별 .060830~.061075로 거의 일정하고 V별
0~.870588로 좁은 가로 띠다. source clamp U/V를 유지한 상태에서 V에 길이를 넣으면 긴
경로의 대부분이 검정 texel을 읽는다. white `fx_k_auraline_02`도 V 가운데 띠이며 wrap이므로
같은 오류는 길이 방향의 반복된 가로 무늬가 된다. sampler를 clamp에서 wrap으로 바꾸는
방법도 원본 계약을 훼손하고 반복만 남기므로 사용하지 않았다.

white 원본 `gra_pow=1`의 식은 `min(3*pow(abs(1-abs(2*z-1)),p),1)`이며 0 근처 branch가 있다.
이 식은 periodic가 아니다. z=0,1에서 0이며 z>1에서 다시 증가한다. 단순히 축만 교정해
z=거리/5m를 넣으면 살아 있는 길이 내부의 5m 지점에서 pinch가 생길 수 있다. red는
`gra_pow=0`으로 대부분 1이지만 경계 branch는 남는다. 원본 parameter를 0으로 덮어 문제를
숨기지 않았다.

발탄 PS `34c7dac3b50dcd40b8c18be67bbf7d5a`는 xy를 alpha texture의 coverage에, zw를
emissive/noise/dissolve detail에 사용한다. 두 material을 동일한 packing으로 강제하면 한쪽
오류를 다른 쪽으로 옮기므로 최종 adapter는 아래처럼 소비 범위별로 분리한다.

### 3. 실제 수정

| profile | packed xy | packed zw |
| --- | --- | --- |
| WaterRibbon2346/2836/3007 | 거리 detail | 정규화 coverage |
| AnimationTrail2379 | 정규화 coverage | 거리 detail |

`Render_Trails`는 퇴화 구간 처리 뒤 실제 최종 업로드 vertex의 첫·끝 U로 전체 살아 있는
길이의 coverage를 만든다. 매 draw uniform을 초기화·bind하여 다른 material의 값이 남지
않게 했다. `Shader_VtxEffectTrail.hlsl::Resolve_NativeTrailUV`가 검토된 네 profile의 양수
tiling에만 위 계약을 적용한다. 원본 CPU packing의 완전 복원 대신 회수한 pixel-program의
유한 coverage/거리 detail 소비를 만족하는 adapter라는 범위를 유지한다.

명시적 tiling=0과 다른 profile은 기존 geometry/UV carrier를 보존한다. native2346의 material와
distortion 함수는 최종 단계에서 zw를 0으로 버리고 있어 기존 generator allowlist와 맞게
`float4(input.uv,input.uv1)`로 복구했다. 이 consumer 수정은 tiling=0에도 영향을 줄 수 있으므로
2346의 과거 재질 출력까지 bit-identical하다는 뜻은 아니다. 원본 shader 식·MIC·DDS·sampler,
history·수명·폭·count·Engine vertex ABI는 수정하지 않았다. 유효하지 않은 span은 실패하고
0 span은 finite 0 coverage를 사용한다. 이전 CSO의 uniform 누락은 bind 실패로 명시한다.

변경 파일은 `Effect_DocumentRenderer_Particles.cpp`의 `Render_Trails`,
`Shader_VtxEffectTrail.hlsl`, `Shader_EffectKoukuNativeGroup2304.hlsli`의 두 입력과
`generate_artist_native_runtime_shader.py`의 잘못된 transverse-edge 설명 주석이다.
생성기에는 해당 exact VS/PS allowlist가 이미 있어 실행 로직을 추가하지 않았다.

### 4. 이전 방식이 계속 실패한 이유와 독립 비평

사용자가 보고한 여섯 번의 각 변경·실행을 전부 재구성한 것은 아니다. 확인하지 못한 여섯
단계를 임의로 만들지 않는다. 실제 확인된 실패 범위는 다음과 같다.

- UV1=0 누락을 복구하는 수정은 필요한 일이지만, upstream의 `.yx`가 잘못된 의미의 좌표를
  공급하면 정확한 원본 PS도 잘린 alpha를 계산한다.
- SpawnPerUnit 중복점을 없애고 원본 tiling 값을 복구해도 올바른 geometry만 만들어진다.
  폭 마스크와 유한 taper가 어느 좌표를 읽는지 확인하지 않으면 화면 연속성을 보장하지 못한다.
- finite/count·연결 triangle list 검사는 alpha 소비를 검사하지 않는다. 임의 UV rectangle에서
  원본 PS와 번역 PS가 같다는 검사도 실제 renderer가 올바른 UV를 넣는지 증명하지 않는다.
- 과거 RESULT의 미설치 기록만으로 이번 실패를 미빌드 탓으로 설명할 수 없다. root가 확인한
  실제 Client EXE·Playback/Codec OBJ·설치 CSO의 시각에는 이후 제품 빌드가 실행된 근거가 있다.

독립 비평은 2346의 최종 zw 누락과 white U=1 내부 pinch를 각각 P1로 지적했다.
단순 축 교정만으로 끝내지 않고 두 항목을 반영했다. 후속 독립 코드 검토에서 profile별
소비·양수 tiling 분기·매 draw bind에 추가 P1은 발견되지 않았다. zero tiling에서2346 출력까지
동일하다고 표현하면 안 된다는 지적도 이 RESULT에 반영했다. 상세 비평은
`out/TrailRibbonStructural20260917/independent-review.md`에 있다.

### 5. 검증과 남은 경계

첫 raw GPU 비교는 실제 MN_RPCT_05 CModel의 staff 13개 frame을 사용했다. 원본 source DDS와
cooked PS DXBC, 같은 per-point color/dynamic payload를 실제 Engine buffer에 업로드했다.
기존 red coverage는 486개 길이 열 중 23~106개만 활성이고 최대 공백은 287~376열이었다.
축을 교정하면 red는 여섯 시각 모두486열 활성·공백0이었다. 원본 PS와 번역 PS의 같은 입력
최대 차이는1.430511475e-6이다. 이 최초 검사는 원인 재현이며 최종 profile별 계약은 아래
후속 결과로 구분한다.

후속 동일 probe는 21 cases×8 draws를 완료했다. staff 13 frames와 발탄 네 실제 baked history의
중간/후반 8 frames다. Engine의 실제 vertex layout/triangle list/upload/draw와 원본 DDS를
사용하며, 마지막 변형은 제품 `Resolve_NativeTrailUV` 함수 원문을 그대로 추출했다.
4개 변형 모두 번역 PS와 원본 DXBC의 모든 RT0 성분 오차<.003 검사를 통과했다. 로그에
출력한 old/tiled 최대오차는1.430511475e-6이고 발탄 old/tiled은0이다. 최종 변형의 개별 최대값을
별도 출력하지 않았으므로 최종까지 동일한 최대값이라고 확대하지 않는다.

| 대상 | 기존 활성 길이 열 /486 | 최종 활성 길이 열 /486 | 최종 내부 gap=0 |
| --- | --- | --- | --- |
| red2836, 6 frames | 23~106 | 모두486 | 6/6 |
| white3007, 7 frames | 174~229 | 408~482 | 5/7 |
| Valtan2379, 8 frames | 418~485 | 482~486 | 7/8 |

활성 판정은 512×128 UV 검사 격자의 각 길이 열에서 alpha>.01인 pixel이 하나라도 있는가다.
이는 화면 pixel 또는 면적 전체가 불투명하다는 판정이 아니다. 끝 gap과 내부 gap을 구분했다.
white의 단순 축 교정은 .25초에서 U=1 예상열338.14의337~338열, .5초에서200.46의200~201열,
1.5초에서321.68의317~325열이 비었다. 최종 normalized coverage에서는 이 gap이 모두 없어졌다.
따라서 축 교정만 적용해 일곱 번째 반복 실패로 남길 수 있는 유한 taper 문제도 반증했다.

남은 세 frame을 정상 noise라고 단정하지 않는다. white2초의62/65열은 각각 최대alpha
.0094629/.0089242, 실제 기록 payload를 거리 방향으로 선형 보간한 particle alpha는
.07724/.07938이며 taper는 .61308/.65008이다. white3초의114~138열은 최대alpha
.0067479~.0097836으로 0이 아니고, particle alpha .04156~.05135이며 taper=1이다.
즉 이 구간은 U=1에서 강제0이 되는 구조 결함과 다르며, 이미 작아진 수명 alpha에 source
texture/opacity 계산이 곱해진 낮은 값이다. 3초112열에도 한 열이 남는다. 이 선형 분해는
actual CSV와 renderer의 distance/color 선형 보간식에 기반하며 GPU float 반올림과는 구분한다.

발탄 마지막 stage009 후반의32열과51~52열은 최대alpha .0049680~.0070930으로 역시 0은 아니다.
이 3열의 DDS/dissolve/particle alpha 기여를 각각 격리한 검사는 하지 않았으므로 외관상
정상이라고 확정하지 않는다. 나머지 발탄7 frames는 내부 gap0이며4개 history 모두 실제
25~129 edge pairs,50~258 vertices,48~256 triangles로 GPU까지 도달했다. 모든 화면이
연속적이라는 승인 대신, 검토한 UV carrier 결함 수정과 이 수치 범위를 완료 상태로 기록한다.

후속 첫 실행은 probe가 `Stage_Document`만 호출해 v15의 baked projection을 생략하여
`actual Valtan edges`에서 실패했다. 기존 실패 EXE·소스·입력 manifest·stdout·stderr·receipt는
`followup/failed-authoring-stage/`에 보존했다. 기준을 완화하지 않고 제품 Catalog와 같은
`Create_DocumentOwnedRuntimeProjection → Prepare_DocumentResources(immutable,true) →
Stage_PrevalidatedVisualProgramDocument → Seek`로 수정했다. 두 번째 실행은12.271초,
exit0, 검토한109개 입력 SHA 유지, timeout 없음이다. raw 결과는
`out/TrailRibbonStructural20260917/followup/coverage-run2-stdout.json`, 요약과 남은 gap 분해는
`out/TrailRibbonStructural20260917/coverage-analysis.json`이다.

이 검사는 geometry를 UV 공간으로 펼친 raw alpha 수치이며 depth/blend/distortion/bloom 등의
후처리를 실행하지 않는다. 따라서 이번 UV 오류가 후처리 없이도 잘림을 만드는 직접 원인이라는
것은 입증하지만, 첨부 화면의 모든 잘림에서 깊이·자기 겹침·후처리 영향을 배제하지 않는다.
원본 PS 전체의 정확한 입력 수식 일치와 잘못 공급한 coordinate 의미를 구분한다.

최신 Renderer TU와 Trail FXC의 격리 컴파일, Python 문법 검사와 변경 범위 diff check는 성공했다.
기존 include의 C4828과 무관 shader X4000/X4008 경고는 남았다. 다른 작업이 sprite detail의
ABI를 바꾼 것을 발견하여 옛 Codec/Playback OBJ 연결은 실행하지 않고 최신 header로 관련
closure 전체를 다시 컴파일했다. 제품 EXE/DLL/CSO 교체, Client/UI 실행·캡처, 사용자 화면 판정은
하지 않았다. source 반영과 사용자 빌드 뒤 실제 화면 검증을 구분한다.

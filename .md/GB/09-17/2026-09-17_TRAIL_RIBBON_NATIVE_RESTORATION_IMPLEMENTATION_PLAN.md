# 발탄·쿠크 Trail/Ribbon 원본 입력과 렌더링 복원 구현 계획

## G00. 현재 연결과 작업 경계

사용자 첨부의 발탄은 큰 무늬가 반복되고 쿠크의 띠는 원작보다 얇고 끊겨 보인다. 이 관찰을
Tick 부족 또는 emissive 부족으로 곧바로 확정하지 않는다. 실제 원본 cooked module·ShaderMap·
VS/PS와 설치된 CModel의 bone/time, 생성된 vertex UV/색/폭을 각 소비자까지 대조한다.
100배 update와 전역 밝기 변경을 기본 해결책으로 삼지 않는다.

발탄 VALTAN_FOUR_SLASH는 source420609 sequence3, Full Restore stage008/009의
mesh_att_battle_10_01/02를 사용한다. 제품 cue의 carrier-v1 문서는 별도이므로 두 입력을
구분한다. 발탄 baked AnimationTrail 재질은 native2379이며 쿠크 저주의식의 왼손·오른손
지팡이 문서는 native2836/3007 Cascade Ribbon 두 개를 사용한다.

현재 branch의 기존 변경과 사용자 피자 복제·회전·재질 편집은 보존한다. 미저장 편집 확인은
별도로 진행하고 라이브 Data는 쓰지 않는다. 코드·생성기 복원은 기존 경로를 수정한다.
새 runtime/public 저장 schema나 별도 shader family를 만들지 않는다. 실제 화면은 사용자가
검증하며 Client/UI 실행·조작·스크린샷 생성은 하지 않는다.

## G01. native2379의 원본 TEXCOORD 입력

현재 native2379의 source VS는 TEXCOORD0.xyzw를 전달하지만 번역 PS는 float4(UV0,0,0)을
사용한다. 원본 PS의 zw 사용을 exact DXBC로 대조하고, 검증된 UV1 adapter에 AnimationTrail을
포함한다. generate_artist_native_runtime_shader.py와 해당 native Group2368의 material/
distortion 입력이 같은 계약을 사용해야 한다. 원본 연산·MIC 값·emissive·blend는 유지한다.

실제 원본 DDS·scalar·ParticleColor·DynamicParameter를 고정한 작은 기존 WARP 비교로
source/현재/수정의 차이를 분리한다. 각 shader 입출력과 UV 축 해석을 기록하고 scene 최종
시각 일치와 혼동하지 않는다. 생성기 재실행이 수정된 출력을 유지하는지도 확인한다.

## G02. SpawnPerUnit의 궤적상 출생 위치

쿠크 Ribbon의 원본 SpawnRate는0, SpawnPerUnit은75cm당1개다. 현재 Playback은 이동에서
발생 개수만 계산하고 같은 tick의 끝점에 모두 생성한다. 실제 설치 CModel 시계에서 중복점과
설정 간격보다 큰 공백을 재현한 뒤, portable source Ribbon의 해당 출생만 거리분율에 따라
이전·현재 위치 사이에 배치한다. 원본 방출 수·수명·폭·UV·60Hz는 보존한다.

Effect_Playback.cpp의 Step/Consume_SourceSpawnPerUnit과 필요한 private 선언을 수정한다.
다른 particle family의 발생 위치와 beam/baked AnimationTrail에는 전파하지 않는다. emitter
loop 경계의 거리 carry, 역재생/reset, 고속 이동, 실제 본 궤적과 실패 시 기존 상태 보존을 검사한다.
같은 파일의 피자 Cylinder 모듈 수정과 함수 소유 범위를 분리한다.

실측 근거: 두 ribbon의 최대50점·tessellation5cm·tiling500cm는 원본 package 값이다. 수정 전 실제 CModel 샘플의 red60/1425·white98/2570 인접점이 중복이며 최대 간격1.81633m였다. 기존 CModel/production source-anchor probe를 재사용하고 `out/TrailRibbonRestoration20260917/geometry`에 수치와 로그를 둔다.

## G03. 실제 Trail vertex UV와 geometry 소비

Render_Trails는 baked edge history 또는 centerline을 CVIBuffer_DynamicTrail에 넘긴다.
현재 tilingDistance=0 경로는 U=pointIndex를 넣어 segment마다 texture를 반복한다. 발탄의
네 원본 TypeDataAnimTrail에는 tilingdistance=100cm가 있으나 두 import builder가 이 값을
detail에 쓰지 않았다. 원본 1m 반복값을 기존 builder와 Codec에서 복구한다. Codec은 baked
AnimationTrail이며 JSON에 tilingDistanceWorldUnits가 없는 경우에만 retained source module을
읽는다. 명시한 0을 포함한 사용자 값은 보존하고 잘못되거나 중복된 source 값은 stage에서
거절한다. sourceRecipe.enabled=false는 baked geometry 사용을 뜻하므로 근거 조회를 막지 않는다.

렌더러의 기존 distance/tiling 수식·폭 축 연속성·단면 winding·퇴화 구간 비연결 및 baked
EdgePairs는 보존한다. 새 C++ 파일·프로젝트 등록은 필요하지 않다. 실제 Codec load/roundtrip/
Playback stage와 명시값 보존·실패 rollback을 검사한다. 원본 CPU vertex bytes가 없는 UV 축
해석은 원본 연산과 동일 수준의 증거로 표현하지 않는다. 라이브 사용자 문서를 덮어쓰지 않는다.

## G04. 피자 검정 레이어와 공동 검증

피자의 dark-aura/flow-mask/world-offset 원본 모듈과 사용자 복제 요소는 기존 세이튼 PLAN
G20 후속에서 조사한다. Cylinder/CircleSurface의 반원·축 제한과 velocity-facing/StartSize를
대조한다. 같은 검정 DDS라는 이유로 원형 Sprite 하나의 외곽을 원작 구조라고 추정하지 않는다.

수정된 CPP/HLSL의 필요한 컴파일과 해당 실제 함수·원본 DXBC 비교, JSON 읽기 및 사용자
문서 보존 해시, git diff --check를 확인한다. 제품 전체 빌드·실행·최종 화면 판정은 사용자에게
남기며 소스 반영·데이터 설치·compiled shader 설치 여부를 RESULT에서 각각 구분한다.


## G05. 유한 coverage와 거리 detail의 분리 및 실제 GPU 소비자 검증

첨부의 끊긴 가로 무늬는 정점 개수·유효 좌표 검사만으로 판정할 수 없다. 실제 원본
TypeData의 history, lifetime, 폭을 유지한 연결 triangle list라도 잘못된 UV로 alpha를
읽으면 보이는 결과는 점선이 된다. 현재 52-byte vertex의 UV는 `(거리/타일길이, 폭)`인데
Trail PS는 `.yx`로 UV1을 만든다. WaterRibbon의 mask DDS는 V 가운데에 띠가 있으므로
현재 코드는 폭 마스크를 길이 방향에 반복하거나 clamp 범위 밖으로 보내 잘라낸다.

원본 VS `f6b274c2c28e4b45b0c2762be4e095fb`는 TEXCOORD0 float4를 그대로 전달한다.
원본 PS `59a22eeec5a51f439595f929dddfe8bf`의 zw에는 mask와 유한 end taper가 있으며,
`34c7dac3b50dcd40b8c18be67bbf7d5a`의 xy에는 alpha mask, zw에는 detail/noise/dissolve가
있다. 원본 CPU vertex packing과 head/tail 방향은 회수하지 못했다. 따라서 packed zw를
UE3 Material TexCoord1이라고 단정하지 않고, 확인한 pixel-program 소비 범위에 맞는
명시적 adapter로 아래 두 좌표를 연결한다. 정규화는 흰 띠가 보기 좋다는 선택이 아니라,
white source의 유한 taper가 길이 내부 U=1에서 다시 0이 되는 문제를 막는 계약이다.

| 대상 | packed xy | packed zw | 근거 |
| --- | --- | --- | --- |
| WaterRibbon2346/2836/3007 | 거리 detail | 전체 살아 있는 길이의 coverage | zw mask V=폭, z의 유한 taper |
| AnimationTrail2379 | 전체 살아 있는 길이의 coverage | 거리 detail | xy alpha mask, zw emissive/noise/dissolve |

`Render_Trails`는 실제 최종 업로드 vertex의 첫·끝 U로 정규화 scale/offset을 만들고 매 draw
`g_TrailSourceUVTransform`을 bind한다. HLSL `Resolve_NativeTrailUV`가 정확히 위 네 profile과
양수 tiling에서만 두 좌표를 배치한다. 다른 profile과 명시적 tiling=0은 기존 geometry/UV
carrier를 유지한다. 2346의 최종 native material/distortion 함수에서 누락됐던 zw는 생성기
allowlist와 일치하게 복구하므로, 2346의 재질 결과까지 과거와 bit-identical하다고는 하지 않는다.
span=0은 finite 0 좌표로, 음수·비유한 span은 명시적 실패로 처리한다. 이전 CSO에 새 uniform이
없어 bind에 실패하면 기존 renderer 실패 경로로 거절한다. Engine vertex ABI, authored 필드,
폭, lifetime, particle count, 원본 shader 수식·MIC·sampler·DDS는 바꾸지 않는다.

검증은 다음 소비자를 한 단위로 연결한다. 첫 검증의 actual CModel point/width/color/dynamic
13개 frame과 발탄 네 baked history의 actual Codec→Playback Seek 8개 frame을 같은 probe에서
사용한다. 최신 제품 geometry 함수 원문으로 strip을 만들고 실제 Engine DynamicTrail GPU
upload/bind/DrawIndexed를 거친다. geometry를 UV 공간으로 펼쳐 실제 DDS·원본 DXBC PS와
번역 PS의 raw alpha를 비교한다. 기존 `.yx`, 단순 축 교정, 모두 normalized, 최종 profile별
계약을 비교하고 전체 평균뿐 아니라 첫·끝 활성 열·내부 gap·U=1 주변 alpha를 기록한다.
이는 카메라·깊이·합성까지 포함한 Client 화면 검증이 아니다.

독립 비평에서 발견한 2346 최종 소비자 누락과 white U=1 내부 pinch를 반영한 뒤 실제 수정
함수 원문을 probe에 추출한다. 현재 제품 TU와 Trail FXC 격리 컴파일을 수행한다. 다른 작업이
같은 header의 ABI를 바꾸면 관련 Codec/Playback closure를 새로 컴파일하며 옛 OBJ와 섞지 않는다.
첫 검증 입력과 receipt를 보존하고 구체적 반증에 필요한 후속만 실행한다. Client/UI 실행·캡처와
제품 바이너리/CSO 설치는 하지 않으며 사용자 최종 화면 판정은 별도로 남긴다.

공식 UE3 근거는 [ParticleSystemReference의 Ribbon/AnimTrail TypeData](https://docs.unrealengine.com/udk/Three/ParticleSystemReference.html)와
[AnimTrails의 animation sample·두 edge·control point](https://docs.unrealengine.com/udk/Three/AnimTrails.html)다.
TilingDistance는 두 번째 UV set의 거리이고 0이면 두 번째 set을 타일링하지 않는 계약이다.
이 문서는 cooked shader의 packed channel 순서를 증명하지 않으므로 원본 CPU bytes 회수와
구분한다. 원본/현재 코드/격리 실행/제품 설치/사용자 화면 결과는 RESULT에 각각 기록한다.

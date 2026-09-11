# 베른 원본 반투명·하늘·직접광 연결 결과

## G00. 현재 구현 범위

기존 베른 전체 복구 PLAN의 재질 연결 범위다. 반투명 장식·additive·하늘 48 MIC를 Base와 Baked PS 조합 20개, source program 44~63으로 연결했다. 물 17 MIC의 Base 38~43은 별도 물 RESULT를 따른다. 이번 단계는 두 범위에서 원본 direct-light PS가 존재하는 52 MIC/21개 프로그램도 연결한다. 새 Level·모델 런타임·별도 렌더러를 만들지 않았다.

## G01. 실제 소비 경로

`SourceMapTranslucentMaterialParameters.h`는 Base와 Baked CB0를 기존 baseConstants의 0~31, 32~59에 구분한다. 시간식이 참조하는 parameter는 남은 60~63을 사용한다. 원본 FName number suffix, 실제 texture expression index, 원본 색 공간 및 sampler 주소를 보존한다. `SourceMapWaterMaterialParameters.h`와 이 헤더의 lightConstants/lightTextureMask는 선택된 원본 Light PS 바인딩을 따르며, 필수·추가 파라미터 검사와 transaction commit을 유지한다.

`Shader_SourceMapTranslucentPrograms.hlsli`는 Base/Baked, `Shader_SourceMapDirectPrograms.hlsli`는 Light 계산을 소유한다. 프로그램 47/53/55는 원본 unlit additive이며 Baked/Light를 억지로 추가하지 않는다. 61은 실제 원본 Baked PS가 있는 하늘이며 TEXCOORD2를 요구한다. WMSH 1.4의 optional UV2와 requiredExtraUVMask=2로 잘못된 이전 geometry 입력을 거절한다.

`CMapAssetObject -> CMapAssetRenderUtils -> CModel -> CMaterial`이 해당 material slot의 실제 render mode, RNM atlas와 placement UV, 원본 재질 상수를 바인딩한다. source program33~65는 deferred 재질 목록에 등록하지 않는다. 64는 조명을 수용하지 않는 검정 opaque 차폐면이며 row=-1로 표시한다. 동일 model의 material variant는 geometry를 공유하며, native RNM/SDF도 다른 원본 texture와 같은 weak SRV cache를 사용한다.

직접광은 기존 forward PS에서 계산한다. Scene 16개와 기존 transient 최대 384개의 입력을 사용하며, map object의 world bounds에 닿을 수 없는 local light는 CPU에서 제외한다. 배열을 임의로 잘라 광원을 잃지 않으며 초과는 오류다. 실제 Baked PS를 소비하는 표면은 RNM에 포함된 source-character-only local light를 재가산하지 않는다. shadow atlas는 해당 light의 channel과 material channel이 같을 때 direct 출력에만 적용한다. 안개 투과율과 기존 alpha 합성은 현재 renderer adapter이며 원본 CPU/전체 frame 동등성으로 해석하지 않는다.

## G02. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 반투명/하늘 Base+Baked 원본 DXBC 대조 | 91 stage × 9 fixture = 819개, 838,656픽셀, nonfinite 0, 상대 오차 0.001 초과 0 |
| 물·반투명 Light 원본 DXBC 대조 | 52 MIC × 9 fixture = 468개, 479,232픽셀, nonfinite 0, 상대 오차 0.001 초과 0 |
| 실제 C++ Configure | 65행, texture mask union·필수 누락·추가값·finite·실패 시 이전 결과 보존 통과 |
| 실제 C++ 최소 컴파일 | MapAssetRenderUtils/MapAssetObject/Material 통과. 이후 공통 변경은 최종 Product 검증에 포함 |
| 전체 MeshBinary FXC | Base+Baked·직접광·helper64/65를 포함한 Product6 컴파일 및 최종 CShader 생성·material binding 통과. 세부 결과는 G06 |
| 프로젝트 XML·diff | Client vcxproj/filters parse 및 ProjectReference 구조 검사, root 변경 범위 diff check 통과 |

원본 constant는 각 MIC의 원본 uniform expression에서 독립 계산한 binary 입력으로 만들고, 제품 constant는 실제 C++ Configure 출력으로 읽었다. DXBC varying signature의 register와 lane mask도 비교했다. 하나의 register에 서로 다른 semantic이 나뉘어 들어가는 Baked layout을 구분한다. Windows min/max 매크로 충돌은 지역 helper 이름으로 제거했다.

근거는 `out/BernMaterialAudit20260911/translucent_gpu`, `direct_gpu`, `compile_direct.log`에 있다. out 검증 도구는 제품 실행·저장 경로의 선행조건이 아니다.

## G03. 현재 남은 경계

최종 Bern material/catalog/placement Publish와 Check가 통과했다. 23 shard·50,017배치·23,153재질 행을 실제 C++에서도 읽었다. 최종 Product 빌드도 통과했으며 사용자 화면 확인은 아직 수행하지 않았다.

원본 PS 입력을 동일하게 준 수치 검증은 원본 VS 전체, 원작 CPU SH·shadow setup, alpha blend 및 fog 합성 순서, 실제 frame 성능의 동등성을 입증하지 않는다. 물의 Baked PS 네 종류는 물 결과 G04에 따라 연결됐다. static shadow penumbraWidth 0.05는 PROJECT_ADAPTER이며, 원본 전체 후처리와 식생 바람은 이 결과의 완료 범위가 아니다. Client/UI를 실행·조작·캡처하지 않았다.

## G04. black·depth-modulate helper의 원본 대조

Root가 연결한 program64 black와65 shadow-modulate의 실제 C++ strict Configure2행을 검사했다. texture mask0,필수값 누락·추가값 거부,finite,실패 시 기존 입력 보존이 모두 통과했다. 원본 terminal Material과 Engine Default__Material CDO는 `out/BernHelperNative20260911/terminal_material_properties.json` 및 `material_defaults.json`으로 직접 추출했다. 두 terminal에 bAllowFog=false가 명시돼 있어 제품 black의 공통 fog 제외와 shadow의 identity fog 입력에 반영했다. shadow parent의 blend_modulate·mlm_unlit도 직접 확인했다. TwoSided override는 두 terminal/상위 Material CDO에서 발견되지 않았으므로 일반 back cull을 유지한다.

shadow native PS `4af23e405ecf044a83d669e6da977329`의 depth 역변환은 원본 연산 순서인 `1/(depth*scale-offset)`를 보존한다. float 대수식만 같은 다른 역변환 순서는 매우 좁은 depth fade에서 오차가 확대됨을 확인했다. 실제 helper와 원본 PS를 clipW3종·bias3종·power3종·fog4종·depth차8종의864 fixture로 대조했다. 비유한·실패0,최대 상대 차이8.941e-9이며 black alpha blend로 같은 배경색에 합성한 RGB도 일치했다. 제품 shadow는 원본 bAllowFog=false에 따라 이 helper의 fog 입력을1로 고정한다.

black native Base PS `5422741e276185408188f9fba72440fd`와 제품 검정 diffuse 출력은 zero diffuse 입력의9 fixture에서 RGB0으로 일치했다. 원본 shader 자체는 조명식을 포함하므로 unlit shader라고 분류하지 않는다. 원본 component는 lights 수용을 끄며, 연결이 없는 diffuse에 대한 native CPU cb2[3] setup을 독립 역분석했다는 뜻은 아니다. renderer별 GBuffer encoding 전체의 동등성도 이 검사에 포함하지 않았다. 근거는 `out/BernHelperNative20260911`의 native/제품 HLSL,JSON 결과와 packing probe다.

물 Baked 미연결 경계는 후속 [물 결과 G04](2026-09-11_BERN_NATIVE_WATER_IMPLEMENTATION_RESULT.md)에서 해소했다. 최종 Product 및 실제 사용자 화면 확인은 root 통합 결과와 사용자 판정을 따른다.


## G05. 실제 로딩에서 확인한 공통 문제

Renderer가 조명 패스 뒤 native material 시간을0으로 지워 뒤에 그리는 물과 하늘의 시간식이 정지할 수 있었다. 프레임 시작·opaque·light 종료·실패 정리에서 동일 presentation clock을 보존한다.

Product4의 컴파일은 통과했지만 실제 CShader 생성에서 Debug CSO 두 개가 기존256MiB 파일 크기 제한을 넘었다. AnimMeshBinary269,728,974 bytes,MeshBinary290,976,726 bytes였다. 세 native shader(AnimMeshBinary/MeshBinary/Deferred)는 Debug x64에서O1을 사용하고 중복 source debug 정보 내장을 끄도록 프로젝트에 반영했다. 일반 C++ Debug/PDB와 로더 검사는 유지한다. 이후 Product6 산출물 크기·CShader 생성·material bind까지 G06과 같이 통과했다.

공통 shader의 Engine→Client 복사도 검사했다. 이전 Engine SourceStoneSurface가 최신 Client overlay helper를 덮는 결함은 이미 원본 대조를 통과한 정확 함수 본문으로 양쪽을 일치시켰다. 컴파일 경고가0이라는 뜻은 아니며 native 분기의 FXC X4000/X4008과 기존 C++ encoding/PDB 경고는 남아 있다. 실행한 수치검사의 nonfinite0은 모든 frame에서 경고 영향이 없다는 증거가 아니다.


지정 경고를 별도 소스 경로로 확인했다. Base24/28/31/32의 X4000은 원본 alpha discard 직후 조기 반환이며 호출자는 MRT 사용 전에 discard한다. input aggregate 초기화,CB64 전체 bind,출력8필드 할당을 확인했다. Direct60/63 X4008은 literal0으로 꺼진 native shadow 분기 안에 있고 최적화 CSO에서 해당 scene-depth 입력도 제거된다. 기존 대응54 fixture/55,296픽셀 결과는 nonfinite·mismatch0이다. 이번 경고 조사에서 추가 GPU 실행이나 제품 변경은 하지 않았다. 다른 경고의 전체 안전성을 대신 판정하지 않으며 근거는 out/KoukuNativeWarningAudit20260911/audit.json이다.


## G06. 최종 Product 산출물 확인

Engine/Shared/Server/Client Debug Product가 모두 PASS다. 기록은 out/BuildPipeline/runs/20260910T185256023Z-debug-product.json과 out/KoukuFullRestore20260911/product-build-6.log다. 최종 CSO는 AnimMeshBinary1,502,234 bytes,MeshBinary3,639,678 bytes,Deferred1,310,636 bytes이며 기존256MiB 로더 제한 이내다. 최종 Boss/prop33모델50mesh의 실제 CShader 생성·CModel clone·material bind도 오류0으로 통과했다.

베른도 최종 Product6 Engine DLL과 Binary/MapInstance CSO를 실제 CShader로 읽었다. 81개 CModel·81개 material variant의 87개 source 조합과 143개 mesh material을 검사했고, native binding80회·lighting binding206회·special binding18회·Effects pass Apply206회가 오류0 및 native 종료 코드0으로 통과했다. 근거는 out/BernMaterialAudit20260911/cmodel-material-binding-product6-native.log와 material-binding-native-exit.json이다. 이 검사는 headless WARP의 실제 모델·재질 연결 검사이며 Draw나 전체 장면 렌더링 검사는 아니다.

해당 map/Boss JSON17개와 project/filter XML4개의 parse,공통 Engine/Client shader 파일 동일성,변경 범위 diff check를 확인했다. 베른 Area Publish/Check50출력과 원본 배치 transform 보존은 조명/기하 결과를 따른다. Server/Client process와7777 listener는 확인 시 모두 없었다. 실행은 사용자가 Server+Client profile의 Ctrl+F5와 Lobby Bern/KoukuSaydon을 직접 사용한다. Sequence Viewer에서 쿠크세이튼의 연출_팝업북/연출_1관문 피날레를 선택해 Play한다. 이 문서는 Client 화면의 최종 시각 일치를 PASS로 기록하지 않는다.

## G07. 사용자 직접 실행 이후의 성능·장면 복구 경계

사용자는 베른에 직접 진입해 전체적으로 개선됐지만 약 6fps이며, 이펙트 복원 때처럼 원작과 동일하게 복구됐다는 느낌은 아니라고 보고했다. 이는 입장과 일부 시각 개선에 대한 사용자 관찰이며 최종 visual PASS가 아니다. 이전 GPU 식 대조와 Product 바인딩 PASS를 장면 성능이나 원작 전체 화면 일치의 증거로 사용하지 않는다.

현재 authoring 50,017개를 실제 Is_BatchEligible와 assetId/반전 여부 기준으로 재집계한 정적 구조는 배치 대상 48,797개와 16,422그룹이다. 이 중 9,081그룹은 원소가 하나이며 16,330그룹은 RNM variant다. 이는 카메라별 실제 draw 수가 아니다. CMapStaticBatchObject의 frustum 검사는 Render에서 수행하며, authored-visible 인스턴스가 있는 15,745그룹이 그 전에 NONBLEND에 제출된다. 같은 모델의 geometry 공유는 메모리 중복을 줄이지만 배치별 assetId 분할로 인한 CPU 순회·제출 비용까지 제거하지 않는다.

조명도 실제 계측이 필요한 후보가 있다. Renderer.cpp의 원본 재질별 반복에서 Light_Manager가 해당 pass의 scene/transient 광원들을 처리하고 Light.cpp는 사각형을 그린다. forward는 실제 관련 광원 count만 반복하지만, 픽셀의 attenuation이 0이어도 native Direct 함수가 실행된 뒤 최종 감쇠를 곱한다. MapAssetRenderUtils는 해당 재질 바인딩마다 400 float4 배열 네 개, 합 25,600 bytes를 채워 전달한다. 이 경로들의 상대 비용은 CPU/GPU 프로파일 없이 확정하지 않는다.

초기 조사에는 이번 관찰에 대응하는 Profiler JSON이 없었다. 기존 Profiler의 Capture와 Save JSON을 사용자에게 안내했으며, 당시 최신 2026-08-25 기록은 현재 베른 분석에 사용하지 않았다. 이후 사용자가 저장한 새 기록의 실제 분석은 G09에 기록했다. 이번 단계에서 제품 코드는 변경하지 않았다. 사용자가 실행한 Client/Server가 동작 중인 것은 확인했으나 에이전트가 실행·UI 조작·화면 캡처를 수행하지 않았다.

동적 그림자가 활성화된 경우도 별도 계측 대상이다. MapStaticBatchObject의 shadow 인스턴스 구성은 authoring-visible 목록을 사용하며 카메라·광원 frustum으로 줄이지 않는다. 이 항목은 현재 활성 상태나 Render.Shadow의 실제 비용을 확인한 결과가 아니다. Profiler의 Render별 scope는 CPU 제출 시간이고 GPU query는 전체 프레임 시간이므로 동일한 측정값으로 해석하지 않는다.

## G08. 폭포·물보라와 정적 수면의 다른 연결 범위

원본 16개 level의 ParticleSystemComponent를 읽기 전용으로 다시 파싱한 결과 선언 1,300개를 확인했다. 이 중 물 관련 template 11종의 선언은 91개이며 waterfall_001/002, fall splash, fallmist 등이 포함된다. 예를 들어 SL00 export 788은 bfx_low_06.water.par_b_waterfall_002를 참조한다. 이 수치는 serialize된 component 선언 수다. 활성 streaming level, auto-activation, hidden, Kismet 제어와 카메라 가시성을 모두 판정한 실제 재생 수가 아니다.

현재 복구한 water17 MIC와 forward48 MIC는 정적 mesh 재질의 분모이며 이 particle 선언을 포함하지 않는다. Bern authoring/runtime에는 해당 Area의 mapeffects 문서가 없고 Level_Bern에도 MapEffectPresentationRuntime 호출이 없다. 따라서 정적 수면의 Base/Baked/Light 연결을 폭포·물보라·물안개 연출까지 복구했다는 의미로 사용하지 않는다.

기존 CEffectCatalog와 CEffectPresentationService의 원본 효과 재생 기능은 재사용할 수 있다. 다만 현재 MapEffectPresentationRuntime은 deploy/destruction/encounter 의존성과 Server presentation 갱신 경계를 갖는다. 베른의 지속 환경 효과는 원본 component 위치·시간·반복·활성 조건을 확인하고 이 기존 표현 경로에 실제 level 수명을 연결해야 한다. 파일만 추가하거나 원본의 모든 선언을 항상 재생하는 것으로 연결을 대신하지 않는다.

## G09. 사용자가 저장한 실제 베른 프로파일 분석

사용자가 Profiler에서 저장한 Client/Bin/ProfilerCaptures/profiler_20260911_044109_437_frame11498.json을 읽었다. 전체 1,200프레임 중 mapPlacements=50,017인 베른은 마지막 81프레임, 11418~11498이다. 진입 시 37.37초의 긴 프레임을 포함하는 전체 평균 대신 최근 30프레임 11469~11498을 별도로 분석했다. 이 창에도 가시 배치 수가 달라지므로 동일 카메라의 고정 benchmark로 분류하지 않는다.

최근 30프레임 중앙값은 CPU frame 326.282ms, Render.NonBlend 252.491ms, Client.Update 63.614ms, Render.Lights 1.122ms, Render.Shadow 0.012ms, Render.SceneHDR 9.131ms다. drawCalls 중앙값은 3,459회, 실제 runtime mapBatchCount는 16,421, fallback은 1,221이다. G07의 별도 정적 재집계 값을 이 runtime counter와 동일하다고 주장하지 않는다.

유효 GPU query 26개의 중앙값은 336.772ms다. 마지막 4개는 query read latency로 gpuValid=false이며 0ms 성능으로 해석하지 않는다. GPU timestamp는 프레임 전체의 제출 간격을 포함하고, CPU scope에는 D3D 호출과 가능한 driver wait가 포함된다. 따라서 이 기록만으로 CPU 연산과 GPU 포화 중 하나를 확정하지 않는다. droppedCpuScopes와 droppedGpuFrames는 모두 0이다.

이 기록으로 우선 조사할 구간은 불투명 맵의 Render.NonBlend다. 배치 분할, 재질/texture/constant 바인딩, instance culling/업로드, Draw 호출 및 driver wait를 좁혀야 한다. G07의 조명과 그림자 구조적 후보를 이번 프레임의 주원인으로 승격하지 않는다. 계산 결과를 보존하는 성능 수정과 수정 후 사용자의 같은 위치 기록 비교는 아직 수행하지 않았다.

분석 결과는 out/BernMaterialAudit20260911/user_capture_20260911_044109_bern30.json에 저장했다. 1,200/300/60프레임 평균을 기록한 user_capture_20260911_044109_summary.json은 서로 다른 장면과 진입 스파이크가 섞인 참고 자료이며 베른의 대표 성능으로 사용하지 않는다.

이후 사용자가 FPS 수정만 먼저 요청해 동일 인스턴스 재업로드 제거와 MapInstance 최적화를 반영했다. 구현·최종 빌드·사용자 후속 측정 경계는 [베른 FPS 결과](2026-09-11_BERN_FRAME_TIME_RECOVERY_IMPLEMENTATION_RESULT.md)를 따른다. 원본 사용자 캡처와 최근 30프레임 재계산은 C:/Users/user/AppData/Local/Temp/LostArkBernFps20260911의 profiler_20260911_044109_437_frame11498.json 및 user_capture_bern30.json에도 보존했다.

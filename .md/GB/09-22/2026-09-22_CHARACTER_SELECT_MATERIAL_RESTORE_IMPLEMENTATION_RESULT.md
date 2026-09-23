# Character Select 재질 연결과 PBR 진단 구현 결과

## G00. 실제 설치와 게시

현재 source material이 없었던 visible45배치(8개 physical model,15개 effective MIC)에
17개 catalog material variant와32개 named material row를 연결했다. 모델 파일 누락은
없었으며, 기존 D/N fallback과 원본 source material·배치별 RNM/environment 입력 사이의
연결 누락이 원인이었다.41배치는 기존 surface family,나머지4배치는 native208/209/210이다.

최신 디스크를 다시 읽고 stable source placement ID의 assetId만 병합했다. catalog209→226,
material247→279,placementLighting731→775이며 placement804는 그대로다. 적용 전후
804개 ID 집합,45개 변경행의 assetId 외 모든 token이 동일하다. 사용자 삭제
`editor:LV_LOBBY_CLASSSELECT_SL00:1`은 계속 없으며 TRS·visibility·editor2도 보존했다.

신규30 DDS를 `Client/Bin/Resources/Map/`과
`C:/Users/user/Desktop/GBResources/Map/`의 동일 상대경로에 설치했다.30개 모두
Resources·GBResources·apply receipt SHA256이 일치한다. 확보된 원본 mip을 보존했고,
원본 자체가1 mip인 statefx lookup을 임의 생성하지 않았다.

공식 Area Validate→Publish→Check를 모두 통과했다. runtime4파일이 게시되었으며
placement/material authoring과 runtime byte가 동일하다. 적용 당시 Client/Server는
실행 중이지 않았다. 파일 설치를 실행 중 메모리 Reload나 사용자의 화면 확인으로
대신 기록하지 않는다.

- 증거: `out/CharacterSelectMaterialRestore20260922/apply.receipt.json`
- 백업: 같은 디렉터리 `BeforeApply/`
- runtime placement SHA256: `3280a5da84c204f215659fc8d09d7aff92f65c0b7166555fcdde4b1daacc8752`
- runtime material SHA256: `2140b73069e2bf0eea90645be049abe910feca8ce766641134529c15172fb106`

## G01. 원본 native 연결과 남은 입력

208은 sky color-mask,209는 translucent emissive/reflection,210은 masked real-PBR
permutation이다.209는 기존 source-map forward light/RNM 경로를,208/210은 공통
source-character static map 경로를 사용한다.208/210 공통 dispatch·registry·project
등록은 장비 작업과 단일 소유로 통합했다.209의 forward/baked 분류는 Catalog,
MapAssetRenderUtils,Material 및 publisher까지 연결했다. 신규 HLSL leaf는
`Shader_SourceMapCharacterSelectPrograms.hlsli`, `Shader_SourceMapCharacterSelectDirect.hlsli`다.

원본 shader cache의 정확한 MIC/VF/permutation을 연결했지만, 미확정 engine SH,
hemisphere/native BRDF 입력까지 원작과 일치한다고 주장하지 않는다. source wind
flag는 보존했으며 기존 정적 surface 경로의 vertex wind 변형 연결은 별도 미완료다.

## G02. 노란색 진단과 실제 수식 수정

중앙 FLOOR12의 diffuse texture 평균은 sRGB `(0.631,0.541,0.377)`,linear
`(0.356,0.254,0.118)`로 이미 따뜻하다. source diffuseColor도
`(1,.938744,.801603)`이다. 중앙 RNM atlas 영역 평균은
`(.7622,.7586,.7549)`로 거의 중립이며,원본 지정 HDR01 cube의6면 mip0와
material reflection은 평균 B>G>R이다. 이는 texture 통계이며 화면 기여율이 아니다.
RNM gamma나 IBL 자체의 노란 tint를 단일 원인으로 확정하지 않았다.

원본 ARCH01A/FLOOR12 `_02`의 실제 RNM BasePass DXBC와 동등성을 검증했던
`SourcePBRIndirectCandidate.hlsli`를 현재 제품 함수와 대조했다. 기존 부분식은
`albedo * RNM * multiBounceAO`를 더해 `(1-metallic)`과`(1-reflectionBRDF)`를
누락하고 있었다.두 에너지 보상을 복구했으며,lookup 없는 행은 metal mask만 적용한다.
`legacyUnbalancedRNM=1`로 이전 식을 정확히 비교할 수 있다. 현재 중앙 floor336의
metallic은0, brown floor371 및 bridge410/411은0.5019608이다. 따라서 metal mask
복구는 뒤 세 계열의 RNM diffuse를 약49.8%로 줄인다. 전체 노란색의 최종 화면 원인이
이 하나라고 확정하거나 캐릭터까지 같은 PBR 경로라고 설명하지 않는다.

source SH색·hemisphere와 원본 BRDF payload는 여전히 미확정이다. 테스트용 중립 SH를
실제 scene 값으로 넣지 않았다. tone/LUT 누락을 확정하지 않았고 조명색도 바꾸지 않았다.

## G03. Rendering Benchmark shader 계약

`g_MapPBRContributionScale=(directDiffuse,directSpecular,RNM,IBL)` 기본값은1이다.
`g_MapPBRDiagnosticParameters=(normalScale,roughnessOffset,legacyUnbalancedRNM,0)`는
기본 `(1,0,0,0)`이다. 표면 normal/roughness 조절은 recovered map PBR만 적용한다.
기존 marker3 RT7에는 RNM+IBL,RT6에는 IBL을 저장한다. 새 RT는 없으며 기존
indirect/진짜 emissive 분리와 moving-caster shadow 처리를 보존했다.

- view8 RNM: RT7−RT6,view9 IBL: RT6,view10 direct diffuse:albedo×Shade.
- view11 SceneHDR: 캐릭터와 배경 전체의 Bloom/exposure/tone 적용 전.
- view12 SceneTone: 현재 Bloom+exposure 및 sourceTone/Hable 뒤,grading 전.
- view13 SceneGraded: 현재 Resolve_FinalLDR 뒤,FXAA 전.

8~11의 HDR 진단은 RGB 전체에 같은 maxRGB 분모를 사용하고 display gamma를 적용한다.
12/13은 실제 장면 색 변환 결과다. marker 제한이 없는11~13으로 캐릭터·배경의 공통
색 변화 단계를 구분할 수 있다. C++ settings/바인딩/ImGui Benchmark는 상위 작업이
통합하며, 이 문서의 shader 구현만으로 사용자 화면 표시 성공을 주장하지 않는다.

## G04. 검증과 미완료 구분

- 공식 map publisher Validate/Publish/Check PASS,804배치·runtime4파일.
- 설치30 DDS SHA 일치,45 placement asset-only 변경,279 material JSON parse PASS.
- 최신 instanced map effect FXC PASS.
- 최신 whole-scene diagnostic Deferred effect FXC PASS; Engine/Client mirror byte동일.
- 현재 제품 PBR indirect tail을 추출하고 원본 DXBC와 비교한 WARP64조건 PASS,
  max absolute error8.9407e-08,nonfinite0. SH=중립,hemisphere/extraAmbient=0을 명시적으로
  통제한 fixture이며 source owner 값이나 원본 장면 전체를 검증한 것은 아니다.
- legacy 선택과 기존 부분식 WARP64조건 비교 PASS,max error0.
- `git diff --check` PASS. 기존 FXC warning은 유지되어 있으며 error는 없다.

Binary full effect(새 forward209 포함)도 FXC PASS다. 현재 source로 CMapAssetCatalog와
관련 loader를 새로 컴파일하고,17:46 Engine DLL을 별도 비UI probe에서 사용하여 실제
게시 catalog→CModel/Create_MaterialVariant→DDS SRV load를 실행했다.17variant/32개
material override 모두 성공,실패0이며 native208/209/210을 포함한다.
`ModelProbe/consumer-result.json`과`final-validation.json`에 결과·DLL/source hash를 기록했다.
새 settings/Benchmark를 포함한 최종 Product build는 상위 작업이 별도로 관리한다.
Client/UI는 실행하지 않았다. 최종 색감·그림자·의상·문양의 사용자 화면 판정은 미완료다.

중앙 물방울/삼중곡선 문양의 원본 carrier는 미식별이다. FLOOR12·BRIDGE의 원본 LOD
geometry/UV1 자체 누락 근거는 없으며 Module488은 존재한다.11개 remote floor/star는
실제 PLAYER_CLASS Matinee/CameraActor와 연결되지만 현재 중앙 문양 carrier의 증거는
아니다. 사용자 최신 우선순위와 다른 세션 전담 지시에 따라 이 조사를 중단했고 임의 geometry 추가나 삭제 복구는
수행하지 않았다.재질45배치 완료와 이 미확정 경계를 분리한다.


## G05. 통합 Benchmark와 공통 색 변환 검증

Recovered map materials 아래의 최근32개 재질 나열 표를 제거하고 Benchmark의
Pixel rendering inputs로 통합했다. 전체 합성3단계 보기, 유효한 scene tone/grading,
Bloom/SSAO/FXAA, light receiver/character ambient, shadow/fog 입력을 실제 runtime
설정에서 표시한다. 선택한 한 PBR surface는 성공한 draw binding의 복사본으로
base/normal/ORM/reflection/RNM/environment/emission 수치를 보여 준다. instancing stream의
RNM 수치는 CPU snapshot에서 얻지 못했을 때 미샘플 상태로 명시한다.

비교 조절은 direct diffuse/specular, RNM/IBL, normal multiplier, roughness offset 및
이전 RNM 식이다. 유효 범위는 유한 gain/normal[0,4], roughness offset[-1,1], legacy0/1,
reserved0이다. 적용 전 전체 검증 실패 시 이전 상태를 유지한다. 현재 Level에서만 소비하고
Workbench 종료/Level 전환 시 진입 material 설정 전체를 복구한다. Capture A/B도 같은
소유권에 등록하며 시작 실패 때 즉시 이전 설정을 유지한다. 캡처 fingerprint에 새 수치들을
포함했고 캡처 중 조절은 잠근다. 실제 GPU 픽셀값 readback이나 사용자 화면 검증은 아니다.

Renderer·RenderingBenchmark·MapAssetRenderUtils·MainApp 4 TU 개별 obj 컴파일 PASS.
실제 Apply_MaterialRenderSettings 메서드를 추출한115조건 검증 PASS: NaN/Inf/범위/예약값,
이전값 보존, Level scope 포함. 로그는 PixelCompile 아래에 있다. peer 검토에서 발견한
MRT 정리 우회와 SourceMaterials 토글 복원 누락을 수정했다. 정식 Engine obj는18:19:38,
DLL은18:19:48로 해당 Renderer 변경 이후이며 전체 Product는 진행 중이다.

공통 색 변환 read-only audit는 현재5개 HLSL 함수가 기존 native DXBC 동등성 검증본과
동일함을 확인했다. 현행 BGRA8 LUT bake/read WARP858입력(회색138)의 최대 RGB 오차는
0.0001437068로8-bit 한 단계의0.0367배였다. SceneHDR=FP16, LUT=BGRA8 UNORM,
backbuffer=RGBA8 UNORM 및 올바른 BGRA packing을 확인해 중복 hardware sRGB와 channel
swap 근거를 찾지 못했다. 이는 현재 코드/저장 profile 경계이며 실행 중 draft·최종 화면,
조명/재질/큐브 샘플 기여까지 배제한 검사는 아니다. 증거는 out/SharedColorAudit20260922다.

## G06. 미연결 원본 환경 입력과 중앙 문양 경계

원본 HDR01/HDR02 SH9 RGB는 확보돼 있다. 그러나 현재 RENDER_ENVIRONMENT_STATE와
CMaterial 공통 입력은 cube/color/rotation만 전달한다. native realPBR program9의
SH7-row 및 hemisphere slots는0이고 map 부분식은 그 SH 곱을 생략하므로 두 소비자가
같은 간접광 비율을 쓰지 않는다. 이 누락은 확인했지만 전체 노란 분위기의 기여량까지
확정하지 않았다. SH basis·정규화·convolution·좌표 변환과 동적 hemisphere owner는
복구가 남아 있다. 원본에 값이 있다는 사실만으로 임의 packing을 제품에 넣지 않는다.

중앙 문양은 사용자 지시로 다른 세션 전담이다. remote11 floor/star는 UE3 Matinee와
선택 카메라에 연결된 연출 무대이며 현재 중앙 문양 원본으로 확정하지 않았다. 원본
SL00 packageVersion868/licensee16,1112exports/479imports를 실제 파싱했다. UE4라
UModel이 못 읽는 상황이나 검은색 자체가 복원 불가능한 증거로 처리하지 않는다.

## G07. 사용자 마무리 요청 시점의 확정 상태

사용자의 정리 요청에 따라 새 조사와 추가 전체 binding 검사는 여기서 마무리한다.
Benchmark의 실제 진입 위치는 F1 → Rendering Workbench → Benchmark →
Pixel rendering inputs다. 최근 재질 나열은 제거했고 실제 합성 단계 보기와 유효 입력,
map PBR 기여 비교를 남겼다. 전체 노란 분위기의 해결 완료나 원작 화면 일치를 주장하지
않으며, 원본 SH/hemisphere owner와 native BRDF payload의 복구는 남아 있다.

정식 Product는19:04 전체 PASS 기록이 있다:
`out/BuildPipeline/runs/20260922T100454659Z-debug-product.json`.
그 뒤 최신 동시 변경과 생성 파일 정규화를 포함한 정상 Product를19:06에 시작했다.
이 마지막 실행은 정리 시점에 Engine/Shared/Server PASS, Client FXC 진행 중이다.
최신 전체 링크 성공으로 기록하지 않으며 실행을 강제 종료하지 않았다. 로그는
`out/CharacterMaterials20260922/product-final-build.console.log`이고 종료 결과는
공식 build pipeline이 `out/BuildPipeline/runs`에 기록한다.

생성 파일28개는 최신 외부 프로그램을 보존한 채 정규화했다. 전체 expanded Base/Light
byte와24개 guard 선택의 token이 전후 동일하고 실제 재실행 검사3/3 PASS다.
장비337모델·549 native slot 생성/clone/잘못된 교체337건 거절은 완료했지만,
최종 전체 shader binding 검사는4/337에서 정리 요청으로 종료해 미완료로 기록했다.
이는 실제 map17variant/32override 소비자 검사와 별도의 장비 전체 검사다.
자세한 범위는 같은 날짜의 CHARACTER_EQUIPMENT_MATERIAL_RESTORE_RESULT를 따른다.

마지막 JSON6/XML4 parse와 변경 범위 diff-check PASS다. 전달 리소스1,201개는
19:25에 Resources/GBResources 전체 hash를 다시 대조했고 불일치0, 앞선 receipt와
변경0이다(`delivery-close-check.json`). Client/UI는 실행하지 않았다.


## G08. before-restoration.v1 버튼과 적용 범위 (2026-09-23)

Rendering Workbench → Benchmark → Rendering restoration에서 기존 Before 대신
`before-restoration.v1`을 명시한다. 이 버튼으로 baseline을 적용한 뒤 같은 버튼의
`Return from before-restoration.v1`을 누르면 Workbench 진입 profile로 복귀한다.
복원 source profile 버튼은 활성 상태를 표시하고, 두 profile의 full ID를 보여 준다.
별도 Live rendering comparison이 켜져 있으면 Reset comparison 경로도 알린다.

기존 profile transaction을 그대로 사용하므로 오류 시 현재 renderer를 보존하고,
Capture 중 변경은 잠근다. 도구 종료·외부 scene owner/Level 변경 시 기존 복귀 경계를
유지한다. 저장 scene light·environment·shadow·fog·post-process 선택이며 source material
식·render pass·map light 배치·Effect 자료를 과거 버전으로 교체하지 않는다.

수정은 기존 `RenderingBenchmark.cpp`의 UI 선택부뿐이다. ASCII/UTF-8(BOM 없음)과
CRLF를 유지했다. 기존 vcxproj/filter 등록을 확인했고 새 파일 등록은 없다.
Debug RenderingBenchmark TU를 기존 include/define 조건으로 컴파일해 exit0을 확인했다.
로그와 실행 명령은 `out/RenderingWorkbenchBaseline20260923/`에 있으며 포함된 기존
CP949 헤더의 C4828 경고가 있었으나 error는 없다. 변경 범위 `git diff --check` PASS다.
UI만 바뀌므로 새 테스트 하네스는 만들지 않았다. 전체 Product 링크와 실제 버튼 클릭·
사용자 화면 판정은 이 TU 결과에 포함하지 않는다. Data/Resources/게시본은 바꾸지 않았다.


## G09. cubeDiffuse scene profile carrier (2026-09-23)

`RenderingProfileService.h/.cpp`와 rendering publisher에 선택적
`environment.cubeDiffuse`를 연결했다. model은 `RGBM6_LAMBERT_SH3`, intensity0~4,
7행의 finite float4 성분−64~64와 마지막w=0을 요구한다. 원본 cooked RGBM6 cube를
적분한 프로젝트 diffuse 근사이며 native SH9 packing이나 원작 scene 값의 확정으로
기록하지 않는다. 실제 Data/Resources/runtime 문서는 이 carrier 작업에서 바꾸지 않았다.

presence flag로 미선언과 명시적인 intensity0/SH0 block을 구분한다. parser가 전체를
검증한 뒤 staged catalog를 교체하고, Validate_Profile도 같은 범위를 검사한다.
명시 block 없이 nonzero intensity/계수를 넣거나 cube 경로를 제거한 메모리 profile은
거절한다. Serialize_Catalog는 명시 block의7행·intensity를 보존하고 미선언에는 추가하지
않는다. Stage_RenderEnvironment 성공 뒤 stagedEnvironment의 vDiffuseSH와
fDiffuseIntensity를 채워 기존 Commit_Resolved transaction에 전달한다.

RenderingProfileService Debug TU 컴파일 exit0, 변경 범위 diff-check PASS다.
`Tools/RenderingPipeline/test_publish_rendering_profiles.py`의 cubeDiffuse roundtrip,
invalid rollback 및 기존 environment rollback3테스트가30.629초에 PASS했다.
optional, 명시0, intensity1/4와 음수계수·경계64를 왕복했고 model/field/행 수/행 길이/
비수치/비유한/범위/미세한 reserved w 오류26종에서 이전 게시 byte를 유지했다.
테스트의 Publish는 임시 디렉터리만 사용했다. 실제 authoring/runtime 게시본은 보존했다.

컴파일·테스트 로그는 `out/RenderingWorkbenchBaseline20260923/`의
`RenderingProfileService.log`, `cube-diffuse-publisher-tests.log`다. C++ Save를 직접
실행한 검증과 실제 shader/GPU 소비·Product 전체 링크·사용자 화면 판정은 이 carrier
검증에 포함하지 않는다. 새 H/CPP가 없어 프로젝트와 filter 등록 변경은 없다.

## G10. 캐릭터 선택 cube diffuse 소비와 근거 정정 (2026-09-23)

확보한 FLOOR12_02 BasePass `82f66791c7d2d249b9c993b51ca86845.asm`의106행은 albedo의
mad_sat다. 같은 PBR family의 근거로 현재 albedo 0~1 제한을 제거하지 않았다. main336의
FLOOR12_01과 동일 shader binary라는 확인이나 clamp 앞 엔진 색 계수의 복원까지 검증한 것은
아니다(G12). 원본 PS의 WorldInfo1101 → BG_PCSELECT02 CharacterCloseupScene/Uber1736
정적 후보는 color LUT가 비어 있어 neutral을 유지했다. 전체 원본 camera/UI의 활성 후처리
체인까지 neutral이라고 확정한 것은 아니다. “LUT 부재/클램프=결함”을 그대로 채택하지 않았다.
하늘 geometry와 별도로 기존 HDR01 specular cube는 이미 연결되어 있었다.

source serialized SH9의 native7행 packing·dynamic hemisphere owner는 여전히 미확정이다.
이 값을 추정해 원본 복원으로 표시하지 않았다. 대안은 실제 원본 cooked HDR01/02를
linear RGB*A*6로 해석하고 exact solid-angle 적분한 Lambert E/pi의 L2 근사다.
HDR01 바닥(+Y) irradiance/pi는(.341692,.377373,.438266)이며 diffuse blue 기여가 있다.
136법선의 직접 Lambert 적분 대비 HDR01 최대절대오차.009427/RMSE.003568/최대상대3.06%다.
HDR02 어두운 방향은 상대오차가 커 후보자료에만 보존했고 이번 profile 후보에는 넣지 않았다.
원본 source SH9는 이 계산에서 재해석하지 않았다. 근거는 out/RenderingIndirect20260923이다.

Engine RenderEnvironment는7행과 intensity를 전달하고 Renderer combine에서 한 번 바인딩한다.
map PBR marker3에만 albedo×(1-metallic)×(1-F0)×materialAO를 곱해 기존 RNM/IBL에 더한다.
이는 프로젝트 근사이고 RNM에 구워진 sky 기여 중복 가능성이 남는다. 추가 diffuse에는
직접광 그림자/dynamicBakedShadow를 다시 곱하지 않고 screenAO와 기존 fog를 적용한다.
원본 native 캐릭터 program의 SH 슬롯, 기존 RNM/IBL 및 실제 emissive는 바꾸지 않았다.

현재 게시 FLOOR12 주 바닥은 source `LV_LOBBY_CLASSSELECT_SL00:export:336`, stable
placement ID `12143455122424954600`, asset
`MAP_54B7E9F1F9F0_BG_ELG_ARYANORB_FLOOR12_SM_WINGART_OVR_42610589A3EB`다.
설치 WModel 48,784bytes를 읽어 실제 slot `SLOT_000_bg_elg_aryanorb_floor12_mi_wingart`와
게시 override의 일치를 확인했다. `bg_base_pbr_opa`는 MapAssetCatalog에서
`PBR_OPAQUE(4)`로 해석되고 Loader의 materialOverrides와 MapAssetRenderUtils를 거쳐
`g_SurfaceProgram=4`로 바인딩된다. 일반/instance shader 모두 `Depth.w=3`을 기록하므로
새 cube diffuse 소비 조건을 충족한다. FLOOR12 export371과 bridge410의 slot0,
bridge411의 두 slot도 같은 PBR marker3 경로다. 사용자 source materials 토글이 OFF이면
기존 fallback으로 돌아가며 이 기여도 적용되지 않는다. 이는 현재 Data·설치 slot·소비 코드의
연결 확인이고 실행 중 GPU marker readback이나 사용자 화면 판정은 아니다.

Benchmark는 `PBR cube diffuse sky (project approximation)` 단독 보기와
`Cube diffuse sky contribution` session gain0~4를 제공한다. 값0은 이전 간접광과 비교한다.
profile 기본값0, 현재 Level gate·capture fingerprint·도구 닫기 복구를 유지한다.
`before-restoration.v1`은 이전 저장 scene profile로 돌아가며 material/pass 코드 회귀가 아니다.

### 실행한 검증

- Renderer/RenderingBenchmark/RenderingProfileService Debug TU compile PASS.
- 최신 shared Deferred fx_5_0 /O1 compile PASS; 기존 X4000 warning은 유지.
- 실제 HLSL 두 함수와 Texture2D.Load를 사용한 비UI WARP15,552조건 PASS.
  법선9(0포함), 회전4, gain0/1/4,metal0/.5/1,F0 clamp,AO/marker 조건을 포함한다.
  독립 SH9 CPU oracle 대비 irradiance 최대오차2.38419e-7, 최종diffuse1.19209e-7.
  nonfinite/negative0,gain0·비PBR12,960조건 추가량 정확0과 baseline bit 보존.
- 실제 HDR01 +Y,albedo.5/F0.04/gain1의 추가 RGB(.164012,.181139,.210368),B>G>R.
- Publisher 후보 Validate PASS, 기존 및 신규 rollback 테스트 PASS(G09).
- 관련 project/filter XML4 parse 및 git diff --check PASS. 신규 등록 파일 없음.
- Engine/Client Deferred mirror byte 동일. Client/UI 실행·캡처는 하지 않았다.

### 승인 후 반영·게시와 남은 빌드 확인

사용자가 “저장 완료, 종료·반영·빌드 진행”을 명시적으로 승인한 뒤 최신 저장본을
다시 읽고 revision77→78로 반영했다. `scene.character-select.source-rendering.v1`과
`scene.character-select.warm-high-key.v1`의 HDR01 environment에만 cubeDiffuse
intensity1을 병합하고 공식 rendering publisher로 runtime 게시를 완료했다.
baseline/customizing-dark, LUT/tone, light, shadow, fog와 나머지 profile·무관한 필드는
보존했다. 교체 직전 CAS hash 재확인, 백업, 원자적 교체를 완료했으며 증거는
`out/RenderingIndirect20260923/apply.receipt.json`의 `APPLIED_AND_PUBLISHED`,
`unrelatedFieldsPreserved=true`와 source/runtime 전후 SHA256이다. 실제 반영은
실행 중 메모리 Reload나 사용자 화면 확인을 뜻하지 않는다.

승인 시 사용자가 Client(PID52376)와 Server(PID34340)를 이미 종료했고, 남은
Client(PID50660)는 승인 범위에서 종료했다. 사용자 Visual Studio 빌드와 겹친 에이전트의
중복 빌드만 중단한 뒤 기다렸다. 최종 제품 설치 확인은 아래 G11에서 이어서 기록하며,
사용자 화면 판정은 아직 이 문서의 검증에 포함하지 않는다.
유령 발탄 변경의 실제 모델검사는 대응 GHOST_VALTAN/VALTAN_EDITOR_VISIBILITY
RESULT에서 별도로 기록한다. 일반 발탄 재질 변경으로 설명하지 않는다.

## G11. 사용자 빌드 후 설치·실행본 검토 (2026-09-23 11:24 KST)

사용자가 Visual Studio 빌드를 마치고 EXE를 실행한 뒤 추가 빌드 대신 적용 검토만 요청했다.
Client.exe는11:22:22 링크본이고 실행 중 PID32192는11:22:57에 시작했다. 로드한 Engine.dll은
이 저장소 Client/Bin/Debug의 실제 배포본이며 Engine/Bin/Debug와 SHA256이 같다.
관련 CPP의 제품 OBJ도 마지막 소스 수정 이후 생성됐고 EXE 링크가 그 뒤에 수행됐다.
ClientStartup.user.log의 해당 PID는11:22:58 Rendering.Load_Runtime 성공,
11:23:01 Initialize ready를 기록했다. 이는 catalog 로드 성공이며 현재 선택 profile의
GPU 화면이나 Workbench의 클릭 결과를 읽었다는 뜻은 아니다.

추가 Product 빌드는 하지 않았다. 공식 Product -SkipBuild 검사는 PASS이며 필수 runtime
누락·유효성 오류는 모두0이다. 증거는 out/BuildPipeline/runs/
20260923T022445050Z-debug-product.json이다. 이는 설치 검증 영수증이지 에이전트가 새
전체 빌드를 수행한 영수증은 아니다. 실행 중 Client/Server를 종료·Reload·조작하지 않았다.

제품 Engine/Client Deferred CSO는 SHA256이 같고 G10에서 검증한 독립 /O1 Deferred CSO와도
byte-identical이다. EXE에는 before-restoration 복귀 버튼·cube diffuse 비교 UI·profile parser가,
실제 로드 DLL/Deferred에는 새 cubeDiffuse 입력이 포함됐다. 산출물 SHA와 문자열·동일성 근거는
out/RenderingIndirect20260923/product-artifact-review.json에 보존했다.

유령 발탄은 실제 배포 DLL 및 base+14개 animated cohort CSO와 SHA256이 같은 격리 복사본으로
CShader::Create의 전체 variant admission을 통과했다. base program84/pass16은 blend0,
depthWrite1, pixelShader 유효이며 pass17은 blend0/depthWrite1/PS 없음이다. 직접 shard84의
pass16은 성공하고 base 소유 pass17은 의도대로 거부한다. 새 정책의 실제 제품 로딩 계약을
확인했으며 Client/UI 창이나 draw는 생성하지 않았다. 증거는
out/GhostOpaque20260923/admission-debug/{product-inputs.json,compile.log,probe.log}다.

### 장시간 셰이더 빌드의 확인된 원인과 남은 경계

현재 branch/HEAD는 codex/kouku-gate3-bingo-flow-0923 / ff868f0ade다. 마지막 checkout은
03:06:27이며 이번 검토 중 branch 전환·Clean·Rebuild는 하지 않았다. 실제 FX dependency
tlog에서 공통 Deferred 변경은 Engine15개, 공통 SourceCharacterForward 변경은 Client의
animated/static30개 CSO에 전파됐다. 새 ghost PS가 non84 cohort에서도 forward dispatch를
다시 생성하는 불필요한 컴파일 비용을 확인했다. runtime discard는 컴파일 시 코드 제거를
보장하지 않는다. 최적화 산출물의 ghost PS 합계는15개 FX에서1,980,904bytes이며,
animated84의 이번 /O1 컴파일은10:58:46~11:18:31에 수행됐다.

static84에는 ghost PS가 추가되지 않았지만 공통 include 변경으로10분29초간 재컴파일됐고,
완성된 CSO는 이전 것과 byte-identical이다. 이는 실행 오류와 별개인 빌드 비용이다.
non84의 compile-time stub과 native84 direct light 호출로 비용을 줄이는 수정은 이번
최종 요청의 적용 검토 범위에서 수행하지 않았다. 소스 변경 없이 현재 제품을 확인했다.
원본 serialized SH9/native hemisphere의 정확 복원과 사용자 화면 판정도 계속 미완료다.

## G12. 사용자 화면 판정: 캐릭터 선택 색 복원 실패

G11의 제품 파일·로딩 검증 뒤 사용자는 유령 발탄 표시 성공과 캐릭터 선택 맵의 색 복원 실패를
명시했다. 첨부 codex-clipboard-2aa83b44-0d95-4709-845c-2723157731c9.png의 바닥은 여전히
원본 비교의 회백색과 다르다고 판정했다. 따라서 캐릭터 선택을 시각 복원 완료로 처리하지 않는다.

이번 변경은 colorGradingLut를 새로 복원한 것이 아니다. 두 복원 profile의 LUT 경로는 빈 값이고,
기존 UE3_CUSTOMIZABLE tone 설정을 유지한 채 원본 cube 기반의 프로젝트 diffuse 근사만 추가했다.
native packed SH 색의 곱과 원본 hemisphere owner 입력은 계속 미연결이다. G10의 중립 albedo
fixture에서 추가광의 B>R을 확인한 결과를 실제 갈색 FLOOR12의 중립색 복원 성공으로 확장할 수 없다.

검토 중 사용자 Save로 authoring/runtime이 revision79가 됐다. 두 JSON은 서로 같으며 rev77
백업 대비 허용한 cubeDiffuse 추가, revision, 생략된 shadow.dynamicBakedStrength=0 명시와
float32 저장 차이 외에27개 profile의 의미 변화는 없다. rev78 적용 영수증의 최종 SHA와는 다르므로
동일 SHA라고 기록하지 않는다. before-restoration, LUT/tone/light/fog와 두 cube intensity1은
보존됐으며 사용자 저장본을 덮거나 재게시하지 않았다.

현재 선택값은 Workbench의 Rendering restoration 아래 Active profile과 session 비교 gain으로
확인해야 한다. 상단 Scene profile은 편집 draft ID이며 현재 renderer 적용 ID를 보장하지 않는다.
시작 로그는 catalog 로드까지만 보여 준다. source profile 정적 근거와 전체 원본 camera/UI/volume
후처리의 실제 활성 체인은 구분해 재검토 중이다. 이 조사 중 코드·profile 수정이나 추가 빌드는 하지 않는다.

### 재검토로 확정한 색 입력과 LUT 경계

실제 설치된 FLOOR12_01 diffuse1024²와 ORM64²를 읽었다. 원본 saturation.35,
diffuseColor(1,.938744,.801603), brightness2를 적용한 reflection0 fixture의 평균 albedo는
(.600465,.496761,.347859), R/B1.72617이며 clamp에 걸린 texel은0%다. 실제 ORM은 균일하고
AO1/metallic0/roughness.627451/F0.04다. 여기에 현재 +Y cube diffuse를 더하는 항만 계산하면
(.196967,.179966,.146357), R/B1.34580으로 여전히 따뜻하다. 이는 실제 카메라의 반사·RNM·
후처리를 포함한 화면 전체 측정이 아니다. 입력 DDS SHA·원본값·계산·경계는
out/RenderingIndirect20260923/floor12-material-color-audit.json에 보존했다.

확인한 원본 PBR 식의 SH color×albedo×(RNM+hemisphere)와 현재 별도 additive cubeDiffuse는
같은 복원이 아니다. source diffuse/reflection의 sRGB와 normal/ORM/RNM의 linear 연결은
유지되며 marker3에 일반 g_ColorTint가 중복 적용되는 경로는 발견하지 않았다. 노란색 전부를
LUT 미사용이나 clamp 하나로 설명하지 않는다. 확인된 원본 FLOOR12_02의 mad_sat 앞에는
selectioncolor와 엔진 cb2[3].w 배율/cb2[3].rgb 오프셋도 있으며 이 입력의 전체 복원은 미검증이다.

LUT 독립 감사에서는 다른 정적 owner도 확인했다. LV_LOBBY_PS export151은
lv_atm_border.tex.lv_atm_border_lut를 참조하지만14개 정적 volume 전부 기존 CS focus/eye
근사 좌표 밖이다. CameraActor28개의 명시적인 nonneutral LUT도 확인하지 못했다. 따라서
이 다른 LUT를 현재 바닥에 적용해야 한다는 근거가 없으며 UI native·scene 전환·camera override의
전체 활성 경로는 미검증이다. 정확한 결론은 “확인한 정적 후보는 neutral이고 다른 활성 LUT는
아직 확인되지 않았다”다. 원본 전체의 neutral 확정이나 LUT 복원 완료로 확장하지 않는다.


## G13. 원본 PBR 간접광 코드와 입력 owner 복원

현재 소스에 원본 간접광 carrier와 consumer를 반영했다. 제품 데이터/리소스 최종 게시와
Product 빌드 결과는 아래 후속 적용 기록에서 구분한다. 전체 반영 코드는
2026-09-22_CHARACTER_SELECT_NATIVE_INDIRECT_DETAIL_PLAN.md에 보존한다.

### 원본 근거와 적용 범위

- 현재 RefShaderCache에서 FLOOR12_01의 exact static key를 join했다. source PS82f66791과
  native binder를 기준으로 SH9→packed7, cube color.w=0, sin/cos 회전, sky/ambient 입력을 복원했다.
- EFEngine SH packer0x5f0c30은286개 입력 및 추가 default cube2개에서112bytes가 bit-exact다.
  TextureCube IncidentLightingSH가 resource를 거쳐 packer로 전달되는 owner도 확인했다.
- 원본 BRDF는128×32/128sample/RG16_UNORM이다.4096×2채널 모두 native 수학 slice와
  bit-exact이며 기존 프로젝트128×128 lookup과 별도 리소스로 유지한다. color-grading LUT와 다르다.
- 실제 원본 component/view/CDO/native SetMesh를 join한181개 사용 PBR 행은 HDR01 6개,
  HDR02 12개, engine default PBR_Cubemap_00 163개다. 제품의 기존 environment17개만
  보면 놓쳤던 export501의 명시적 HDR01도 확인했다. 미배치 base arch1개는 수정하지 않는다.
- 원본586 placements 중562는 Sky1087이 이미 baked RNM에 포함되고24는 lighting channel이
  달라 추가 hemisphere가0이다. .15를 전역으로 주입하지 않는다. 정상 lighting view의
  ambientAndSkyFactor는(0,0,0,1)이며 native minimum roughness는0.045다.

sourceIndirect가 있는 PBR과 profile.useSourcePBRIndirect=true를 모두 만족해야 원본 cube,
BRDF, SH, color/rotation을 쓴다. 이전 모드는 이전 SRV와 식을 보존한다. 새 environment의
legacyEnabled=false는 이전의 환경 없음 상태를 보존한다. native 모드에서 기존 cubeDiffuse
추가 근사를 억제한다. Workbench는 입력 가용 여부와 profile 활성, 색/회전/sky를 표시한다.
독립 검토에서 nullable surface 접근을 발견해 null guard와 hasEnvironment 내부 참조로 수정했다.

### 실행한 검증

- Client RenderingProfileService/RenderingBenchmark/MapAssetCatalog/MapAssetRenderUtils,
  Engine Model/Material/Renderer의7개 Debug x64 TU compile PASS. nullable 수정 TU 재compile PASS.
- 최종 Shader_VtxMeshBinary fx_5_0 /Od compile PASS. 제품 /O1 빌드는 별도 기록한다.
- source FLOOR12 DXBC 대 후보 간접광 식의 WARP64×64pixel 비교: 실패0/NaN0,
  최대오차4.47035e-8. native off 대 이전 식: 실패0/NaN0, 최대오차2.98023e-8.
  fixture는 양쪽의 TBN과 tangent-up을 같은 직교 basis로 맞췄다. 초기 비일관 fixture의
  오차를 숨기지 않고 fixture-basis.json에 원인과 수정 범위를 기록했다.
- Python 기존/신규15 tests, Python/PowerShell/C++ 환경계약152 cases 및 직접 override214 cases PASS.
  전체 publisher AST parse와 잘못된 native 필드에서 이전 output/receipt 보존 확인.
- Rendering profile 선택값의 미선언/false/true roundtrip과6가지 잘못된 형식의 publish rollback PASS.
- 실제 FLOOR12 WModel UV0/UV1의32,768 면적 가중 표본에서 HDR01 SH 곱은 R/B를 줄인다.
  controlled NoV=.6에서 indirect R/B1.594→1.215, luminance는68.4% 감소한다. 흰색 밝기 복원
  성공으로 해석하지 않는다. 세 view의 albedo clamp 작동 비율0%이므로 clamp 제거 근거도 없다.

검증 자료는 out/RenderingIndirect20260923의 Floor12Exact, NativeBRDF, NativeGlobalCube,
NativeOwnerAudit, NativeImplementation에 보존한다. source-environment-owners.receipt.json은
asset/material stable ID마다 원본 cube·색·회전·placement와 충돌0을 기록한다.

### 원본 전체 화면과 남은 경계

직교 TBN의 PS 수식 비교는 원본 VS의 packed basis/비균일 스케일, 전체 direct light·SSAO·
fog·tone와 실제 camera/UI override 동등성을 증명하지 않는다. 확인한 정적 LUT 후보는
비어 있지만 전체 활성 postprocess 체인은 미확정이다. 새 색 LUT나 임의 파란 tint는 적용하지
않는다. 사용자 화면 확인 전에는 캐릭터 선택의 하얀색·하늘색 복원 완료로 판정하지 않는다.

### 사용자 중간 빌드 오류

12:23 Client.log의 C2039는 새 Client가 이전 EngineSDK의 RENDER_ENVIRONMENT_STATE/
MODEL_SURFACE_PARAMETERS를 사용해 발생했다. Engine 원본 헤더 반영은12:19, SDK 복사본은
각각10:43/09-19 상태였다. 수정 중 시작된 빌드가 서로 다른 시점의 입력을 읽은 경우다.
Engine→Shared→Server→Client 순서의 Product Build를 시작했고 Engine PASS 뒤 원본/SDK
두 헤더 SHA가 일치함을 확인했다. branch 변경이나 Clean/Rebuild는 하지 않았다.

### G13 데이터·리소스 실제 적용

12:33 이후 최신 디스크 rev79와 stable asset/material/profile ID의 변경 필드를 다시 확인한 뒤
백업·CAS·원자 교체와 공식 Map/Rendering publisher를 실행했다. Map의181행에만 sourceIndirect를
추가했고 기존 environment17행과 미배치1행, placementLighting, 나머지 material 값은 보존했다.
신규 native-only164행의 legacyEnabled=false는 이전 mode의 환경 없음 상태를 유지한다.

RenderingProfiles는 rev80이며 source-rendering.v1/warm-high-key.v1의 useSourcePBRIndirect=true와
cubeDiffuse.intensity=0만 변경했다. before-restoration/customizing-dark/LUT/tone/light/fog 등
나머지 값과 무관한 JSON 원문 형식도 보존했다. authoring/runtime 의미 일치 PASS다.
Map 공식 게시에서 줄바꿈만 달라진 무관 worldsequences는 semantic equality/CAS 확인 뒤
기존 바이트를 보존했다. 사용자 편집이나 Client Reload/종료를 자동 수행하지 않았다.

신규 Resources와 GBResources/Map 동일 상대경로의 DDS2개를 설치했고 SHA가 일치한다.
`Map/Lighting/CharacterSelect/pbr_cubemap_00.rgbm.cube.dds`는 원본 cube128²/6faces/8mips이며
SHA67620fe0c6ae9716a1eadea1cc45d77d9bbae9fd305348ec78822e32cbfa8afd다.
`Map/Lighting/CharacterSelect/character_select_native_brdf_rg16.dds`는 원본 generator와
bit-exact인 RG16 lookup이며 SHAc374749515c1a950135dda67463219531c6285697732e986b30955a518afea8b다.
원본 HDR01/HDR02는 기존 설치를 재사용하며 source payload와 실제 mip을 대조했다.

최종 patch manifest/field 원값·owner receipt는 NativeDataCandidate/patch-manifest.json,
적용·backup·실제 파일 SHA는 같은 폴더 applied.receipt.json에 보존한다. 실제 Client 화면의
색 판정은 아직 사용자 확인 전이다. Product Debug Build는 아래 최종 기록처럼 PASS다.

### G13 최종 Debug 제품 빌드

12:49:53 Product Debug Build PASS(SkipBuild=False). Engine10.888초, Shared0.272초,
Server0.417초, Client1,409.974초이며 전체1,422.735초다. 공용 맵 재질 include에 의존한
static/animated31개 셰이더와 Client C++를 정상 Build했고 새 멤버 오류는 재발하지 않았다.
기존 셰이더/인코딩 경고는 남아 있다. Clean/Rebuild나 branch 변경은 없었다.

정본 결과는 out/BuildPipeline/runs/20260923T034953862Z-debug-product.json과
NativeImplementation/product-build.log다. Client.exe는12:49:52, Engine.dll은12:26:22에
생성됐다. Engine/Client 설치 DLL 일치, 원본/SDK 새 헤더 일치, 게시 데이터/Resources·
GBResources의 최종 SHA 확인 PASS. git diff --check PASS.

진행 중 별도 Release 빌드와 사용자가12:38에 시작한 VS Debug 빌드가 같은 저장소에서
추가로 실행됐다. VS Debug는 같은 CSO를 중복 컴파일해 중단할 빌드 선택을 요청했으나
응답 전 이 작업의 Product Build가 먼저 끝났다. 다른 빌드·VS 창·Client UI는 조작하지 않았다.

추가 읽기 전용 감사에서 sourcePS cb2[3]은 FSceneView+140의 기본(0,0,0,1)이
실제 PS constant buffer2의 네 번째 float4까지 전달됨을 확인했다.
NativeOwnerAudit/albedo-view-override.receipt.json에 근거를 기록했다. 외부 runtime view
overwrite는 여전히 미검증이며 이 확인 때문에 제품 코드를 다시 수정하지 않았다.

유령 발탄은 앞선 사용자 화면 PASS를 유지한다. 이번 Character Select의 실제 흰색·하늘색
판정은 새 실행 파일의 사용자 화면 확인 전이며 완료로 기록하지 않는다. color-grading LUT
신규 복원은 없고 원본 SH·cube·BRDF 및 확인된 간접광 owner 복원이다.

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

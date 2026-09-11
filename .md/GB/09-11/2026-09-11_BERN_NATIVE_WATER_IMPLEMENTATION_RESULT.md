# 베른 원본 물 재질 연결 결과

## G00. 현재 범위

사용자가 제공한 베른 원작 이미지와 현재 화면의 차이를 원본 자료에서 추적했다. 기존 물 표현의 기본색만 바꾸지 않고 원본 MIC 17개, 고유 Base PS 6종을 기존 CModel → CMaterial → CMapAssetObject의 반투명 렌더링 경로에 연결했다. 베른 전체 복구는 진행 중이며 이 결과는 물 재질의 코드와 수치 검증 범위다.

원작 수면에도 녹색 기본색이 있다. 화면의 청색·보라색 반사는 기본색 하나로 결정되지 않으며, 원본 normal, reflection, opacity, depth bias, scene-color sampling을 함께 소비해야 한다. 현재 광장 물 asset의 원본 MIC는 `lv_ber_berncastle.mat.lv_ber_berncastle_water_01_mi`다. 기존 설치 기본 재질 `lv_atm_koilsv_water_01_mi`와 원본 배치 override를 구분했다.

## G01. 실제 소비 경로

- `SourceMapWaterMaterialParameters.h`는 원본 uniform-expression 배열과 실제 native binding의 위치를 따라 프로그램 38~43을 패킹한다. FName의 number suffix도 원본 effective parameter와 일치시킨다.
- `Shader_SourceMapWaterPrograms.hlsli`는 선택된 원본 PS 명령을 실행한다. 원본 시간식, 물결 normal, Fresnel, reflection, depth fade, opacity를 소비한다.
- `MapAssetObject::Late_Update`는 기존 `Request_SceneColorSnapshot`을 요청한다. `Renderer`가 opaque scene을 `Target_EffectSceneColor`에 복사한 뒤 반투명 물이 읽는다. 현재 출력 중인 RenderTarget을 동시에 SRV로 읽지 않는다.
- `MapAssetRenderUtils`는 실제 scene depth, scene color, 현재 scene ambient, 공유 안개 입력을 바인딩한다. 기존 mapwater tint/distortion을 중복 적용하지 않는다.
- source 좌표는 프로젝트 `(x,y,z)`에서 원본 `(x,-z,y)` 센티미터로 되돌린다. native clip 입력과 world-position 입력을 원본 VS signature에 따라 구분한다.
- beach-wave 프로그램 43은 원본 `01.scale`과 원본 vertex blue mask에 따른 수직 변위를 연결했다. 이 VS 입력의 화면 검증은 아직 하지 않았다.

## G02. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 실제 C++ parameter parser/configure | MIC 17개, texture mask·필수값 누락·추가값 거부·finite·실패 시 이전 값 보존 모두 통과 |
| 원본 DXBC와 제품 HLSL WARP 비교 | 17 MIC × 3 시간/시점 × 3 texture fixture = 153건, 156,672픽셀 |
| 수치 비교 | nonfinite 0, 상대 오차 0.001 초과 픽셀 0, 최대 상대 오차 5.89207e-6 |
| 실제 VtxMeshBinary 전체 FXC | 컴파일 성공 |
| project/filter XML | parse와 ProjectReference GUID 구조 검사 통과 |

수치 비교는 동일한 입력을 준 원본 PS와 제품 함수의 계산 검증이다. 원작 CPU의 모든 engine-owned 상수, 실제 장면의 샘플링 위치, 원본 VS 전체, 최종 시각 결과까지 입증하지 않는다. 근거는 `out/BernMaterialAudit20260911/water_gpu/`의 로그와 CSV다.

## G03. 남은 연결과 검증

`water_rows.json` 17행을 전체 Bern source-material 작업에 전달했다. 후속으로 원본 Light PS가 있는 물 9 MIC/4개 PS의 lightConstants와 texture union을 같은 forward 경로에 연결했다. 직접광의 원본 대조와 조명 adapter 경계는 [반투명·직접광 결과](2026-09-11_BERN_NATIVE_FORWARD_IMPLEMENTATION_RESULT.md)를 따른다. 물 Baked PS 네 종류는 아래 G04에 따라 연결됐다. 전체 mapmaterials/placement/RNM publisher 결과, 마지막 공통 코드 변경 이후 Product 빌드, 실제 Client 화면 비교는 전체 베른 작업에서 따로 확인해야 한다. Client와 UI는 실행하거나 캡처하지 않았다. 기존 프로젝트 tone mapping을 원작 postprocess와 동일하다고 기록하지 않는다.

## G04. 물 40~43의 Baked PS 연결

G03의 Baked 미연결 상태를 해소했다. 원본 물 9 MIC에 선택된 Baked PS 네 개를 기존 program40~43에 추가했다. 원본 ID는 `f0d20984ab98d54cbc20069e78326eef`, `38679c063b6cc841a5b31d779fff66a8`, `7ef8b0897718f64daa8ebb92a01e92ce`, `725198d14d9e5d479d4f5700efcb4881`이다. native CB0 크기16/16/23/15를 baseConstants 상단32~59에서 읽고, 실제 material parameter의 최대 패킹 위치는48이다. program43의 기존 scale63과 Base/Direct 함수 본문을 보존했다. 재질 JSON의 named input과 texture union은 바뀌지 않았다.

`EvaluateSourceMapWater`는 실제 placement의 lightmapUV와 g_HasBakedLighting을 받아 Base/Baked를 선택한다. 기존 CMaterial/MapAssetRenderUtils가 바인딩한 RNM average/directional texture와 scale을 사용한다. Baked 간접광에 별도 scene ambient를 중복해서 더하지 않는다. Catalog/publisher와 RNM 배치 연결은 map·환경 담당자가 같은 optional bakedLighting 계약으로 통합한다.

- 실제 C++ strict Configure9행: texture mask union·필수값 누락·추가값 거부·finite·실패 rollback 모두 통과했다.
- 설치 후 실제 제품 헤더로 전체 water17행을 다시 컴파일·검사해 같은 항목을 통과했다. Baked9행의 실제 패킹 binary와 네 함수 본문이 WARP 검증본과 일치함을 확인했다.
- 원본 DXBC 대 실제 패킹을 소비하는 Baked HLSL: 81 fixture,82,944픽셀,nonfinite0·상대 차이0.001 초과0,최대 상대 차이1.269e-5.
- 비균일8×8 material/RNM texture, RNM UV varying,3시간·3입력 조합: 추가81 fixture,82,944픽셀,nonfinite0·상대 차이0.001 초과0,최대 상대 차이5.460e-6.
- 합계162 fixture,165,888픽셀을 대조했다. 근거는 `out/BernWaterBaked20260911/gpu`와 `pattern`의 CSV/로그이며 source/제품 varying linkage도 검사했다. 새 함수 최소 PS 컴파일과 변경 소스 diff check는 통과했다.

이는 동일 입력의 원본 PS 계산을 복구한 결과다. 원작 CPU 전체 상수·장면 전체·최종 화면 동등성을 의미하지 않는다. 설치 후 전체 Product와 publisher의 최종 결과는 root 통합 검증으로 기록한다. Client/UI 실행·캡처·사용자 시각 판정은 수행하지 않았다.

최신 native material variant와 RNM binding을 원본 sourceId로 대조한 결과,9 MIC 중7개가 실제 RNM25배치에 연결된다. 원본 slot·배치·UV와 대응 asset은 `out/BernWaterBaked20260911/actual_rnm_join.json`에 기록했다. 이25개는 최종 로드 scope의 화면 표시 개수를 뜻하지 않으며, 최종 publisher/visibility 판정은 통합 결과를 따른다.

환경 담당자의 최종 통합 검사에서40~43은19variant material행·25source-slot에 적용됐고 대상 미소비0을 확인했다. 맵 담당자의 Catalog/publisher/CMaterial/CModel/MapAssetRenderUtils 계약은40~43만 Baked를 허용하며, 원본 Baked가 없는38/39의 negative 검사도 통과했다. 이 통합 수치는 담당자의 실행 결과로 확인한 것이며 사용자 화면 판정과 구분한다.

# 쿠크 맵 특수 표면의 원본 셰이더 연결 결과

## G00. 구현 범위

[계획 G01](2026-09-11_KOUKU_FORWARD_MATERIAL_AND_BERN_SCALE_IMPLEMENTATION_PLAN.md)에 따라 일반 배경 셰이더에 넣을 수 없는 원본 MIC 13개, 실제 material override 14행을 기존 `CModel -> CMaterial` 경로로 연결했다. 원본 spotlight 8개, 반투명 표면 4개, sky 1개다. 이 결과는 재질 계산과 입력 연결에 대한 것이며 사용자의 화면 승인은 아직 없다.

| 프로그램 | family | 선택된 원본 pixel shader ID |
|---|---|---|
| 33 | source.map.spotlight.v1 | b45f2673af7b9b48b0ef92280aa46ad5 |
| 34 | source.map.translucent-tiled.v1 | 1566c6cdf2ffc24aa1da17964a7aed32 |
| 35 | source.map.translucent-reflection.v1 | 10427e64f512ab4f86b47a72e36ef247 |
| 36 | source.map.translucent-bump.v1 | 61aadd65a765fd40a8427a76c66e8d2e |
| 37 | source.map.sky-simple.v1 | 29b7302ad93326409979e84649171179 |

## G01. 실제 소비 경로

`SourceMapForwardMaterialParameters.h`는 원본 MIC에서 선택된 uniform expression에 필요한 parameter만 읽어 기존 native material 상수 배열로 pack한다. 누락·추가·비정상 값을 거절한다. `SourceCharacterMaterialParameters.h`의 기존 진입점이 `source.map.*`를 이 함수에 위임한다. map JSON의 texture expression index와 소문자 `srgb`/`linear`는 `MapAssetCatalog`가 검사하고 실제 `CMaterial` 수명으로 보관한다.

`Shader_SourceMapForwardPrograms.hlsli`에는 원본 DXBC 명령을 옮긴 5개 계산과 현재 엔진 입력을 제공하는 adapter가 있다. 기존 `Shader_VtxMeshBinary.hlsl`의 `PS_MAIN_ALPHA` 및 `PS_MAIN_SKY`가 결과를 소비한다. spotlight는 기존 additive 배경 경로, 반투명과 sky는 해당 catalog render mode를 통해 이 pass로 들어간다. 일반 instanced 배경 shader로 보내지 않는다.

`MapAssetRenderUtils`는 실제 카메라, scene ambient, `Target_Depth`, 공통 height fog를 bind한다. 원본 spotlight의 scene depth fade는 원본 cm 단위의 clip depth와 현재 projection/depth를 연결한다. native PS가 이미 opacity를 RGB에 곱하는 additive 표면은 현재 `SrcAlpha + One` blend에서 RGB를 다시 지우지 않도록 alpha 1을 제출한다. 최종 object opacity는 기존 값으로 곱한다.

## G02. 실행한 수치 검증

- 실제 C++ parameter reader/Configure: 14행 전부 pack 성공. 원본 parameter arity·mask·누락/추가 parameter 거절과 실패 시 기존 상태 유지 검사 통과.
- 원본 5개 PS DXBC와 변환 HLSL의 D3D11 WARP 비교: 45조건, 46,080 pixel, 비정상 값 0, 상대 오차 0.001 초과 0. 같은 constant와 synthetic texture 입력을 사용했다.
- `Shader_VtxMeshBinary.hlsl` 전체 FXC `fx_5_0` 컴파일 성공. 후속 공통 안개와 최종 render mode 연결의 제품 통합 검증은 전체 복구 RESULT에 합산한다.
- 새 header/HLSLI의 Client project/filter 등록과 XML parse 확인.

실측 파일은 `out/KoukuFullRestore20260911/forward_gpu.log`, `gpu_results.csv`, `forward_programs.json`, `forward_rows.json`, `packed_manifest.csv`다. 테스트 프로그램은 임시 `out` 산출물이며 제품 실행 경로나 배포 선행조건으로 추가하지 않았다.

## G03. 원본과 구분하는 입력 및 화면 경계

수치 비교는 선택된 pixel shader 계산과 같은 입력에 대한 비교다. 원작의 전체 frame, 실제 DDS의 모든 UV·mip·sampler 동작, 투명 정렬, 원작 전역 조명 상태의 동등성을 뜻하지 않는다. material 자체의 반사 texture와 engine 소유 cubemap/SH는 별개다. 현재 forward의 no-lightmap hemisphere 입력은 기존 scene ambient를 사용하며, 원작 spatial SH를 복원했다고 주장하지 않는다.

안개는 원본 저장값과 원본 환경 volume 경계를 소비하는 공통 지수 안개 식에 연결한다. 후속 검증에서 원본 TExponentialHeightFogPixelShader DXBC와 실제 EvaluateSourceExponentialFog를 720조건/737,280픽셀로 비교해 nonfinite 0, 상대 오차 0.001 초과 0, 최대 상대 오차 4.17233e-7을 확인했다. 근거는 `out/KoukuFullRestore20260911/fog_gpu.log`와 `fog_gpu_results.csv`다. 이 검증은 동일한 입력의 PS 계산에 대한 것이며 CPU의 density/falloff 단위·directional terminator 상수 생성 전체의 원본 동일성을 뜻하지 않는다. 기존 Hable tone mapping과 원본의 후처리 설정/LUT 사이 차이도 별도다. Client/UI 실행·화면 캡처·visual PASS는 수행하지 않았다.

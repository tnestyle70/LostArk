# 쿠크 원본 후처리 연결 계획

## G00. 현재 근거와 목표

원본 `WorldInfo`는 `efpostprocess.postprocesschain.postprocess_tonemapper_epic`을 사용한다. 해당 chain의 `UberPostProcessEffect`는 `Tonemapper_Customizable`과 FXAA3이며 WorldInfo와 환경 volume의 PostProcessSettings를 소비한다. 현재 제품의 Hable curve는 원본 customizable rational/toe curve와 다르다.

설치 원본 `BAVC5ANI5QXGJ5JIX9C2J9CQSQ9VNH6.bin`의 GlobalShaderCache에서 1,785개 code record를 순차 파싱하고 각 LZ4·DXBC 길이를 검사했다. UberBlend 96개, LUTBlender 5개, exponential fog 3개와 VS 2개를 확보했다. 원본 LUT01/02는 256×16 PF_A8R8G8B8, sRGB=false, NoMipmaps다. source postprocess fields 14건과 원본 PostProcessSettings struct default를 확보했다.

## G01. 독립 shader 함수

`Shader_SourcePostProcess.hlsli`는 native tone 함수, LUT 좌표와 보간, LUTBlender의 color correction을 소유한다. 모델 재질이나 환경 volume 선택은 소유하지 않는다. 이 파일은 먼저 `out/KoukuPostprocess20260911`에서 생성·수치 검사하며 공유 Renderer·RenderingProfile 파일은 통합 담당과 연결 계약이 확정된 뒤 수정한다.

`SourceCustomizableTonemap`은 원본 pixel shader의 `A/B/split/linear-steepness` packed float4와 toe factor를 받는다. `SourceGradeLutValue`는 원본 LUTBlender의 packed color constants를 받는다. `SourceSampleLut256x16`은 native slice 좌표와 인접 blue slice 보간을 사용한다. 임의 Hable/ACES curve나 색상 LUT를 새로 만들지 않는다.

원본 shader 연산과 original authored field를 구분한다. native CPU의 scale/range → packed coefficients 변환이 확보되지 않은 값은 원본에서 읽은 수치로 표시하지 않는다. 먼저 원본 shader와 packed fixture의 동치성을 검증한 후 원본 property의 소비 경계를 결정한다.

## G02. 후속 소비자와 실패 보존

Engine Renderer는 현재 후처리 pass에서 source tone·LUT helper를 선택한다. Client RenderingProfileService는 Area·camera volume의 source profile과 Resources-relative LUT를 resolve한다. staging 중 LUT 또는 parameter validation이 실패하면 기존 profile을 유지한다. Engine의 일반적인 color transform 계약에 Client map 이름을 넣지 않는다.

새 HLSLI를 제품 shader에 포함하면 Engine/Client 원본 shader mirror와 Shader project 등록을 통합 담당이 함께 확인한다. 별도 모델 runtime, gameplay actor, Client/UI 자동 실행을 추가하지 않는다.

## G03. 검증

원본 native DXBC와 독립 HLSL helper를 WARP float RT에서 비교한다. neutral·실제 LUT01·LUT02, 다양한 HDR 값과 toe factor를 포함하고 finite·오차·검은색·white-point 조건을 확인한다. 동일 packed constants에서 native shader의 연산 동치를 증명하며 원본 CPU packing과 제품 최종 화면을 대신 증명하지 않는다. Client 화면은 사용자가 직접 확인한다.

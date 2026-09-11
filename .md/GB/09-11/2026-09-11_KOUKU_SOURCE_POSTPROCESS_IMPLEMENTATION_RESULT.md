# 쿠크 원본 후처리 조사·수치 검증 결과

## G00. 현재 완료 경계

원본 후처리 입력과 native shader를 확보하고 독립 HLSL 함수의 수치 동치성을 확인했다. **제품 후처리에는 아직 연결하지 않았다.** 원본 CPU의 TonemapperScale/Range → packed coefficient 및 LUT color constant 변환이 확정되기 전에 임의 상수를 넣지 않기로 통합 담당과 결정했다. 제품은 이 범위에서 기존 후처리를 유지한다.

독립 helper는 `out/KoukuPostprocess20260911/Shader_SourcePostProcess.hlsli`에 있다. 소비자가 없는 production HLSLI/header를 추가하지 않았으며 Renderer·RenderingProfileService·profile JSON도 이 조사에서 수정하지 않았다.

## G01. 원본 소비자와 입력

쿠크 WorldInfo는 `efpostprocess.postprocesschain.postprocess_tonemapper_epic`를 실제 참조한다. 해당 chain은 다섯 MaterialEffect 이름 슬롯, UberPostProcessEffect, AmbientOcclusionEffect 순서다. 다섯 MaterialEffect에는 serialized Material 할당이 없으므로 이름만 보고 활성 material program을 추정하지 않았다. UberPostProcessEffect에는 `Tonemapper_Customizable`, `postprocessaa_fxaa3`, `bUseWorldSettings=true`가 저장돼 있다.

원본 Engine package의 ETonemapperType enum은 Off=0, Filmic=1, Customizable=2다. PostProcessSettings struct 기본값은 Scale=1, Range=8, ToeFactor=1, LUT=null이다. 쿠크 WorldInfo의 override=true와 생략 필드를 이 struct 기본값으로 완성하면 Scale=0.85, Range=8, ToeFactor=1이다. 별도 Uber 효과의 toe=0.5를 WorldInfo toe로 잘못 대입하지 않는다.

원본 설정 14건을 기록했다. 주요 BlendVolume의 struct 완성 값은 다음과 같다. 실제 camera volume 선택과 Kismet blend 가중치는 별도 runtime 소비 경계다.

| 원본 항목 | Scale | Range | ToeFactor | LUT |
|---|---:|---:|---:|---|
| WorldInfo | 0.85 | 8 | 1 | null |
| BlendVolume_0 | 0.8 | 8 | 0.6 | LUT_02 |
| BlendVolume_1 | 1 | 8 | 0.8 | LUT_01 |
| BlendVolume_0의 Light blend data | 0.2 | 8 | 0.6 | null |
| BlendVolume_1의 Light blend data | 0.2 | 8 | 0.6 | null |

LUT01·02는 `lv_lut_midnightc.tex.lv_lut_midnightc_lut_01`·`_02`다. 둘 다 실제 256×16 PF_A8R8G8B8, sRGB=false, texturegroup_colorlookuptable, tmgs_nomipmaps다. exact headless export로 out에만 TGA를 확보했다. 16-slice 주소에서 입력 회색 8/15의 원본 byte RGB는 LUT01=(139,143,132), LUT02=(194,189,173)이다. 이는 LUT 원본 입력값의 차이이며 실제 장면의 최종 pixel 비교 결과는 아니다.

## G02. native GlobalShaderCache 확보

설치 원본 `ReleasePC/BAVC5ANI5QXGJ5JIX9C2J9CQSQ9VNH6.bin`은 2,230,721-byte BMSG cache다. 총 1,785개 code record를 순서대로 읽고 이름·record boundary·LZ4 크기·DXBC container 길이를 모두 검사했다. code section 끝은 byte 1,112,105다.

그중 UberPostProcessBlend 96개, LUTBlender PS 5개, exponential height fog PS 3개, 관련 VS 2개를 확보했다. 원본 fog 자료는 조명 담당에게 전달했으며 이 문서는 그 담당의 제품 수정·검증을 대신 기록하지 않는다.

`FUberPostProcessBlendPixelShader<X2YZW>`의 두 번째 숫자 2 분기는 customizable rational curve와 low-end gamma/toe 보간을 사용한다. 마지막에는 256×16 LUT의 인접 blue slice 두 번을 bilinear sample하고 blue weight로 보간한다. `FLUTBlenderPixelShader<1..5>`는 neutral와 source LUT의 weighted blend 후 color correction을 계산한다. 실제 실행에서 선택하는 나머지 permutation 조건은 단순 shader 이름만으로 확정하지 않았다.

원본 native 연산은 Hable과 다르다. Epic의 [UE3 Color Grading 문서](https://docs.unrealengine.com/udk/Three/ColorGrading.html)도 rational tone curve, customizable toe, 16³ LUT와 blend 후 color adjustment 순서를 설명한다. 구현 수치 근거는 설치 원본 DXBC이며 문서의 예제 상수를 현재 LostArk CPU 상수로 대체하지 않았다.

## G03. 실행한 수치 검증

`postprocess_probe.cpp`는 게임 창 없이 WARP float RT를 읽는다. 원본 `FUberPostProcessBlendPixelShader02000`과 helper, 원본 `FLUTBlenderPixelShader<2>`와 helper에 같은 packed constant를 넣었다. neutral·원본 LUT01·원본 LUT02 각각에서 HDR·toe·color adjustment 6조건을 비교했다.

**36 case, 147,456 pixel, nonfinite 0, 상대 오차 1e-4 초과 0.** 검사한 packed fixture에서 pixel shader 연산의 동치성이 확인됐다. 원본 CPU setup, LUT render target quantization, 실제 camera blend 또는 제품 최종 시각적 일치를 증명한 것은 아니다.

## G04. 남은 실제 연결

- authored Scale/Range가 native A/B/split/linear-steepness를 만드는 CPU 변환
- scene shadows/highlights/midtones/desaturation과 view gamma/overlay가 LUTBlender constants를 만드는 CPU 변환
- camera volume 선택·시간 blend·Kismet Light 가중치
- bloom/DOF/FXAA/SSAO의 원본 buffer와 실행 순서
- serialized Material이 없는 named MaterialEffect의 동적 소비자

입력과 증거는 `out/KoukuPostprocess20260911/postprocess_handoff.json`, `source_packages.json`, `class_defaults.json`, `lut_samples.json`, `native_global_programs.json`, `postprocess_gpu.csv`에 보관한다. 제품 연결은 위 CPU 경계를 확인한 뒤 기존 Renderer 후처리 경로에서 수행한다. 사용자 화면 확인은 미실행이다.

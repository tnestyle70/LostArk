# 베른 원본 렌더링 입력과 Benchmark 비교 구현 계획

## G00. 목적과 조사 근거

카메라 복원 작업에 이어 베른과 Character Select의 밝기 차이를 원본 패키지, 현재 프로필,
실제 GPU 소비자로 대조한다. 참고 이미지의 인상을 원본 수치로 취급하지 않는다.
원본 WorldInfo/환경 영역의 postprocess 값은 override와 클래스 상속까지 확인한다.
Lightmass 환경색은 bake 저작 입력이며 runtime 환경광과 같다고 단정하지 않는다.
기존 Bern RNM과 315개 광원 중 285개 SOURCE_CHARACTER 분리를 유지한다.

## G01. 실제 광원 수신기 결함

Engine/Bin/ShaderFiles/Shader_Deferred.hlsl의 SOURCE_CHARACTER 수신기에서 원본 monster
셰이더를 쓰는 구운 배경도 direct light를 다시 받는 경우를 차단한다. 기존 native map baked
binding과 픽셀별 RNM flag를 사용하며 일반 캐릭터, ALL/UNBAKED 수신기 의미를 유지한다.
RNM·발광·반사가 합쳐진 emissive 버퍼의 안개 처리는 별도 의미 확인 없이 전체 변경하지 않는다.

## G02. 원본 후처리 입력의 연결

확인된 원본 입력은 기존 RenderingProfileService → Engine renderer → deferred final 경로에
선택적으로 연결한다. 기본값은 현재 출력과 같은 항등 변환이다. 원본 연산식·LUT 활성 여부가
확정되지 않은 값을 원본과 동일한 출력으로 표시하지 않는다. 프로젝트 구현으로 연결한 값은
adapter로 구분하고 현재 Hable/노출과 원본 tone mapper 수치를 혼동하지 않는다.
지역별 override가 확인되면 기존 convex 환경 영역, source priority, blend time을 사용한다.
JSON parse/validate/serialize와 publisher가 같은 optional 계약을 소비한다.
이번 실제 연결은 bloom threshold/intensity/tint와 display-space desaturation의 네 입력이다.
원본 Highlights/Midtones·ToneScale·LUT의 CPU packing과 sun exclusion 의미는 미확정이라
수식이나 공간 마스크를 추측해 추가하지 않는다. 지역 bloom에는 profile multiplier를 한 번 적용한다.

## G03. Rendering Benchmark의 이전·복원 비교

기존 RenderingBenchmark.h/.cpp와 MainApp의 wiring을 확장한다. 기존 프로필의 명시적
qualityOverride 복사로 이전 설정을 보존하고, 확인된 source 입력을 별도 후보 profile에 둔다.
Before / Restored / Return to entry는 Activate_Profile을 사용하고 자동 Save/Publish하지 않는다.
같은 Level이며 현재 활성 profile이 도구가 적용한 마지막 profile일 때만 종료 복귀한다.
다른 scene 연출이나 Level 전환이 소유권을 가져가면 도구는 상태만 폐기한다.
기존 재질 A/B와 렌더링 비교를 구별하고 캡처 중에는 프로필 전환을 막는다.

## G04. 등록·검증·사용자 확인

새 C++ 파일은 없으며 기존 프로젝트 파일을 재배치하지 않는다. 관련 JSON/XML parse,
실제 수신기 조건·프로필 parsing·직렬화·실패 보존, source 기본값/override 수치 검증,
publisher Validate/Publish, Debug Product build와 git diff --check를 수행한다.
Client/UI를 실행하거나 캡처하지 않는다. 원본 조사, 소스 반영, 빌드, 사용자 화면 확인을
RESULT에서 구분하고 미지원 LUT/DOF/광선·환경 particle/원본 CPU packing을 구체적으로 남긴다.

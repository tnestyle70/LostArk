# 화염 재질의 mask 입력과 native 합성 교정

## G00. 목표와 현재 호출 경로

사용자 첨부에서 화염링을 이어야 할 띠가 빠지고 주변 불꽃·불티만 남는다. 이어진 요청에 따라
같은 결함을 가진 화염 재질과 공용 native adapter의 동일 계약까지 함께 조사·교정한다.
기준 HEAD는 `29ad2df5c938f269af5c2f6ad750cdb6159ee43a`, 브랜치는
`codex/sequence-capture-focus`다. 시작부터 있던 다른 기능의 미커밋 변경은 보존한다.

`effect.kouku.gate3.backstep.ring`과 `.ring.flame`의 hoop는 native2875,
회전 불꽃은 native2876이다. 기존 Authored → Playback → DocumentRenderer →
Mesh/Particle family CSO 경로를 사용한다. geometry·회전·크기·입자 발생량은 이 수정의 대상이 아니다.

## G01. 원본 masked prefix의 실제 W lane

`Shader_EffectKoukuNativeGroup2816.hlsli::ArtistNative2875`는 source 배열을 0으로 만들고
`source[0].x=1`만 공급한다. 원본 PS `cb42fe675cadf94aac44e861c822207d`의
14·19번 명령은 `CB0[0].w`를 mask에 곱한다. 20~22번 명령에서 mask−0.1을 검사하므로
현재 W=0이면 모든 픽셀이 discard된다. 재질이 소유하는 rows1~4와 engine row0을 구분한다.

`generate_artist_native_runtime_shader.py`와 해당 설치 HLSLI를 함께 교정한다.
동일 결함의 native PS/VS, masked blend, material binding의 unowned row와 실제 읽는 lane을
대조한 cohort만 row0에 기존 particle RGBA 입력을 공급한다. 원본 serialized float4 wire와
기존 창술사 masked LocalVF 교정을 함께 대조한다. W를 상수1로 만들면 원본 입자 fade가 사라지고,
XYZ를0으로 만들면 같은 row의 RGB 곱을 사용하는 재질이 검게 되므로 실제 `input.color`를 연결한다.
원본 mask 식과 임계값을 삭제하거나 모든 shader의 W를 강제로 1로 만들지 않는다.
CB row1에 뒤이어 적용되는 selectioncolor는 원본 material binding이므로 보존한다.

## G02. opacity가 포함된 RGB의 최종 합성

native2876은 RGB에 opacity를 이미 곱하고 A=0을 출력한다. distortion 동반 early return이
일반 native dispatch의 coverage 변환을 빠뜨려 제품 SrcAlpha/One에서 RGB가 다시 0이 된다.
이전2484·2811·2812 교정과 동일한 결함이다.

원본 nativeBlend와 PS 출력, 설치 dispatch를 비교해 같은 additive cohort의 SceneColor alpha를
일반 dispatch와 같이 1로 연결한다. translucent의 alpha, 원본 RGB·fade, distortion MRT는 보존한다.
기존 `install_kouku_gate1_native_shaders.py`의 sourceBlend 기반 생성 경로와 설치 결과의
일치도 검사한다. 재생성 시 같은 결함이 돌아오지 않도록 현재 생성기를 기준으로 검증한다.

## G03. 화염의 원본 world-position 입력

화염2874와 같은 PS를 사용하는3316은 원본 TEXCOORD5의 world position에 clip position을
잘못 넣으며 engine row0과 WorldToLocal3행이0으로 남는다. 기존2349의 emitter WorldToLocal
binding·source cm 좌표 변환을 정확한 PS/VS cohort에 확장한다. 화염 십자 예고3604는 원본
world position과 neutral pre-view translation XYZ=0/opacity W=1만 필요하며 별도 행렬은 없다.
이미 투영된 clip 좌표를 원본 PS에서 다시 투영하거나 모든 native carrier를 바꾸지 않는다.

생성기, 해당3개의 설치 native 함수와 기존 particle renderer의 uniform binding을 한 변경으로
연결한다. 공통 runtime·자료구조·Resources를 추가하지 않는다.

## G04. 컴파일·수치 검증과 제품 적용

기존 ProductEffectShaderWarpProbe 기반의 작은 offscreen 수치 probe를 재사용한다.
원본 DDS·native parameter·PS 입력이 같은 상태에서 전후 RGB, discard/coverage, 비유한 값을
대조한다. 실제 설치 hoop geometry를 사용한 숫자 검사와 정규화 quad 검사를 구분한다.
새 영구 하네스나 Client/UI 실행·화면 캡처는 추가하지 않는다.

변경 HLSLI의 실제 Debug FX를 격리 컴파일하고 생성기 Python syntax, 동일 cohort 재생성,
정상 translucent와 기존 수정된 additive control, `git diff --check`를 확인한다.
새 C++ 파일·프로젝트 등록·JSON 저장 계약 변경은 없다.

현재 표준 Debug Client/Server가 실행 중이다. 미저장 편집을 보존하고 사용자가 종료한 뒤
정본 Product 증분 Build로 CSO를 적용한다. 실행 중 제품 출력 교체는 하지 않는다.
최종 화염 형태·연속성·원본 일치 판정은 사용자가 수행하며 RESULT에 수치 검증과 구분한다.

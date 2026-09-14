# 화염 재질의 잘림·소멸 원인과 수정 결과

## G00. 실제 결함과 변경 범위

사용자가 첨부한 화염링 이미지에서 연결된 띠가 사라지고 주변 불꽃·불티만 남는 형태를
열람했다. 이어진 화염 전반 수정 요청에 따라 현재 설치 native material의 동일 결함을 조사했다.
소스는 [구현 계획](2026-09-14_FIRE_MATERIAL_CLIPPING_IMPLEMENTATION_PLAN.md)에 따라 교정했다.
화염이라는 이름 하나로 mask·blend·좌표·발생량을 같은 문제로 취급하지 않았다.

| 확정 결함 | 수정한 프로그램 | 실제 변경 |
|---|---:|---|
| masked LocalVF의 particle RGBA row0 누락 |13| 원본 row0에 실제 particle color/alpha 전달 |
| additive+distortion early return의 alpha 중복 적용 |130| 이미 opacity를 포함한 RGB의 출력 coverage를1로 연결 |
| 화염 world-position/WorldToLocal 입력 누락 |3| 원본 world varying, opacity 및 필요한 emitter inverse 공급 |

합146개 native 프로그램의 입력·출력 연결을 교정했다. 130개 additive는 원본 PS가
`A=0`을 출력한다는 것까지 확인했다. 이름상 화염 우선 프로그램은16개이며 화염 관련
42문서가 소비한다. 같은 합성 결함을 가진 전체130개는332문서가 소비한다.
이 숫자는 별도 이펙트 개수나 동시에 생성되는 입자 수, 화면 승인 수가 아니다.

기준 HEAD는 `29ad2df5c938f269af5c2f6ad750cdb6159ee43a`이며 기존 기능 브랜치의
미커밋 변경을 보존했다. 이번 작업은 authored Effect, Composition, Resources, 입자 개수,
사용자 위치·회전·크기·수명 값을 변경하지 않았다. 새로운 runtime·C++ 파일·프로젝트 등록은 없다.

## G01. 링 본체와 주변 불꽃은 서로 다른 경로로 사라졌다

실제 `effect.kouku.gate3.backstep.ring`과 `.ring.flame`의 hoop는 native2875,
`fx_b_me_lava_01`이다. 기존 설치 hoop822정점과 noise002/noise017/fire004 DDS를 사용한다.
원본 PS `cb42fe675cadf94aac44e861c822207d`는14·19번 명령에서 `CB0[0].w`를
mask에 곱하고20~22번 명령에서 mask−0.1을 검사한다. 기존 생성기는 row0의 X만1로
초기화했으므로 W=0, 결과−0.1이 되어 모든 픽셀이 discard됐다.

원본 material의 native vectors는 CB0 rows1~4를 소유하고 row0만 engine 소유다.
serialized shader parameter의 같은 위치에 있는16-byte row0 wire와 기존 정상 quest·
창술사 masked LocalVF를 대조했다. 이 row는 particle RGBA이다. `input.color`를 연결해
source start alpha0.5와 ColorScaleOverLife를 유지한다. W를1로 고정하는 초기 진단 후보는
최종 구현이 아니다. 같은 cohort의 다른 PS는 row0 XYZ도 RGB 곱에 쓰므로 XYZ=0도 올바르지 않다.

수정13개는 `2423,2454,2777,2783,2802,2829,2875,2889,2985,2989,2996,3108,3314`다.
고유 PS9개·정확한 VS/LocalVF·masked blend·unowned row[0]를 생성기에서 검사한다.
원본 mask 식과 threshold, row1의 selectioncolor 바인딩은 유지한다.

링의 회전 불꽃 native2876은 다른 결함이다. 원본은 opacity를 RGB에 이미 곱하고 A=0을
출력한다. distortion early return이 A=0을 그대로 SrcAlpha/One에 전달해 RGB가 다시0이 됐다.
일반 native dispatch와 동일하게 SceneColor coverage만1로 연결했다. 원본 fade는 RGB에 남는다.

## G02. 공용 합성 수정과 재생성 경계

전체321개 distortion early-return의 sourceBlend를 조사했다. 원본 additive/PS A0와
미교정 dispatch가 일치하는130개를15개 dispatch HLSLI에서 수정했다.
기존 정상 `2358/2484/2811/2812` 및187개 translucent dispatch는 그대로다.
원본 RGB 계산·distortion RT·제어 흐름에는 변경이 없다.

`install_kouku_gate1_native_shaders.py`는 이전 수정에서 이미 sourceBlend에 맞는 alpha를
생성하도록 고쳐졌지만, 전체 설치 cohort는 갱신되지 않았었다. 이번에는 그 생성 코드의 실제
case 생성 부분을 메모리에서 실행해130개 설치본문과 일치함을 확인했다. 설치기 파일은 바꾸지 않았다.
광역 원본 생성으로 다른 재질 본문을 덮지 않고 확정된 출력 대입만 변경했다.

기존2580 불뿜기의 원본 opacity 입력은 X lane이 맞다. 9/13의 SpawnPerUnit 문제는
고정 origin에서 birth0이던 발생 입력을 수정한 것으로 shader 문제의 검증 증거는 아니었다.
이번에는 그 수치·발생량·원본 atlas를 건드리지 않았다.

## G03. 화염 world-position 입력

native2874/3316의 PS `2cce8741b31d5049ab2b10e26293372d`는 TEXCOORD5의
원본 world position에 pre-view translation을 더한 뒤 CB0 rows1~3의 WorldToLocal을 적용한다.
기존 코드는 clip position과0행렬을 공급하고 최종 opacity W도0으로 남겼다.

`Effect_DocumentRenderer_Particles.cpp`의 기존2349 emitter inverse 경로를 두 프로그램에
연결했다. source cm·local/world space·유한 행렬 검사·실패 반환을 재사용한다.
native3604의 PS `05daa4c5d7a8e44bbd81c654399ddd09`는 world position과
neutral pre-view translation XYZ=0/opacity W=1만 연결한다. 다른 shader의 좌표를 바꾸지 않았다.

생성기는 정확한 PS/VS·translucent blend·CB row와 원본 VS의 world varying,
vertex material expression/sample 부재를 확인한다. 설치3함수와 원본 archive로 다시 생성한
3함수가 일치하고 deferred 항목은0이었다.

## G04. 검증 자료와 화면 경계

모든 임시 probe·compile log·구조화된 결과는 `out/FireMaterialClipping20260914/`에 있다.
이 out 자료와 CSO/OBJ/EXE는 소스 커밋 대상이 아니다.

- `prefix_cohort.json`: masked13개/PS9개, 원본 float4 wire·CB 소비·50요소33문서.
- `blend_cohort.json`: early-return321개 조사 및 원본 PS A0인 결함130개.
- `blend_patch_validation.json`: HEAD 대비130개 alpha 대입만 변경, 정상 dispatch 보존,
  실제 installer 생성 결과 일치.
- `masked_generation_validation.json`: source archive의13개 재생성 성공 및 설치본문 일치.
  이전 passValues 리터럴 초기화와 현재 동등한0/viewport 초기화 문법만 정규화했다.
- `world_prefix_validation/validation.json`: world3개 원본 재생성 및 설치함수 일치,
  변경 C++ 단일 Debug x64 TU 컴파일 성공.
- `cohort/summary.json`: additive130개×3입력=390 draw에서 수정본 RGB hash가
  수정 전 CSO의 원본 One/One 합성과 전부 일치했다. distortion RT1 변경0,
  정상 translucent8개×3입력=24 draw 변경0이다. before/candidate/OneOne 각414 draw의
  최종 비교는 오류0·NaN0이다. 120개는 비영 RGB, 나머지10개는 양쪽 모두0이므로
  이10개의 화면 표시 성공을 주장하지 않고 합성 동등성만 확인했다.
- `prefix/summary.json`: 최종75 draw 오류0·NaN0. masked13개 모두 alpha0에서0,
  alpha1에서 수정 전0→수정 후 비영 RGB를 확인했다. world3개는 수정본의 비영 출력과
  world 입력 반응을 확인했고 기존2349의 결과는 보존됐다.
- `final_ring_summary.json`: 최종 `input.color` prefix로36 draw를 검사했다. native2875의
  t=1에서 alpha0/0.1/0.5/1에 따른 비영 RGB 픽셀은0/118/2925/3964다.
  기존2877 대조 결과와2876 distortion 출력은 보존됐다.
- `numeric_final_validation.json`: 최종 수치 JSON26개 strict parse 성공.

초기 probe에서 빠졌던 normal matrix는 명시적 identity로 교정하고 해당39 draw를 세 변형에서
재실행해 최종 결과에 반영했다. Product 빌드 이후 복사돼 수정 전 증거가 아니었던 masked7개는
보존된 수정 전 shader source로3개 CSO를 다시 컴파일해21 draw를 교체했다.
native3604의 before9 draw는 여전히 수정 후 복사본이므로 수정 전후 비교에서 제외했다.
3604의 원본 입력 분석·재생성 일치와 수정본9 draw는 유효하다. 교정 전 자료는 별도 파일로
보존했으며 위 요약은 최종 유효 입력과 증거 범위를 기준으로 한다.

초기 `numeric_summary.json`의2875 W=1 후보는 원인 진단용이다. 최종 RGBA prefix의
수치 결과와 혼동하지 않는다. offscreen WARP는 Client/UI 실행이나 화면 캡처를 하지 않고
작은 render target의 숫자만 읽는다. quad fixture의 비영 픽셀 수를 실제 링 geometry,
장면 카메라, 원작 외형의 승인으로 기록하지 않는다.

사용자는 수정된 Debug Client에서 `F1 → Effect Tool V1 → All Effects → KoukuSaydon →
3관문 → 패턴 → 세이튼 → 백스탭 불뿜기`의 `화염링_생성·유지`, `화염링_동일화염포`를
재생해 링 본체와 회전 불꽃, 수명 앞뒤의 fade를 확인한다. 관련 화염포·브레스도 함께 확인한다.
사용자 화면 판정은 아직 받지 않았다.

## G05. 실제 Debug 제품 반영

초기에는 Debug Client PID45256과 Server PID63540이 실행 중이었다. 이후 두 프로세스가
종료된 것을 확인한 뒤 에이전트가 종료·UI 조작 없이 정본 Product Build를 실행했다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

전체 결과 exit0, Engine/Shared/Server/Client 단계 PASS, Client link와 shader/runtime 배포 완료다.
기록은 `out/BuildPipeline/runs/20260914T040506473Z-debug-product.json`, 전체 로그는
`out/FireMaterialClipping20260914/product_build.log`다. Client 단계177915ms,
OBJ56/CSO35/binary1 쓰기가 기록됐다. 이는 현재 작업 트리의 정상 증분 빌드 결과이며
모든 OBJ가 이번 화염 수정만으로 바뀌었다는 의미가 아니다. 기존 다른 기능 변경도 보존된 상태다.
FXC X4000, 기존 include 인코딩 C4828, 외부 DirectXTK PDB LNK4099 경고는 남았고 오류는 없다.

이번에는 Release 빌드, 광역 FullDiagnostic, Data publisher, Client 실행·화면 검증을 수행하지
않았다. Python syntax, 실제 원본 재생성, 변경 C++ 컴파일, `git diff --check`는 통과했다.
JSON/XML 저장 계약과 프로젝트 등록을 변경하지 않아 별도 publish는 필요하지 않다.

마지막 상태 확인 시13:16 KST에 시작된 Server PID42732와 Client PID32800이 실행 중이었다.
에이전트가 실행한 프로세스는 아니며, 빌드 완료 뒤 시작된 상태다. 위 F1 경로에서 사용자
화면 확인을 이어갈 수 있다.

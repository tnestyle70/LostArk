# PointLight Falloff Exponent Engine Foundation 계획

날짜: 2026-08-11

브랜치: `codex/effect-point-light-falloff-v1`

기준 commit: `d79d22715407a501157fbf0b41e7b23d54efcd76`

## 현재 실제 반영 상태와 이번 경계

현재 `LIGHT_DESC`의 PointLight 계약은 position, range, diffuse/ambient/specular까지만 소유하고,
`Shader_Deferred.hlsl`은 `saturate((range - distance) / range)` 선형 감쇠를 고정 사용한다. 따라서
Artist 31470 reconstructed runtime program의 typed `falloffexponent=2.0`을 두 번째 조명 경로 없이
기존 `CLight -> Shader_Deferred` 경로에 전달할 공용 필드가 없다.

이번 변경은 Engine 공용 `LIGHT_DESC`에 유한 양수 감쇠 지수를 추가하고, scene light와 transient
presentation light가 같은 fail-closed 값 경계를 지키도록 한다. 기본값 `1.0`은 shader에서 기존 선형
식 자체를 그대로 반환하여 현재 PointLight 화면을 보존한다. Artist/31470 식별자, emitter 수, 후보
데이터는 Engine에 넣지 않는다. Effect parser/Playback이 값 `2.0`을 설정하는 연결은 후속 R4 범위다.

## 수정 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Engine/Public/Engine_Struct.h` | `LIGHT_DESC::fFalloffExponent=1.f` 공용 typed 기본값 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Engine/Private/Light.cpp` | 최종 방어 검증과 `g_fLightFalloffExponent` 바인딩 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Engine/Private/Light_Manager.cpp` | scene batch를 swap하기 전 유한 양수 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Engine/Private/Presentation_Manager.cpp` | transient light를 push하기 전 유한 양수 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | legacy-preserving attenuation helper와 exponent 적용 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Client/Bin/ShaderFiles/Shader_Deferred.hlsl` | Engine shader와 동일한 runtime 배포본 |
| 추가 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/ProjectAudit/Test-PointLightFalloff.ps1` | 수치·구조·바인딩·mutation focused audit |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | focused audit를 공용 ProjectAudit에 등록 |
| 추가 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp` | ABI/default와 실제 CLight/scene/transient 실패 경계 실행 증명 |
| 추가 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj` | x64 Debug/Release 독립 하네스 빌드 |
| 추가 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters` | 새 C++ 파일과 직접 편입 Engine source 필터 등록 |
| 추가 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1` | configuration별 Engine runtime을 고정한 실행기 |
| 수정 | `C:/Users/user/.codex/worktrees/effect-point-light-falloff-v1/LostArk/Framework.sln` | 독립 하네스와 x64 Debug/Release 구성을 등록 |

제품 Engine에는 새 C++ 파일을 추가하지 않는다. 새 하네스 C++ 파일은 자체 `vcxproj`와 `.filters`에
등록하고, export되지 않은 실제 `Light.cpp`와 `Light_Manager.cpp`를 직접 편입한다. exported
`CPresentation_Manager`는 Engine project reference로 생성된 Engine DLL을 소비한다.

## H 계약

`LIGHT_DESC::fFalloffExponent`는 PointLight의 정규화된 선형 range weight에 적용할 양수 지수다.
생략된 기존 aggregate/default 생성자는 `1.f`를 얻는다. `1.f`는 HLSL의 legacy branch에서
`pow`를 실행하지 않고 기존 `fLinearAttenuation`으로 초기화한 결과를 그대로 반환한다. 값이 NaN, infinity, 0 또는
음수이면 scene batch는 기존 `m_SceneLights`와 순서를 유지하며 `E_INVALIDARG`를 반환하고,
transient light는 기존 frame vector를 변경하지 않고 `E_FAIL`을 반환한다.

## CPP와 shader 변경 위치

1. `tagLightDesc`의 `fRange` 바로 아래에 `float fFalloffExponent = 1.f;`를 추가한다.
2. `CLight::Create`, `CLight::Initialize`, `CLight::Render_Desc`는 PointLight exponent와 range가
   각각 유한 양수인지 allocation/state/bind보다 먼저 최종 확인한다.
3. `CLight::Render_Desc`의 `g_fLightRange` 바인딩 바로 뒤에서 `g_fLightFalloffExponent`를 바인딩한다.
4. `IsValidSceneLight`는 PointLight position/range 검사와 같은 return 식에서 exponent를 검사한다.
5. `Add_TransientLight`는 vector push 전 exponent를 검사한다.
6. shader는 `g_fLightRange` 다음 uniform으로 exponent를 선언하고, `PS_MAIN_POINT` 앞의
   `Resolve_PointLightAttenuation`이 range가 유한 양수일 때만 정규화 선형 감쇠를 만든다. 잘못된
   range는 `0.f`를 반환하고, 유효 range의 exponent가 정확히 `1.f`가 아닐 때만
   `pow(attenuation, exponent)`로 교체한다.

## focused audit 계약

`Test-PointLightFalloff.ps1`은 다음을 실행한다.

- range 내부/경계/외부 표본에서 exponent `1.0` 결과가 legacy 선형 식과 bit-equal인지 확인
- exponent `2.0`이 내부 표본의 감쇠를 줄이고 `0.5`가 늘리는지 확인
- NaN, ±infinity, 0, 음수 입력이 validator에서 거부되는지 확인
- range 0, 음수, NaN, ±infinity가 shader 방어 모델에서 정확히 0 attenuation인지 확인
- struct default, scene/transient fail-closed 검증, C++ shader bind, HLSL uniform/helper/use를 확인
- struct 기본값, C++ bind, shader uniform/pow, 각 validator를 개별 변조했을 때 audit predicate가
  모두 실패하는지 확인
- Engine shader와 Client runtime shader byte hash가 같은지 확인
- helper를 호출만 하고 결과를 버린 뒤 `fAtt=1`로 만드는 변조가 거부되는지 확인
- 독립 compiled harness에서 ABI/default, CLight final boundary, scene rollback, transient no-push를 확인

## 적용과 검증 순서

1. 공용 struct와 세 C++ 경계를 수정한다.
2. Engine/Client deferred shader를 같은 내용으로 수정한다.
3. focused audit와 ProjectAudit 등록을 추가한다.
4. `Test-PointLightFalloff.ps1`, `Test-RenderQualityWorkbench.ps1`, `git diff --check`를 실행한다.
5. build lease를 받은 뒤 Engine → UpdateLib → PointLightFalloffContractHarness →
   ClientFrontendHarness Debug/Release를 순서대로 실행한다.
6. 실행하지 않은 manual runtime QA는 RESULT에서 미검증으로 분리한다.

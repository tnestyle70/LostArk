# PointLight Falloff Exponent Engine Foundation 결과

날짜: 2026-08-11

브랜치: `codex/effect-point-light-falloff-v1`

기준 commit: `d79d22715407a501157fbf0b41e7b23d54efcd76`

상태: frozen-review corrective build PASS, review-ready, uncommitted

## 완료한 계약

- Engine 공용 `LIGHT_DESC`에 `fFalloffExponent = 1.f`를 추가했다.
- 기본값 `1.f`는 shader에서 기존 선형 감쇠를 결과값으로 먼저 보존하고, 값이 `1.f`가 아닐 때만
  `pow(linearAttenuation, exponent)`를 적용한다. 따라서 기존 PointLight는 기존 선형 식을 그대로 쓴다.
- `CLight::Initialize`, `CLight::Render_Desc`, `CLight_Manager::Replace_SceneLights`,
  `CPresentation_Manager::Add_TransientLight`가 nonfinite/0/음수 exponent를 거부한다.
- scene batch는 전체 검증 뒤에만 `m_SceneLights.swap`을 호출하므로 잘못된 행 하나가 있어도 기존
  scene light 집합을 유지한다. transient light는 검증 뒤에만 vector에 push한다.
- `CLight::Render_Desc`가 PointLight pass에서 `g_fLightFalloffExponent`를 기존 shader 경로에
  바인딩한다. 별도 renderer/light manager를 추가하지 않았다.
- Engine shader와 Client runtime shader를 동일하게 갱신했다.
- Artist/31470, emitter 수, 후보 ID를 Engine 코드에 넣지 않았다. 후속 typed Presentation consumer가
  reconstructed program의 `2.0`을 이 공용 필드에 설정할 수 있다.

## 변경 파일

- `Engine/Public/Engine_Struct.h`
- `Engine/Private/Light.cpp`
- `Engine/Private/Light_Manager.cpp`
- `Engine/Private/Presentation_Manager.cpp`
- `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`
- `Client/Bin/ShaderFiles/Shader_Deferred.hlsl`
- `Tools/ProjectAudit/Test-PointLightFalloff.ps1`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp`
- `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj`
- `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters`
- `Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1`
- `Framework.sln`
- `.md/GB/08-11/2026-08-11_POINT_LIGHT_FALLOFF_EXPONENT_FOUNDATION_PLAN.md`
- `.md/GB/08-11/2026-08-11_POINT_LIGHT_FALLOFF_EXPONENT_FOUNDATION_RESULT.md`

제품 Engine 새 C++ 파일은 없다. frozen review corrective에서 독립 compiled contract harness와
그 전용 vcxproj/filters/runner, `Framework.sln` 등록을 추가했다.

## Frozen review corrective 진행 상태

초기 build 뒤 독립 reviewer가 최종 range 경계, shader invalid-range 방어, helper 결과 소비,
compiled rollback 증거, ProjectAudit detail을 freeze 전 필수로 판정했다. corrective 구현과
Debug/Release build/run을 모두 완료했다.

- `CLight::Create`, `Initialize`, `Render_Desc`: POINT range와 exponent가 finite/>0이 아니면
  allocation/state/bind 이전 `nullptr` 또는 `E_INVALIDARG`
- Engine/Client deferred shader: invalid/nonpositive range는 attenuation 0, valid exponent 1은 기존
  `saturate((range-distance)/range)` 직접식 유지
- focused audit: legacy 6 bit-equal, exponent 0.5/1/2 변화, invalid exponent 5 reject,
  invalid range 5개가 0, source/harness mutation 13개 reject
- helper call 결과를 버리고 `fAtt=1`로 덮는 semantic mutation reject
- `LIGHT_DESC` ABI `sizeof=92`, offsets `range=36/exponent=40/diffuse=44` compile assertion과
  default `1.f` runtime bit assertion 추가
- compiled harness가 실제 `CLight.cpp`, `Light_Manager.cpp`, exported `Presentation_Manager`를 통해
  final boundary, scene invalid replacement old-state 보존, transient invalid no-push를 검증하도록 구성
- focused audit는 한 줄 stdout evidence를 내고 ProjectAudit는 빈 detail을 PASS로 기록하지 않음

현재 corrective focused audit 결과:

```text
PointLight falloff exponent audit PASS; legacyDefaultSamples=6; exponents=0.5/1.0/2.0;
invalidExponents=5; invalidRanges=5; mutations=13; engineClientShaderEqual=true
```

XML parse, `dotnet sln Framework.sln list`, `git diff --check`도 PASS다.

corrective build lease에서 `/m:1` 직렬로 실행한 결과:

| 단계 | 결과 |
|---|---|
| Engine x64 Debug | exit 0, compile/link 성공 |
| Engine x64 Release | exit 0, compile/link 성공 |
| `UpdateLib.bat Debug` | exit 0 |
| `UpdateLib.bat Release` | exit 0 |
| PointLightFalloffContractHarness x64 Debug build | exit 0 |
| PointLightFalloffContractHarness Debug run | exit 0, ABI/default/final-boundary/scene-rollback/transient-no-push PASS |
| PointLightFalloffContractHarness x64 Release build | exit 0 |
| PointLightFalloffContractHarness Release run | exit 0, ABI/default/final-boundary/scene-rollback/transient-no-push PASS |

FXC는 Engine Debug/Release 양쪽에서 `isfinite` range guard를 포함한 deferred shader를 compile했다.
PointLight 변경으로 생긴 shader warning은 0개이며 기존 Effects deprecated와 Debug의
`Resolve_FinalFXAA` X4000만 남았다. harness project reference의 기존 post-step은 `pwsh.exe not found`
문구를 출력했지만 두 configuration 모두 compile/link exit 0이고 exe 직접 실행도 exit 0이었다.

corrective 뒤 전체 ProjectAudit는 108 checks 중 새
`rendering.point-light-falloff-contract`를 PASS로 기록했다. JSON detail은 길이 166의 다음
실제 evidence 문자열이다.

```text
PointLight falloff exponent audit PASS; legacyDefaultSamples=6; exponents=0.5/1.0/2.0;
invalidExponents=5; invalidRanges=5; mutations=13; engineClientShaderEqual=true
```

전체 ProjectAudit exit은 기존 baseline check 15개 때문에 1이다. 실패 이름 집합은 아래 기존
ProjectAudit 절에 기록된 15개와 동일하다.

아래 초기 build/ProjectAudit 증거는 첫 구현의 과거 실행 사실로 보존한다. frozen 판정은 위
corrective rebuild와 compiled harness 결과를 정본으로 사용한다.

## focused 검증

`Test-PointLightFalloff.ps1` PASS:

```text
legacy-default samples: 6 bit-equal
exponent variation: 0.5 / 1.0 / 2.0
invalid numeric samples: NaN, +Inf, -Inf, 0, -1 전부 reject
source mutations: 6/6 reject
Engine/Client deferred shader: equal
```

mutation 범위는 struct 기본값, C++ shader variable bind, scene finite validation, transient finite
validation, HLSL uniform, HLSL exponent 적용이다. 초기 HLSL early-return 형태에서 FXC가 새 X4000
warning을 냈기 때문에 결과를 legacy linear 값으로 먼저 초기화하고 `exponent != 1`일 때만 교체하도록
바꿨다. 재빌드에서 새 warning은 0개였고 기존 `Resolve_FinalFXAA` warning만 남았다.

`Test-RenderQualityWorkbench.ps1` PASS:

```text
legacy-compatible defaults
deferred/shadow/SSAO/bloom/FXAA contracts
Engine/Client Shader_Deferred hash equality
```

## 빌드와 실행 검증

build lease를 받은 뒤 모두 `/m:1` 직렬로 실행했다.

| 단계 | 결과 |
|---|---|
| Engine x64 Debug | exit 0, compile/link 성공 |
| Engine x64 Release | exit 0, compile/link 성공 |
| `UpdateLib.bat Debug` | exit 0, public header/lib/shader 배포 성공 |
| `UpdateLib.bat Release` | exit 0, public header/lib/shader 배포 성공 |
| ClientFrontendHarness x64 Debug build | exit 0, compile/link 성공 |
| ClientFrontendHarness x64 Release build | exit 0, compile/link 성공 |

두 FrontendHarness build 끝에는 사용자 전역 post-step의 `pwsh.exe not found` 문구가 출력됐지만
프로젝트 compile/link와 MSBuild exit는 0이었다. exe를 별도로 실행해 실제 test 결과를 확인했다.

Debug/Release `ClientFrontendHarness.exe`는 모두 exit 1, `failures: 19`였고 실패 이름과 개수가
두 구성에서 동일했다. PointLight 또는 이번 변경과 연결된 실패는 0개였다. 기존 exact-base Effect/
DimensionMaster 데이터 baseline 19개는 다음과 같다.

1. Effect Source Mesh Size Converts UE Units And Axis Basis Once
2. Current DimensionMaster Authored Documents Preserve Exact Semantic Module Counts
3. Current DimensionMaster Vector Field Occurrences Resolve Exact Source Volumes
4. All Effects Stages Every Current DimensionMaster Authored Document
5. DimensionMaster A Hit Mesh 14 And 20 Preserve Implicit Alpha Identity
6. DimensionMaster D 2050240 V12 Source Material Profiles Round Trip Losslessly
7. DimensionMaster D 2050240 Cooked Distribution Payloads And Sprite Sizes Are Exact
8. DimensionMaster D 2050240 Source Color And SubUV Baselines Execute Once
9. Every Current Imported Baseline Round Trips And Preserves Pending Materialization State
10. Canonical DimensionMaster T Round Trips Summon Model Cue
11. All Effects Joins Every PlayerSkills EffectId To Valid Authored Document
12. Manual Mesh Create Contract Round Trips One Percent Scale Carrier Layer
13. Effect Local Crack Admits Legacy Carrier But Rejects Partial Named Contract
14. Effect Fail-Closed Mesh Without Base Stages For Hidden Rendering
15. DimensionMaster Q Stable Emitter Material Tuning Saves And Reloads Losslessly
16. Four-Class Effect Runtime Matches Generated Assembly Component Emitter Contract
17. Effect Runtime Stages Every Four-Class Product Document
18. Data Files Stages Every Current WFX Component
19. Effect Runtime Invalid Catalog Preserves Committed Assembly State

Client full build는 integration owner가 후속 통합 SHA에서 실행하기로 한 경계라 이 slice에서는
ClientFrontendHarness까지 수행했다.

## 전체 ProjectAudit

`Invoke-ProjectAudit.ps1`은 exit 1, 기존 baseline check 15개로 종료했다. 새
`rendering.point-light-falloff-contract`는 개별 PASS였고 console에도 focused audit PASS가 출력됐다.
기존 실패 check는 다음과 같다.

```text
maps.extracted-area-runtime-roots
maps.character-select-area-contract
projects.data-source-visibility
effect.g09-authoring-world-runtime-boundary
effect.g09-cross-document-contract
effect.artist-31470-reconstructed-source-capability
effect.artist-31470-material-texture-runtime-binding
effect.artist-31470-exact-dds-runtime-deployment
effect.artist-31470-wmodel-geometry-contract
effect.artist-31470-reconstructed-runtime-program
effect.wfx-component-assembly
effect.representative-authored-readiness
effect.four-class-authored-clip-product-exact101
actors.catalog-assets
actors.dimensionmaster-runtime-animation
```

전체 감사 뒤 tracked 변경은 이 slice 파일에만 남았고 publisher/audit 부수 변경은 없었다.

## 최종 정리

- `git diff --check`: PASS
- Engine/Client deferred shader text equality: PASS
- MSBuild/cl/link/ClientFrontendHarness/Client/Server 잔류 process: 0
- corrective build lease: release 완료, MSBuild/cl/link/fxc/PointLight harness/Client/Server process 0
- commit/push: 수행하지 않음
- manual runtime eye validation: 수행하지 않음
- Artist 31470 runtime program/Playback가 exponent `2.0`을 실제 제출하는 연결: 후속 R4

이 결과는 공용 PointLight 기반만 완료한 것이다. Artist F runtime/Product admission은 변경하지 않았고,
실제 F 재생 판정은 typed parser/Playback/Presentation 연결과 manual eye validation 뒤에만 가능하다.

# Artist 31470 F PointLight Typed Consumer 구현 계획

날짜: 2026-08-11

브랜치: `codex/artist-f-pointlight-consumer-v1`

기준 commit: `8352a3718bcf0a3f9386e913a8a90807e70f8bd0`

기준 tree: `77c75c6915d482e61b89c273f41f36d7407b4188`

## 목표와 종료 증거

현재 `CEffectPlayback`은 authored Light의 `fFalloffExponent`를
`EFFECT_EVALUATED_LIGHT`까지 전달한다. 그러나
`CEffectObject::Submit_Presentation`은 `LIGHT_DESC`를 만들 때 이 필드만 복사하지 않아,
명시적인 `2.0f`도 Engine의 `LIGHT_DESC` 기본값 `1.0f`로 바뀐다.

이번 슬라이스는 기존 `EFFECT_EVALUATED_LIGHT -> CEffectObject ->
CPresentation_Manager -> CLight -> Shader_Deferred` 경로에 정확한 typed 변환을 연결한다.
별도 Light manager나 renderer를 만들지 않는다. Artist reconstructed Catalog의
runtime execution, submit, render와 Product admission은 계속 false다.

종료 증거는 다음과 같다.

```text
evaluated exponent 2.0f -> LIGHT_DESC 2.0f bit-exact
LIGHT_DESC 2.0f -> Presentation transient vector 2.0f bit-exact
같은 descriptor -> 실제 CLight::Render_Desc와 deferred shader bind 성공
legacy explicit/default exponent 1.0f bit-exact 유지
0, 음수, NaN, infinity exponent와 잘못된 range/finite 입력 fail-closed
변환 실패 시 caller의 기존 LIGHT_DESC 미변경
reconstructed runtimeExecutionAdmission=false
reconstructed productAdmission=false
PlayerSkills/animevents 변경 0
```

독립 frozen 재감사의 교정 gate는 다음을 추가한다.

```text
Artist exact tuple:
  pre-evaluated world position = (1.25, -2.5, 3.75, 1.0)
  source radius 200 * UE scale 0.01 = range 2.0
  intensity 10.0, falloff exponent 2.0
  normalized white RGBA = (1, 1, 1, 0)
  ambient RGBA = (0, 0, 0, 0), specular RGBA = (0, 0, 0, 0)
immediate LIGHT_DESC와 transient storage에서 위 전체 tuple bit-exact
Point render 제출에서 caller shadow=true여도 g_iApplyDirectionalShadow=0
enabled authored Light exponent는 finite positive, missing legacy default는 explicit 1.0
Artist 31930의 generator-owned enabled 두 행은 1.0으로 재생성
```

이 consumer fixture는 CPU executor가 이미 평가한 packet부터 시작한다. 따라서
`EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION`의 emitter transform + particle location 계산과
원본 `castCompositeShadow=false`, `affectCompositeShadowDirection=false` 해석은 CPU executor의
선행 계약이다. 이 슬라이스는 그 두 source 입력을 직접 재구성했다고 주장하지 않는다.

## 현재 실제 반영 상태와 경계

- `Engine::LIGHT_DESC::fFalloffExponent` 기본값은 `1.f`이며 ABI offset은 `40`이다.
- `CLight::Create/Initialize/Render_Desc`, `CLight_Manager`,
  `CPresentation_Manager::Add_TransientLight`는 유한 양수 exponent와 Point range를
  fail-closed로 검증한다.
- `CLight::Render_Desc`는 `LightDesc.fFalloffExponent`를
  `g_fLightFalloffExponent`에 직접 바인딩한다.
- `CEffectPlayback`은 `Element.Detail.Light.fFalloffExponent`를
  `EFFECT_EVALUATED_LIGHT::fFalloffExponent`에 저장한다.
- `CEffectObject::Submit_Presentation`의 기존 `LIGHT_DESC` 조립만 exponent를 생략한다.
- reconstructed preparation의 `Admit_Execution`, `Admit_Submit`, `Admit_Render`는 false라서
  이 슬라이스가 Artist 프로그램 전체 실행을 열지 않는다.

## 수정 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Public/Effect_LightPresentation.h` | evaluated Light를 Engine descriptor로 바꾸는 단일 public pure seam 선언 |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Private/Effect_LightPresentation.cpp` | finite/range/exponent 검증, stage 후 일괄 commit, 정확한 필드 매핑 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Private/Effect_Object.cpp` | Submit에서 수동 조립 대신 동일 pure seam 호출 후 기존 Presentation Manager 제출 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Default/Client.vcxproj` | 새 H/CPP 등록 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Default/Client.vcxproj.filters` | 새 H/CPP를 기존 Effect 필터에 등록 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp` | 실제 pure seam, Presentation Manager, CLight, WARP shader 경로를 한 실행에서 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj` | Client pure seam 직접 편입, Client include, D3D11 link 등록 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters` | Client seam source 필터 등록 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1` | 실제 Engine deferred shader 절대 경로를 harness에 전달 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/ProjectAudit/Test-PointLightFalloff.ps1` | Object delegation, pure seam, project/harness, Product-false mutation gate 확장 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Public/Effect_AuthoringDocument.h` | enabled Light의 inherited/default falloff exponent를 legacy `1.f`로 명시 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Client/Private/Effect_DocumentCodec.cpp` | enabled Light exponent를 finite positive로 fail-closed 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | default/round-trip bit-exact, explicit zero reject, failed parse output 보존 회귀 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/LevelPlacementExtractor/build_imported_effect_documents.py` | semantic base-key 집합이 실제로 없을 때만 inherited legacy `1.0`; scalar indexed key, alias 중복, strict-invalid 값은 생성 거부 |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py` | missing/default, 세 casing, scalar `[0]/[1]`, canonical+indexed, invalid value, case-alias duplicate와 escaped JSON key 회귀 |
| 재생성 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/Data/Effects/Imported/Artist/CurrentCombat/Converted/effect.artist.skill.31930.imported.effect.json` | enabled PointLight 두 행의 unresolved class default를 explicit `1.0`으로 materialize |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-pointlight-consumer/LostArk/.md/GB/08-11/2026-08-11_ARTIST_31470_F_POINT_LIGHT_TYPED_CONSUMER_RESULT.md` | 실제 구현·자동 검증·미검증 경계 기록 |

## H 계약

`Try_BuildEffectPointLightDesc`는 상태를 소유하지 않는다. 호출자는
`EFFECT_EVALUATED_LIGHT`와 기존 `LIGHT_DESC`를 전달한다. 함수는 position, range,
intensity, color, ambient, falloff exponent와 곱셈 결과가 유한한지, range와 exponent가
양수인지, intensity가 음수가 아닌지 모두 확인한다.

검증을 통과한 local `LIGHT_DESC`만 마지막에 `OutLight`에 대입한다. 실패하면
`OutLight`를 한 줄도 바꾸지 않는다. 성공 descriptor는 Point type, world position w=1,
입력 range/exponent, intensity를 곱한 diffuse/ambient, zero specular를 가진다.

이 함수는 Presentation Manager나 shader를 호출하지 않는다. `CEffectObject`가 변환 성공 뒤
기존 `CPresentation_Manager::Add_TransientLight`를 호출하고, Engine이 frame render에서 같은
descriptor를 `CLight::Render_Desc`로 소비한다.

## CPP 호출 흐름

```text
CEffectPlayback evaluated frame
-> CEffectObject::Submit_Presentation
-> Try_BuildEffectPointLightDesc
-> local LIGHT_DESC 검증과 commit
-> CPresentation_Manager::Add_TransientLight
-> CLight_Manager::Render_Lights
-> CLight::Render_Desc
-> g_fLightFalloffExponent
-> Resolve_PointLightAttenuation
```

변환 또는 Engine 제출이 실패하면 `Submit_Presentation`은 `E_FAIL`을 반환한다.
`CPresentation_Manager::Submit_FrameProviders`는 기존 계약대로 frame 전체를 clear하므로
부분 Light frame을 유지하지 않는다.

## 하네스와 focused audit

기존 compiled `PointLightFalloffContractHarness`를 확장한다. harness는 production의 새 CPP를
직접 편입하므로 별도의 test-only 변환을 만들지 않는다.

1. evaluated `2.0f`를 변환해 `LIGHT_DESC`와 transient vector의 exponent가 bit-exact `2.0f`인지 확인한다.
2. WARP D3D11 device, 실제 Engine `Shader_Deferred.hlsl`, `CShader`, `CVIBuffer_Rect`를 만들고
   같은 descriptor로 `CLight::Render_Desc`를 실행한다.
3. evaluated `1.0f`가 변환, Presentation, CLight 경로에서 bit-exact인지 확인한다.
4. exponent `0/-1/NaN/+Inf`, range `0/-1/NaN/+Inf`, nonfinite position/intensity/color와
   overflow 결과를 거부하고 output descriptor 및 transient vector가 그대로인지 확인한다.
5. 기존 `LIGHT_DESC` final boundary, scene batch rollback, transient no-push 회귀를 유지한다.

focused PowerShell audit는 새 header/CPP/Object/project/harness source를 UTF-8 strict로 읽고,
production Object가 helper 뒤 기존 Manager만 호출하는지, helper가 exponent를 실제 대입하는지,
Client와 harness project가 같은 source를 compile하는지 확인한다. exponent copy, Object delegation,
finite 검증, project source 등록을 각각 변조하면 audit가 실패해야 한다.

## 적용과 검증 순서

1. pure seam H/CPP와 Client project/filter 등록을 추가한다.
2. `CEffectObject::Submit_Presentation`의 Light 조립을 pure seam 호출로 교체한다.
3. compiled harness와 runner를 확장한다.
4. focused audit와 mutation matrix를 확장한다.
5. XML parse, `git diff --check`, Product/admission/static binding 검사를 실행한다.
6. serial build lease에서 Engine -> UpdateLib -> PointLight harness -> Client를 Debug/Release로 빌드한다.
7. Debug/Release harness와 focused audit를 실행한다.
8. 전체 ProjectAudit을 실행하고 이 lane과 무관한 baseline 실패를 RESULT에 분리한다.
9. 변경을 unstaged/uncommitted 상태로 동결해 independent review를 요청한다.

수동 인게임 F 재생과 육안 판정은 이 슬라이스 범위가 아니다. typed reconstructed executor가
실제로 `EFFECT_EVALUATED_LIGHT`를 생성하고 Product admission이 열린 뒤에만 수행한다.

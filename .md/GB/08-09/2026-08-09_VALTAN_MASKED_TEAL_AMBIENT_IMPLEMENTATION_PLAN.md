# Valtan 몸·도끼 masked teal ambient 구현 계획서

## 목표

Valtan 몸과 도끼의 원본 E(emissive) texture가 지정한 부위에만 R=0, G/B 가중치의 청록색 ambient/glow를 적용한다. scene light의 diffuse/specular/ambient를 전역 변경하지 않고 Valtan material profile 안에서 닫는다.

## 현재 실측

- 몸 모델 `Character/Valtan/MN_RPBF_01.wmodel`은 다음 3개 material을 가진다.
  - `mn_rpbf_01_mi`
  - `mn_rpbf_01_1_mi`
  - `mn_rpbf_01_2_mi`
- 도끼 모델 `Character/Valtan/ValtanWeapon.wmodel`은 다음 2개 material을 가진다.
  - `wp_mn_rpbf_01_mi`
  - `wp_mn_rpbf_01_1_mi`
- 다섯 material은 diffuse/normal/specular/emissive texture를 모두 가진다.
- `CBody_Valtan::Render`와 `CValtan::Ready_PartObjects`의 weapon descriptor는 이미 `material.valtan.monster-base.v1`을 소비한다.
- deferred G-buffer에는 별도 per-material ambient RGB target이 없다. 전역 light ambient를 바꾸면 Valtan 외의 벽과 맵까지 바뀐다. 원본 E texture를 specific-part mask로 사용해 additive material channel에 색을 주는 것이 현재 렌더 계약에서 가장 좁은 구현이다.
- 다른 세션은 같은 utility의 full-surface emissive override와 shadow render를 수정 중이다. 이번 변경은 profile resolve 상단만 수정하고 그 변경을 보존한다.

## 변경 파일

- `Client/Private/DeferredMaterialRenderUtils.cpp`
- `Tools/ProjectAudit/Test-ValtanMaskedTealAmbient.ps1`
- `Tools/ProjectAudit/Test-RenderQualityWorkbench.ps1`의 기존 Valtan profile assertion 1개

H, 새 C++ 파일, project/filter 등록은 없다.

## 데이터와 호출 흐름

```text
몸/도끼 mesh material name
  -> Resolve_DeferredMaterialProfile("material.valtan.monster-base.v1", name)
  -> exact 5-slot intensity + RGB(0, 1.35, 1.55)
  -> Bind_DeferredMaterialInputs
  -> E texture sample * teal weight * slot intensity
  -> Target_Emissive 합성
```

E texture가 없는 mesh는 기존 `g_HasEmissiveTexture` 경계에서 적용되지 않는다. 알 수 없는 material name은 intensity 0으로 반환해 identity처럼 청록색을 확산하지 않는다.

## G00. exact material profile

- 기존 color `(0.15, 1.5, 0.9)`를 `(0, 1.35, 1.55)`로 교체한다.
- 몸/도끼 5개 material의 기존 강도 5/10/15를 보존한다.
- unknown material fallback을 1에서 0으로 바꾼다.
- 다른 세션의 `DEFERRED_EMISSIVE_OVERRIDE` binding은 수정하지 않는다.

## G01. 회귀 하네스

- exact profile ID와 다섯 material 이름, 강도, R=0/G/B 가중치를 검사한다.
- unknown fallback 0을 검사한다.
- 몸과 도끼 call site가 같은 profile을 소비하는지 검사한다.
- 다섯 E texture의 실제 존재와 두 binary shader의 E-mask 곱셈 계약을 검사한다.
- 전체 Rendering Workbench audit의 이전 color assertion을 새 exact R0 teal 계약으로 교체한다. 같은 파일에 진행 중인 SSAO/shadow/Effect 검사는 건드리지 않는다.

## 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-ValtanMaskedTealAmbient.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git diff --check
```

사진이나 스크린샷은 검증에 사용하지 않는다.

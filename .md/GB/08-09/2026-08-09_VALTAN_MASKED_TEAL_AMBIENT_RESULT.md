# Valtan 몸·도끼 masked teal ambient 수정 결과

## 완료 상태

Valtan 몸과 도끼의 저작된 E(emissive) texture가 지정한 부분에만 R=0, G/B 가중치의 청록색 unlit energy가 적용되도록 material profile을 교정했다. scene 전체 ambient나 Bern/Character Select map material은 변경하지 않았다.

이미지·스크린샷을 이용한 색 판정은 수행하지 않았다. WModel material/texture slot, call site, shader binding, 전용 audit와 빌드만 사용했다.

## 확인한 렌더링 계약

몸 `MN_RPBF_01.wmodel`의 3개 material과 도끼 `ValtanWeapon.wmodel`의 2개 material은 모두 diffuse, normal, specular, emissive texture를 가진다. 몸과 도끼 call site는 공통 stable profile ID `material.valtan.monster-base.v1`을 사용한다.

현재 deferred G-buffer에는 per-material ambient RGB target이 없다. 전역 light ambient를 바꾸면 Valtan의 특정 부분뿐 아니라 맵과 다른 물체까지 바뀐다. 따라서 저작된 E texture를 특정 부분 mask로 유지하고, additive emissive target을 해당 material의 ambient/glow 채널로 사용하는 것이 현재 파이프라인에서 가장 좁고 정확한 경계다.

## 실제 변경

- `Client/Private/DeferredMaterialRenderUtils.cpp`
  - exact color를 `float4_t(0.f, 1.35f, 1.55f, 1.f)`로 고정했다.
  - 몸/도끼 5개 exact material의 기존 강도 `5 / 10 / 15`를 유지했다.
  - 알 수 없는 material name은 강도 `0`으로 fail-closed 처리한다.
  - E texture가 없는 mesh에는 기존 `g_HasEmissiveTexture` 경계로 적용되지 않는다.
- `Tools/ProjectAudit/Test-ValtanMaskedTealAmbient.ps1`
  - profile ID, exact material names, R0 teal 값, 강도, unknown fallback, 몸/도끼 소비자, E-mask shader 경계를 검사한다.
- `Tools/ProjectAudit/Test-RenderQualityWorkbench.ps1`
  - 기존 Valtan color assertion 한 줄만 새 R0 teal 계약으로 갱신했다.

같은 utility에 다른 세션이 추가한 full-surface emissive override와 shadow 변경은 보존했으며 수정하지 않았다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `Test-ValtanMaskedTealAmbient.ps1` | PASS |
| `Test-RenderQualityWorkbench.ps1` | PASS |
| 몸/도끼 WModel 정보 검사 | PASS, 3 + 2 materials 및 E slot 확인 |
| Release Client shader/C++ build | PASS |
| Release Client 10초 startup probe | PASS |
| ProjectAudit `rendering.quality-workbench-contract` | PASS |

전체 ProjectAudit의 rendering 실패는 다른 세션 authored profile의 `globalQuality.fxaaEdgeThreshold` 범위 오류다. Valtan masked teal 계약과 Workbench 코드 감사는 통과했다.

## 수동 검증과 남은 경계

- 이미지 확인 금지 규칙에 따라 몸/도끼의 육안 색 비교는 미실행이다.
- 이 변경은 exact E-mask와 수치 계약을 고정한다. 이후 색 강도를 조절할 때는 R을 0으로 유지하고 G/B와 material별 intensity만 조절해야 한다.
- 전역 ambient, diffuse/specular 값, Effect Tool 문서, HDR/SSAO/shadow 설정은 이번 변경 범위가 아니다.

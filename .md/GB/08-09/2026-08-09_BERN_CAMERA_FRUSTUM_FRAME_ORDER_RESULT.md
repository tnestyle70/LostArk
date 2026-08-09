# Bern 이동 중 카메라·프러스텀 프레임 순서 수정 결과

## 완료 상태

구현과 전용 자동 검증을 완료했다. Bern에서 Server snapshot으로 갱신되는 플레이어 Transform과 follow camera binding이 map visible-list 작성보다 한 프레임 늦던 순서를 교정했다.

이미지·스크린샷을 이용한 판정은 수행하지 않았다. 이 문서의 결과는 코드 호출 순서, 전용 audit, 빌드, 프로세스 기동 결과만 기록한다.

## 확인한 원인

기존 `CGameInstance::Update_Engine`은 `CObject_Manager::Late_Update` 뒤에 `CLevel_Manager::Update`를 호출했다. 그 결과 Bern의 한 프레임은 다음 순서로 진행됐다.

```text
이전 camera state refresh
-> follow camera Late_Update
-> map frustum culling / render submission
-> Bern replication snapshot 적용
-> local character Transform 및 camera target binding
```

이동 입력, 최초 follow bind, Server snapshot 경계에서는 map culling이 이전 카메라 상태를 소비하고 gameplay Transform은 이후에 commit됐다. 정지 프레임과 이동 프레임의 visible-list 기준이 달라지는 것이 이동 중 대규모 clipping의 직접 원인이었다.

## 실제 변경

- `Engine/Private/GameInstance.cpp`
  - `Post_Physics_Update` 다음에 `CLevel_Manager::Update`를 실행한다.
  - 그 다음 `CObject_Manager::Late_Update`에서 follow camera pipeline/frustum 갱신과 map culling/render submission을 수행한다.
  - public header와 새 C++ 파일은 추가하지 않았다.
- `Tools/ProjectAudit/Test-CameraFrustumFrameOrder.ps1`
  - `Post_Physics_Update -> Level update -> Late_Update` 순서를 검사한다.
  - Bern replication과 camera binding, follow camera pipeline refresh, map culling 소비 경계를 함께 검사한다.

다른 세션의 HDR, SSAO, shadow, Effect 변경은 되돌리거나 재작성하지 않았다. `GameInstance.cpp`의 해당 프레임 순서 블록만 이 수정 범위다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `Test-CameraFrustumFrameOrder.ps1` | PASS |
| Release Engine/Shared/Server/Client build | PASS |
| Release Server `--contract-test` | PASS, `failures : 0` |
| Release Client, `Client/Default`, 10초 startup probe | PASS, probe PID만 종료 |
| Debug Server/Client 산출물 갱신 | PASS, 2026-08-09 05:07 |
| ProjectAudit `camera.follow-same-frame-transform` | PASS |

Release `Invoke-BuildAndRegression.ps1`은 모든 빌드와 관련 harness를 통과한 뒤 최종 ProjectAudit의 비관련 2개 항목 때문에 종료 코드 1을 반환했다. 실패 항목은 `maps.product-editor-visual-scope`와 다른 세션의 `rendering.profile-parser-contract` FXAA 범위 오류다. 이번 frame-order 계약 실패는 없다.

Debug 전체 자동화는 Server/Client 링크까지 완료한 뒤 후속 Effect source intake가 5분 호출 제한을 넘겼다. intake 프로세스는 CPU가 계속 증가하며 정상 실행 중이어서 강제 종료하지 않았다. 이 프로세스가 만드는 Effect 산출물은 다른 세션 범위이므로 이번 결과에서 평가·stage·revert하지 않는다. 별도로 시작한 Debug `ClientFrontendHarness`도 60초 제한을 넘겨, 해당 호출에서 남은 exact harness PID만 종료했다. 따라서 Debug build는 PASS지만 Debug 전체 regression은 완료로 기록하지 않는다.

## 수동 검증과 남은 경계

- 이미지 확인 금지 규칙에 따라 Bern 이동 장면의 화면 판정은 미실행이다.
- 자동 검증은 동일 프레임의 Transform, camera pipeline, frustum, map submission 순서를 고정한다.
- 실제 Server+Client Bern 연속 이동의 최종 체감 확인은 사용자가 실행 화면에서 확인할 수 있으나, 이 문서에서는 PASS로 기록하지 않는다.

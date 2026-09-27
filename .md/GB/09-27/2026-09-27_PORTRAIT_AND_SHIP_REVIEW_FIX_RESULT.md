# PR #465 초상과 #467 승선 거절 보완 결과

## 초상

한 프레임의 두 번째 이후 초상이 앞 초상의 작은 viewport와 depth 없는 출력 상태를
상속하던 문제를 수정했다. 요청마다 원래 출력/depth를 복구하고 전체 SceneHDR 크기의
viewport로 geometry·light·combine을 실행한다. 종료 시 모든 기존 viewport와 출력,
카메라, SSAO 상태를 복원한다.

캐릭터의 source translucent hair·eyelash와 opaque-forward mesh를 field와 같은
NONLIGHT/BLEND 재질 pass로 combine 다음에 그린다. `Request_Portrait`는 optional
forward callback을 추가했고 기존 호출은 기본값으로 유지한다. avatar override는
각 geometry/forward 호출에 동일하게 적용하고 바로 복원한다.

G-buffer depth marker만으로 alpha를 판정하지 않는다. 초상 combine이 불투명 coverage를
1로 쓰고, forward pass가 SceneHDR alpha에 alpha-over를 누적한다. resolve는 이 coverage를
출력하며 투명 픽셀의 RGB를 tone mapping 전에 straight alpha로 바꿔 UI의 두 번 감쇠를
막는다. field RGB material·light pass와 tone/exposure/gamma/LUT/FXAA 설정은 재사용한다.
기존 초상 정책인 SSAO·bloom·screen presentation post 제외는 유지한다. 따라서 전체 field
후처리와 완전히 동일하다고 판정하지 않는다. rendering option 정본은 변경하지 않았다.

## 선박

항구 밖에서 선박 변경이 거절되기 전에 `End_VehicleSkill`이 기존 mount action/flight를
종료하던 순서를 수정했다. `Begin_ShipVoyage`는 해상 목적지 검증을 마친 성공 경로에서만
기존 action을 종료한다. 비행에서 승선할 때에는 검증된 착지 위치를 복귀 부두로 보존한다.
거절 시 active vehicle뿐 아니라 mount skill·clock·cooldown도 유지하는 회귀 조건을 추가했다.

## 실행한 검증과 남은 확인

- `Shader_Deferred.hlsl` FXC `fx_5_0 /O1` 컴파일 성공. Engine/Client mirror byte 일치.
- 컴파일된 실제 PORTRAIT_RESOLVE pass를 D3D11 WARP로 실행했다. G-buffer depth를
  바인딩하지 않은 fixture에서 alpha 0/1/0.25/0.625를 보존했고, 같은 radiance의 불투명/
  투명 입력 RGB가 tone mapping 이후 동일했다. 이 검증은 shader resolve만의 수치 검증이다.
- `git diff --check` 통과. 기존 C++ 인코딩과 CRLF를 보존했다.
- WARP probe·CSO·실행 파일은 Git 제외 `out/PrPortraitIntegrationFix20260927`에 있다.
- C++ Product build와 Server vehicle regression 실행 결과는 상위 통합 검증에서 기록한다.
  본 작업에서 Client/UI/Server를 실행하지 않았고, 실제 여러 초상과 머리/속눈썹 최종 화면은
  사용자의 확인이 남아 있다.
- 이 보완은 authoring/published data를 추가 교체하지 않았다.

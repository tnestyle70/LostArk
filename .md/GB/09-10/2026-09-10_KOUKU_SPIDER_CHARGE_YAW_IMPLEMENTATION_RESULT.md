# 쿠크 거미 돌진 몸 방향 보정 결과

## 구현

`chargeYawOffsetDegrees`를 기존 ENTER_AREA charge의 optional 저작 값으로 연결했다. Composition parse/validate/serialize와 Workbench의 Charge facing offset, projector, Gameplay publisher, Server catalog와 실제 charge motion이 같은 값을 소비한다. 기본 0, finite -360..360 범위이며 nonzero yaw는 양수 charge distance가 있어야 한다. 기존 PATTERNLOGICCHARGE 5열은 yaw 0, 새 6열은 명시 yaw로 읽는다.

Server는 기존 `atan2(dx,dz)` body yaw에 offset을 더한다. 이동 방향과 7m EndPosition, target 한 번 고정, Navigation/Collision, 접촉 검사 순서는 보존했다. 다른 native body 축이나 GameRoom은 수정하지 않았다. 소스 통합 payload는 `out/KoukuBallMaterial20260910/spider-charge-yaw.patch.json`이고 P15에서 사용하는 `kakulsaydon.g1.logic.31`에 +90만 추가한다. 사용자가 저장 중인 Composition JSON은 직접 교체하지 않았다.

## 검증

- Client CompositionDocument/ActionWorkbench 및 Server GameplayCatalog/KoukuSaydonBrain/LogicRuntime/ServerGameplayContractTests 개별 컴파일 성공.
- 실제 Composition C++의 parse → serialize → reparse에서 logic31 yaw +90 보존과 직렬화 동등성 통과. 입력은 out probe이며 사용자 정본 저장은 하지 않았다.
- 기존 projector charge test에서 +90의 window 투영, 361도/NaN/거리0+yaw 거부, 기존 charge 시간·절대 이동·retarget 충돌 검사를 통과했다.
- 실제 publisher의 logic-window 분기를 out 입력으로 실행해 거미 3개 charge sidecar의 거리7/yaw90, yaw 필드 없는 기존 입력의 0 기본값, 비정상 각도·거리 없는 yaw 거부를 확인했다.
- Python compile, PowerShell AST parse, 담당 파일 diff check 통과.
- root가 최종 Server 소스를 out에 전체 컴파일·링크한 뒤 `--kouku-object-overlap-contract-test`와 `--kouku-bundle-contract-test`를 실행하여 exit0/failures0을 확인했다. charge contract는 +90 body yaw, target이 움직여도 기존 방향 유지, 7m 끝점과 BODY-follow FEAR 접촉을 함께 확인한다. 근거는 `out/KoukuLivePublishFix20260910/build-with-charge.log`, `charge-contract.log`, `contract-with-charge.log`다.

## 남은 통합

사용자 마지막 Save/종료 뒤 root가 최신 정본에 공 materialProfile과 이 yaw payload를 합치고 publish 및 Product EXE를 교체한다. 현재 실행 중인 Client/Server는 교체하지 않았다. 사용자 화면에서 몸이 돌진 방향을 향하는지 확인하는 단계는 남았으며 Client/UI 조작·캡처·visual PASS는 하지 않았다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.

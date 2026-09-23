# Valtan Release 패턴 제어 결과

## 완료한 변경

Client의 Level_ValtanArena replicated boss 준비·spawn/despawn 응답·arena preset 상태와 BossTool의 Complete Play admission을 공통화했다. PatternAuditionService/PatternFlowService 자체는 이미 공통이므로 재구현하지 않았다. root MainApp이 같은 서비스를 양 구성에서 update한다.

PlayerController의 고정 arena 위치 제출과 correlated 결과 drain, disconnect 초기화를 공통화했다. Debug 자유 picking은 유지한다. Kouku gate가 사용하는 Return to Start와 NONE/MARIO/MAZE mode도 같은 Server 검증 경로를 사용한다.

Server는 stable-ID Valtan play/restart/next/flow 요청의 단순 Release 거절을 제거하고 기존 session·boss placement·revision·sequence·owner 검증을 유지했다. pending/active/completed 상태, fixed tick, owner 이탈, 마지막 player 퇴장, generation pin, terminal lifecycle flush를 함께 공통화했다. 선언만 열고 실제 소비자를 막아두는 중간 상태를 남기지 않았다. arena preset의 기존 preflight/transaction도 같은 경로를 사용한다.

Valtan spawn은 기존 disabled boss.valtan.center / BOSS_VALTAN / ENCOUNTER_VALTAN allowlist이며, despawn은 Valtan 소유 트리만 제거한다. Kouku spawn/despawn도 기존 arena boss allowlist/raid-running 거절을 유지한다. Release teleport는 Valtan/Kouku에서만 가능하며 기존 navigation, 높이, collision, alive/capture 상태와 자기 session player 검증을 거친다.

Release에서 Bern 자유 teleport, Character Select Valtan audition, 일반 wave 재소환, self-kill, generic WORLD 재생, local ActionWorkbench/map/camera preview는 확대하지 않았다. retired health/timeline opcode는 기존 REJECTED_PATTERN_UNAVAILABLE를 유지한다.

## 검증

- 변경 C++ git diff --check: PASS.
- 변경 header/cpp 전처리 조건 균형 검사: PASS.
- root의 MainApp/Level_KakulSaydonArena Release 단일 TU: PASS 통지 수신.
- `--valtan-pattern-control-contract-test`를 기존 테스트 TU에 추가했다. 실제 published room/boss로 session/revision 거절, Complete Play ACTIVE→COMPLETED_HOLD, flow stop→STOPPED_HOLD와 pinned revision, retired opcode 거절, Release Character Select 경계를 검증한다.
- 기존 teleport 검사에서 Valtan/Kouku admission·높이·충돌·중복·capture와 Kouku start cleanup을 양 구성 공통으로 검사한다. Bern Release 거절만 유지한다.
- 이전 Run_ValtanAudition의 Release 전체 거절 분기는 기존 Debug의 상세 검증을 공통 소비하도록 갱신했다.
- Debug/Release Product: PASS (`out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`, `20260923T224850148Z-release-product.json`).
- `--valtan-pattern-control-contract-test`: Debug 8 PASS / failures 0, Release 9 PASS / failures 0, 양 구성 exit 0 (`out/ReleaseRaidTools20260924/debug-valtan-pattern-control.log`, `release-valtan-pattern-control.log`). actual Complete Play/flow lifecycle와 Release 경계 모두 통과했다.
- 최종 teleport: Debug 8109 PASS / failures 0 (`debug-debug-teleport-verified.log`), Release 7576 PASS / failures 0 (`release-debug-teleport.log`), 양 구성 exit 0. arena 이동·응답·상태·중복 명령과 once-only trigger 검사가 모두 통과했다. 별도 담당이 fixture를 실제 overlap 경로로 옮기고 Return to Start가 선택한 trigger-once 정책을 보존하도록 보정한 최종 결과다.

## 남은 확인

Client/UI를 실행하지 않았다. 사용자 F1 Load Pattern/Complete Play/flow 및 arena 이동 화면 확인은 별도다. C++ 신규 파일 없이 기존 TU와 CLI만 확장하여 프로젝트 등록 변경은 없다.

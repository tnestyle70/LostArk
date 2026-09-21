# 쿠크 전체 재생 종료 후 개별 재생 준비 복구

## G00. 실측과 목표

전체 raid의 `Stop_KoukuRaid`는 보스를 despawn하지만 Client의 `m_iActiveDebugGate`는
마지막 관문 presentation을 유지한다. BossTool은 그 index만 보고 같은 관문 activation을
생략하므로, 다음 개별 Complete Play가 이미 제거된 NPC를 기다린다. 최근 진단의 STOP
request 3에는 `Complete Play stopped by its owner` 응답이 도착했다. 개별 audition의 IDLE은
전체 raid와 다른 owner의 정상 상태이며, 이를 임의 Reset하는 방식으로 고치지 않는다.

## G01. 서버 승인과 관문 화면의 분리

`Level_KakulSaydonArena.h/.cpp`에 Debug gate 요청 당시의 world generation과 raid epoch,
최종 spawn/teleport 승인이 완료된 scope를 각각 보관한다. 기존 typed command와 승인을
유지하며, 현재 gate index와 승인 scope가 모두 일치할 때만 기존 배치를 재사용한다.
새 raid epoch나 world가 오면 이전 승인은 재사용할 수 없다. NPC 구성만 늦은 경우는
재소환하지 않고 기존 승인을 기다린다. 승인 전에 scope가 바뀌면 이전 요청을 성공으로 commit하지 않는다.

`KoukuSaydonBossTool.cpp`는 이 승인 조회로 activation 필요 여부와 준비 중 scope를 확인한다.
완료/정지 방송을 반복 처리하며 신규 audition을 취소하거나 오래된 terminal로 새 승인을 지우지 않는다.

## G02. 게시 프로세스 관찰

`MainApp.cpp`에서 두 Workbench의 `Tick_Background`와 게시 결과 회수는 active raid guard
앞에서 수행한다. 전체 재생, 닫힌 F1, 다른 workspace 선택과 관계없이 종료된 publisher handle을
회수하되 살아 있는 publisher, draft, revision 검사는 유지한다.

## G03. 검증과 남은 경계

실제 변경 함수로 구성한 격리 회귀에서 종료 뒤 같은 관문 재승인, 늦은 NPC 구성, 새 epoch,
world 변경, 실패한 승인, 게시 중/완료 관찰을 확인하고 수정 전 실패를 대조한다.
기존 MainApp 요청/응답 회귀로 timeout 뒤 late admission 및 queued STOP 계약을 재확인한다.
변경한 실제 TU는 scratch 출력 경로로 컴파일한다. 기존 미커밋 코드와 데이터는 보존하고
제품 링크, Client/UI 실행, source/data Publish는 수행하지 않는다. 새 C++ 제품 파일이 없어
vcxproj/filter 등록은 필요 없다. 자동 수치 검증과 사용자의 실제 화면 확인은 RESULT에서 구분한다.

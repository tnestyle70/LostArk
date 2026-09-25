# Kill Boss 뒤 관문 Sequence 연결 결과

## G01. 원인과 반영

Kill Boss는 정상 boss 사망 처리를 사용한다. 그러나 Complete Play의 raid owner가
없는 단독 보스/패턴 실행에서는 관문 동의 후 `Advance_Gate`의 legacy spawn/teleport
분기로 들어갔다. 이 분기는 Bingo만 준비·intro에 연결하고 1~3관문은 직접 이동시켰다.
첨부 Source In 준비 실패와는 별도의 경로 누락이다.

`GameRoom_GateProgress.cpp`와 `GameRoom_KoukuRaidFlow.cpp`에서 단독 실행의
ADVANCE/RESTART도 `Begin_KoukuRaidPreparation -> 전원 READY ->
Begin_KoukuRaidCinematic`을 사용하도록 연결했다. 기존 Bingo 전용 flag를
`bGateVoteEntry`로 일반화하고 요청 관문, 제안자와 전원 동의를 확인한다.

준비 동안 이전 player/boss/패턴 상태를 유지한다. 전원 READY가 완료된 fixed tick에서
이전 audition, Bingo, 카드 미로 상태를 정리하고 Mario cursor를 1로 초기화한다.
플레이어의 부활·게이지·쿨타임 리셋 후 intro의 authored arrival이 이동을 담당한다.
일반 Complete Play의 준비 거절 조건과 기존 pinned raid 전이는 유지한다.

G3 입구 deck의 ENTER_GATE3는 intro 뒤 전투 공간으로 이동하는 별도 동의이므로
기존 participant/destination 검증 경로를 유지한다. G3 처치 후에는 기존 자동
false-clear 5초 -> Encore Sequence -> Bingo 흐름을 사용한다.

## G02. 검증과 남은 경계

기존 `ServerGameplayContractTests_KoukuRaid.cpp`에 2/3/4인 × 5전이 × 성공/FAILED의
30개 fixture를 추가했다. G1/G2 ADVANCE와 G1/G2/G3 RESTART에 대해 부분 동의·
부분 READY·FAILED의 world 보존, 모든 READY 뒤 CINEMATIC 진입과 이동 지연,
G3 재시작의 Mario cursor reset을 검사한다.

- Debug Server ClCompile 성공. 로그: `out/KoukuMadnessRaid20260924-server-debug-compile.log`.
- 독립 검토에서 누락됐던 이전 audition 정리와 Mario cursor reset을 찾아 반영했다.
- 최종 v37 기준 Debug/Release Server·Client ClCompile 모두 exit0.
  로그: `out/KoukuMadnessRaid20260924-{server,client}-{debug,release}-v37-compile.log`.
- 최종 Product 링크와 `--kouku-raid-contract-test`는 실행 중 Client 2개와 Server 1개의
  사용자 저장·종료 확인을 기다린다. 기존 EXE를 새 계약의 실행 증거로 사용하지 않았다.
- 테스트 소스 추가와 실행 PASS는 구분한다. 실제 Client 연출 화면은 사용자 확인 대상이다.

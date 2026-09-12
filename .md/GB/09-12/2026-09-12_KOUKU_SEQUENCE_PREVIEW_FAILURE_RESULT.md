# 쿠크 입장 시퀀스 실패 처리 결과

## G00. 구현

배우가 있는 입장 Pattern은 Bundle preview adapter를 사용한다. WORLD Play 또는
Seek가 실패하면 `Fail_Preview`가 기존 Stop 경로로 actor/WORLD/effect/session을 정리하고,
실패 Pattern ID와 원인을 one-shot event로 보존한다. clock과 성공 completion은 함께
해제된다. `Begin_BundlePreview` 최초 sample 실패도 성공 admission으로 반환하지 않는다.

MainApp은 UI transport를 적용한 다음 실패 event를 completion보다 먼저 소비한다.
Complete run이면 기존 `CancelKoukuGateCompletePlay(error)`에 연결하고, 일반 preview면
기존 Stop에 연결한다. 단일 Pattern WORLD 실패도 같은 소비자를 사용한다. 실패 후
일반 animation 상태가 오류를 덮어쓰거나 완료 Gate를 전달하지 못하도록 분기한다.
입장 admission 실패도 즉시 Complete cancel로 전달한다. Root의 Prepare/Finish/Update/
Cancel 함수 본문, Level gate 상태는 G00의 변경에서 수정하지 않았다. G03에서
Update의 Flow 제출 호출만 고정 revision 인수를 전달하도록 변경했다.

## G01. 검증

- 현재 `KoukuSaydonPresentationPlayer.cpp`와 `MainApp.cpp` Debug 개별 컴파일 성공.
  기존 헤더 인코딩 경고만 있고 compiler error는 없다.
- 실제 Fail_Preview/Consume_FailedPreview/Consume_CompletedPreview/Stop_Preview 네 함수
  본문을 기존 out CPU 검사 방식으로 실행했다. GPU·actor 정리는 카운터 stand-in으로
  한정했다. 실패 시 성공 completion 없음, one-shot ID/원인 보존, cleanup 호출, 명시 Stop,
  정상 완료, 다음 run의 오래된 실패 폐기 모두 통과했다.
- actual MainApp 소비 순서, 두 WORLD 실패 지점의 즉시 return, 최초 sample admission
  거부를 source 검사했다. `git diff --check` 통과.

증거는 `out/DimensionMasterALTV20260912/preview_failure_compile.log`,
`preview_terminal_run.log`, `preview_terminal_probe.cpp`다. 제품 링크/Client 실행/UI 조작/
화면 검증은 하지 않았다. 실제 WORLD/GPU 실패 주입을 실행했다고 기록하지 않는다.

## G02. Root에 전달한 별도 결함

1. Gate reply 대기 중 Stop/timeout이 Level gate pending과 PlayerController pending
   teleport ID를 남겨 입력/재시도 및 늦은 reply commit에 영향을 주는 경계.
2. Validate_PatternFlow 뒤 Play_PatternFlow가 다시 Reload해 실제 제출 revision을
   비교 이후에 바꿀 수 있는 경계.

첫 번째 pending retire 수정은 Root가 소유하며 이 문서의 검증으로 대신하지 않는다.
두 번째 revision 경계는 아래 G03에서 구현·검증했다.


## G03. Flow 제출 직전 revision 고정

`CKoukuSaydonBossTool::Play_PatternFlow`의 선택 인수 `expectedSourceRevision`은 기본값
0으로 기존 F1 호출을 유지한다. `Prepare_PatternFlow`가 실제 저장 Flow와 publish inventory를
재로드한 직후, 0이 아닌 입장 revision과 현재 `m_iSourceRevision`이 다르면 오류를 보존하고
Server Flow service 호출 전에 거부한다. MainApp Complete Play는 입장 시 보존한
`m_iKoukuCompletePlayFlowRevision`을 이 인수로 전달한 뒤 세션 상태를 해제한다.

- 현재 `KoukuSaydonBossTool.cpp`와 `MainApp.cpp` Debug 개별 컴파일 성공. 기존 헤더의
  인코딩 경고만 있으며 compiler error는 없다.
- 실제 `Play_PatternFlow` 함수 본문을 out CPU 검사에 사용했다. 파일 재로드와 Server 전송은
  경계 stand-in으로 두고, 재로드 7→8 시 pinned 7 거부·전송 0회·오류 보존, 동일 revision 7의
  PATTERN/BUNDLE 전송, 기존 2인수 F1 호출의 최신 revision 9 전송, Prepare 실패의 원인 보존과
  전송 차단을 확인했다. 파일 변경 경쟁을 제품 Client에서 실행했다고 기록하지 않는다.
- MainApp이 고정 revision을 초기화 전에 전달하고 일반 F1은 기존 2인수 호출을 유지하는지,
  실제 함수의 Reload → revision guard → Server submit 순서를 source 검사했다.
  관련 파일 `git diff --check` 통과.

증거: `out/DimensionMasterALTV20260912/shared_stop_source_compile.log`(MainApp/BossTool 재컴파일), `flow_revision_run.log`,
`flow_revision_source_check.log`, `flow_revision_probe.cpp`. 제품 링크/배포/Client 실행은 하지 않았다.


## G04. 공유 shell Stop의 Complete 취소 연결

MainApp의 공유 Animation Preview transport는 활성 benchmark Complete run의 STOP을
`CancelKoukuGateCompletePlay`로 보낸다. 연출이 끝나 owner가 NONE이 된 Server spawn/player
move 대기 중에도 이 Stop만 허용한다. 일반 preview Stop은 기존 경로를 유지하고 다른
workspace나 owner 없는 Pause/Resume에는 권한을 넓히지 않는다.

- 현재 MainApp/BossTool Debug 개별 컴파일 성공: `shared_stop_source_compile.log`.
- 실제 공유 Stop 분기를 out CPU 검사로 실행했다. 입장 중 취소, preview owner 해제 후
  Server 대기 취소, 일반 Stop 유지, 타 workspace Stop와 owner 없는 Pause 격리 통과.
  `shared_stop_run.log`, `shared_stop_source_check.log`에 기록했다. 실제 cancel 소비자는
  Workbench Complete/preview/Level gate/입력/HUD/암전/follow 복귀에 연결됨을 source로 확인했다.
- 기존 F1 `Get_DebugGates` 함수 전체는 HEAD와 일치한다. G1/G3 player position은 모두
  (-2.45, 1.32, 945.17)이며 서로 다른 기존 stable boss placement를 사용한다.
- 정상 완료에서는 같은 MainApp Update 안에서 preview의 SceneProfile 복귀와 fade hold가
  처리되어 그 사이 Render가 없다. Server spawn/move 승인 뒤 follow pose를 복구할 때 fade
  alpha는 1을 유지하고 다음 Level Update부터 기존 0.4초 밝아짐을 사용한다. HUD는 pending 동안
  숨기며 해제 뒤 Server snapshot을 다시 받은 다음 UI update에서 나타난다. 화면 fidelity와
  실제 GPU 전환은 사용자 확인 전이다.

검증은 out/source 범위이며 Client/UI 실행과 캡처는 하지 않았다.

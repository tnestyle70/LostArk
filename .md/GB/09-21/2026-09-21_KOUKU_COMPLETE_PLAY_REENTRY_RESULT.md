# 쿠크 전체 재생 종료 후 개별 재생 준비 복구 결과

## G00. 원인과 완료 범위

소스 수정과 격리 검증을 완료했다. 이후 부모 작업에서 revision 2024 Pattern publish와 정본 Debug
제품 빌드·링크·배치도 완료했다. 결과는 `out/BuildPipeline/runs/20260920T224103825Z-debug-product.json`의 PASS다.
Client 실행·UI 조작은 하지 않았다. 사용자는 새 Client에서 `Complete Play - Sequences + Pattern Flow`를 정지/완료하고
같은 관문의 개별 Complete Play를 다시 눌러 최종 화면 동작을 확인해야 한다.

Server `Stop_KoukuRaid`는 보스를 despawn한다. Client의 `m_iActiveDebugGate`는 마지막 관문
화면을 유지하지만, BossTool이 이를 배치 승인으로 재사용하면서 typed gate activation을
건너뛰었다. 이후 사라진 NPC를 기다려 준비가 진행되지 않았다. 진단
`client-session-59848.jsonl`의 126~128행에는 STOP request 3, 정지 응답, ABORTED epoch 1이
기록되어 있어 단순 응답 누락과 구분된다. 개별 audition의 IDLE 문구는 다른 owner의 상태다.

## G01. 구현

- `Level_KakulSaydonArena.h/.cpp`는 요청 당시 world generation·raid epoch와 최종 승인 scope를
  따로 보관한다. 모든 spawn/teleport 응답이 성공하고 scope가 그대로일 때만 승인을 확정한다.
  gate index·현재 world·raid epoch가 모두 같은 승인만 `Is_DebugGateApprovedForServerPlay`로 재사용한다.
- `KoukuSaydonBossTool.cpp`는 위 승인으로 activation 필요 여부와 준비 중 유효성을 검사한다.
  전체 재생 뒤 같은 관문도 새 typed 승인을 받는다. 이미 승인한 NPC의 느린 생성은 재소환 없이 기다린다.
- `MainApp.cpp`는 두 Workbench의 publisher polling과 결과 회수를 active raid guard 앞으로 옮겼다.
  전체 재생 중이고 F1/workspace가 닫혀 있어도 완료된 process handle을 회수한다. 실행 중인
  publisher와 draft/revision 제한, 로컬 preview guard는 유지했다.

오래된 terminal 이벤트로 개별 audition을 Reset하거나 새 승인을 지우는 경로는 추가하지 않았다.
기존 receipt·timeout·queued STOP·PATTERN_COMPLETED 후속 패턴 계약은 변경하지 않았다.
MainApp의 기존 hammer/encore 변경과 Level의 무적 표시 등 다른 미커밋 변경을 보존했다.
기존 UTF-8 BOM 여부와 CRLF도 유지했다. 새 제품 C++ 파일이나 project/filter 등록은 없다.

## G02. 검증 증거

| 검증 | 결과 |
|---|---|
| 실제 BossTool 준비/Update, Level 승인 commit/조회, MainApp route, Workbench Poll 함수 회귀 | 수정 후 36/36 PASS; 수정 전 snapshot 15개 실패 |
| 기존 실제 MainApp + ClientReplication mailbox 회귀 재실행 | Debug 41/41, Release 27/27 PASS |
| MainApp.cpp, Level_KakulSaydonArena.cpp, KoukuSaydonBossTool.cpp 실제 Debug TU | scratch `/c` 3개 PASS, exit 0 |
| 변경 파일 `git diff --check` | PASS |
| 별도 agent의 4개 변경 파일 독립 코드 검토 | PASS, 수정 필요 finding 없음 |

새 회귀는 `out/KoukuCompletePlayReentry20260921/run-probe.ps1`로 실행했다.
`generate_probe.py`는 실제 변경 함수/branch를 추출하고 Engine/NPC/resource/socket 협력자만
결정적 stub으로 연결한다. 수정 전후 소스와 추출본 hash는 `probe-manifest.json`, 결과는
`baseline/contracts.log`, `candidate/contracts.log`에 있다. 새 소스의 별도 재구현을 검증한 것이 아니다.

검증 입력은 terminal 뒤 같은 관문, 최종 despawn 전 남아 있는 옛 NPC, 승인 뒤 늦은 NPC 생성,
중복 terminal과 새 승인 보존, 새 raid epoch, world 변경, 늦은 이전 승인, 승인 실패·취소·timeout,
준비 중 새 raid 및 resource 대기, 재생 중 publisher 실행/완료·닫힌 workspace·단일 결과 회수다.
기존 mailbox 검증은 timeout 뒤 late admission, queued STOP, stale/다른 owner 응답을 계속 통과했다.

컴파일 명령·로그·OBJ는 같은 out 폴더의 `client.rsp`, `client.cmd`, `client.log`와 3개 OBJ다. 마지막 no-gate sentinel 검사 추가 뒤
Level TU만 `level-final.rsp`로 재컴파일했고 `level-final.log`에 성공을 기록했다.
기존 헤더의 문자 집합 경고는 남아 있다. 격리 컴파일 자체는 제품 EXE/DLL을 교체하지 않았고
후속 정본 Product 빌드에서 새 Client.exe와 DLL 배치를 완료했다. 자동 검증은
사용자의 화면·실제 리소스 재생 판정을 대신하지 않는다.

# 쿠크 준비 현황과 Workbench 초기 숨김 결과

## G00. 실제 4인 대기 원인

사용자가 네 Client 모두 재생됐다고 확인했다. 기존 실행의 Server 로그
`Server/Bin/Debug/Diagnostics/server-room-perf-33896.log`에서도 같은 runEpoch1의
ReadyMask15/Participants4와 RaidPhase2(CINEMATIC)를 확인했다. 로컬 준비 완료는
Player1 05:25:07.640, Player4 05:29:33.061, Player2 05:29:36.905,
Player3 05:29:50.392(KST) 순이다. 최초 준비자와 마지막 준비자의 차이는282.752초다.
Debug 기능 차단이나 전원 준비 후 교착이 아니라 다른 참가자의 준비를 기다린 경우다.
실측 요약은 `out/KoukuRaidDiagnostics20260926/observed-four-player-readiness.json`이다.

세션 시작 LAN 스크립트가 점검한192.168.0.22와 실행 Client 로그의 실제 연결 주소
192.168.200.113은 달랐다. 전자의 not-listening을 이번 대기의 원인으로 판단하지 않는다.

## G01. 실제 소스 변경

- `MainApp.h/.cpp`: 전체 Raid phase와 개별 Pattern 상태를 분리했다. 고정 START roster와
  Server readyMask로 x/N 진행률·참가자별 READY 수신 상태를 표시한다. 최대4명과 실제
  참여 인원을 구분한다. 이 Client의 PlayerId/process ID·실제 endpoint·마지막 수신 나이도 표시한다.
- run epoch별 로컬 준비 단계·전체 오류·READY/FAILED 제출 결과를 보존한다. 준비 완료
  통지 성공과 Server 확인을 구분하고, Server 확인 이후에는 낡은 통지 대기 문구를 숨긴다.
  송신 결과는 기존 세션 진단에도 기록한다. 실패 wire reason의 길이 제한은 유지한다.
- Server ABORTED 사유와 재시작 필요를 표시한다. 원격 PC의 세부 준비 단계는 기존
  packet에 없으므로 추정하지 않고 그 PC의 F1 패널에서 확인하도록 안내한다.
- `EnsureDebugTool(..., bShowWindow)`의 false 경로는 embedded tuner의 객체 준비만
  수행한다. Bingo Size·Whirlwind tuning·background Publish가 Workbench를 자동으로
  열거나 focus/preview owner를 바꾸지 않는다. 사용자의 명시적 Open은 기존 동작을 유지한다.
- Handbook의 실제 파티 초대 범위를 Bern으로 교정했다. Debug/Release 차단은 없으며
  Valtan의 새 초대 우클릭은 비활성, Kouku의 초대 UI는 미연결이다.

Server gameplay, Shared wire, 저작 데이터와 렌더링 설정은 변경하지 않았다. 새 C++ 파일과
프로젝트 등록 변경도 없다. 기존 미커밋 변경은 보존했다.

## G02. 자동 검증

MSVC14.44.35207 x64의 현재 프로젝트 compile 명령을 기반으로 MainApp translation unit을
Debug와 Release 각각 컴파일했다. 제품 OBJ/PDB 대신 out의 독립 OBJ를 사용했고 공유 PCH
쓰기 충돌을 피하기 위해 PCH 없이 같은 강제 include를 사용했다. 두 구성 모두 exit0이다.
소스와 헤더의 전후 SHA가 일치하며 기존 UTF-8(BOM 없음)·CRLF를 유지했다.

- `out/KoukuRaidDiagnostics20260926/compile-Debug.json`:11.52초, exit0.
- `out/KoukuRaidDiagnostics20260926/compile-Release.json`:9.29초, exit0.
- 각 compile 로그에 C4819 코드페이지 경고가 있으며 컴파일 오류는 없다.
- 이번 변경 파일의 `git diff --check` 성공. 저작 JSON/XML 변경은 없다.
- 독립 WIP 검토에서 확인한 Server READY 이후 낡은 대기 문구를 수정한 뒤 두 구성을
  다시 검증했다. 최종 소스 SHA는48a99432a160d131a03097b133421742592a98082709492ebc5ef0f02731576e다.

## G03. 실행 파일 반영과 사용자 화면 확인

이번 작업은 실행 중인 Client10608/14044/39576/43532와 Server를 종료·조작하지 않았다.
현재 EXE의 최종 링크·교체는 하지 않았고 새 진단 UI의 실제 화면은 아직 확인하지 않았다.
위 네 Client의 정상 시퀀스 로그는 기존 실행 증거이며 새 UI의 화면 검증을 대신하지 않는다.

사용자가 실행을 마친 뒤 정상 Debug Product Build와 Client 재실행이 필요하다. 새 실행에서
F1을 처음 열 때 Action Workbench가 숨김이고, 명시적 Open으로 열리는지 확인한다. Complete
Play 패널에서 준비 x/N·각 Player의 READY·로컬 상세·CINEMATIC 전환을 확인한다.

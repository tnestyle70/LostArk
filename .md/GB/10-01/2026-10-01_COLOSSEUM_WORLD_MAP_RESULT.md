# 콜로세움 M 월드맵·스퀘어홀 이동 결과

## G00. Client 구현 상태

`CMainApp`의 기존 M키 월드맵에 Colosseum의 replica marker snapshot과 typed player controller를
연결했다. 콜로세움에서는 기존 베른 성 목적지 지도를 전체 보기로 열고 스퀘어홀1~3의 지도 아이콘,
지역 트리와 확인창을 사용한다. 다른 월드의 좌표인 Colosseum local/party/boss/NPC 마커는 숨기고
현재 위치·플레이어 표시 버튼과 베른 전용 출항 요청은 비활성으로 유지한다. 이 귀환 요청은
요금을 차감하지 않으므로 해당 확인창에만 요금/실링 아이콘을 표시하지 않는다.

`Level_Development`는 자기 소유의 기존 controller와 read-only marker 수집을 공개하며,
Colosseum에서도 `Pump_ServerApprovedWorldTransfer(m_eLevel)`를 호출한다. 따라서 Server의
Bern 입장 승인이 도착하면 기존 Level transition과 session handoff를 소비한다. 새 UI, packet,
Resources, local teleport 경로는 추가하지 않았다. Bern의 기존 노래/암전과 출항 흐름은 그대로다.

수정한 C++은 `MainApp.cpp`, `Level_Development.h/.cpp`, `WorldMapWindowView.h/.cpp` 다섯 개다.
기존 MainApp profiler 수정과 PlayerController의 다른 작업은 보존했다. 모든 수정 파일의 UTF-8
BOM 없음과 CRLF를 유지했다. 기존 C++의 프로젝트/filter 등록을 확인했으며 추가 등록은 없다.

## G01. Client 자동 검증

- `cl /Zs` Debug 및 Release 각각 MainApp, Level_Development, WorldMapWindowView 세 translation
  unit 성공, exit0·컴파일 오류0. 최종 로그는 `out/ColosseumWorldMap20261001/client-debug-final.log`,
  `client-release-final.log`이며 명령과 response file도 같은 폴더에 보존했다.
- 변경 파일 `git diff --check` 성공. `client-edit-receipt.json`에 편집 전후 SHA256과 원본 백업,
  `client-static-validation.json`에 현재 파일 hash·인코딩·개행 검사 결과를 남겼다.
- 기존 JSON을 읽어 베른 default area 한 개, destination ID1~3, 설치된
  `UI/Minimap/Maps/BernCastle.png` 존재를 확인했다. Data/Resources는 변경하지 않았다.
- M은 기존 전경 창/텍스트 입력/연출 억제 조건 안에 있고 UI 클릭은 기존 pointer scope와
  frame claim을 사용한다. 지도 확인은 한 번 소비되는 기존 hole request를 통해서만 제출한다.
  로딩 진입 시 기존 `Close_RuntimeWindowsForLoading`이 열린 창을 정리한다.

처음 scratch 구문 검사의 추가 `WIN32_LEAN_AND_MEAN` 정의가 Windows SDK의 `byte` 충돌을
만들었다. 제품에 없는 해당 정의를 검사 명령에서 제거한 최종 검사로 위 성공을 확인했다.
제품 파일이나 공통 빌드 설정은 이 검사 문제 때문에 수정하지 않았다.

## G02. Server 구현과 통합 컴파일

`Handle_UseSquareHole`은 Colosseum의 ID1~3 요청을 기존 `Transfer_PartyTo`의 singleton
transaction으로 연결한다. target admission, 정확한 squarehole landing, initial frames와
reliable FIFO 준비를 끝내기 전에는 source player·party·session binding을 바꾸지 않는다.
성공 시 기존 `ServerApp::Transfer_SessionWorld`가 Bern binding을 commit한다. 대상 파티나
roster는 새로 만들지 않고 source의 남은 파티원은 유지한다. 인벤토리·장비·재화·내구도·
닉네임·직업·voice를 보존하며, 빈 인벤토리가 fresh-entry 기본 지급으로 바뀌지 않게 했다.

기존 DebugTeleport 검사에 실제 ServerApp 경유 세 목적지, invalid/ship/dead/busy/중복 거절,
막힌 landing과 가득 찬 FIFO에서 기존 상태 보존, 정확착지와 source party 잔류를 추가했다.
실행 명령은 `Server/Bin/Release/Server.exe --debug-teleport-contract-test`다.

VS18 Insiders의 Release 실제 OBJ 컴파일 결과:

- Client 변경3CPP: 성공, 오류0. 기존 C4819 경고는 각각22/16/42개다.
  `out/ColosseumWorldMap20261001/compile-*.log`에 명령·출력을 보존했다.
- Server 전체 `ClCompile`: 성공(exit0), `ServerPlayer.h` 및 room 공용 header 변경을
  모든 참조 translation unit에 반영했다. `out/WaterpangEndInspection20261001/server-compile-all.log`.
- 독립 코드 검토와 `git diff --check` 성공.

## G03. 실행 파일 점유와 사용자 확인 경계

초기 검증 당시 Release Product runner는 `Client/Bin/Release/Client.exe` PID23052와
`Server/Bin/Release/Server.exe` PID32460의 실행 점유로 링크 전에 차단됐다.
결과는 `out/ColosseumWorldMap20261001/product-build.log`와
`out/BuildPipeline/runs/20260930T172410649Z-release-product.json`에 있다.
위 OBJ·구문 검사 성공을 새 EXE 링크·contract 실행 성공으로 기록하지 않는다.
사용자가 현재 실행 유지로 답했으므로 두 제품 프로세스를 종료하지 않고 제품 EXE 교체를
보류했다. Client 실행·조작·화면 캡처도 수행하지 않았다.

컴파일한 Server OBJ를 입력으로 사용하는 별도 검증 EXE만
`out/WaterpangEndInspection20261001/isolated-server`에 링크했다. EXE/PDB/LTCG/IMPLIB
출력은 모두 out에 두며 제품의 IntDir/OutDir 설정이나 link tracking을 바꾸지 않았다.
기존 제품 Server.exe·link command tracking·Server.iobj의 SHA-256 보존을 확인했다.
`LOSTARK_SERVER_DATA_ROOT`로 게시 snapshot을 읽는 in-process 계약 검사이며 listener를
시작하거나 현재 실행 중인 서버에 접속하지 않는다.

새 Colosseum 구간32검사 전부 PASS. 다만 기존 `--debug-teleport-contract-test` 전체는
Mario 근접 공격 기대값 검사40건이 실패해 exit1이며 전체 성공으로 기록하지 않는다.
`squarehole-contract.log`와 `contract-summary.json`에 세 목적지의 실제 성공/실패 보존과
해당 실패 메시지를 보존했다. 기존 제품 EXE의 격리 복사본도 같은 검사에서 동일한40개
실패를 재현했다(`squarehole-baseline.log`). 기존 Mario 테스트의 고정 피해100 기대값과
9월30일 `6a988d7037`의 Server ±10% 피해 정책이 맞지 않는다. 이 별도 기존 실패를
콜로세움 이동 구현 실패로 혼동하지 않으며 해당 무관한 코드·테스트는 수정하지 않았다.

사용자 확인 경로는 콜로세움 인트로 종료 → M → 베른 스퀘어홀 아이콘 또는 트리 선택 → 확인 →
베른 착지다. M 재토글, 확인창/지도 순서의 ESC, 창 위 클릭의 gameplay 중복 소비 방지,
목적지 착지 뒤 이동은 실제 Client에서 확인이 남는다.

## G04. 후속 Server 제품 링크와 경기 종료 귀환 계약

2026-10-01 정상 Server [Debug 빌드](../../../out/GuidePersonal20261001/server-guide-final-debug-build.log)와
[Release 빌드](../../../out/GuidePersonal20261001/server-guide-final-release-build.log)가 실제 제품 EXE
링크까지 성공했다(각 경고 0/오류 0). G03의 Server 실행 파일 점유·링크 보류는 해소됐다.
최신 Release `--colosseum-match-contract-test`는 **134 PASS, 0 FAIL, exit 0**이다.

[경기 계약 로그](../../../out/GuidePersonal20261001/colosseum-match-release-complete.log)에서
FINISHED 상태의 사망한 인간도 기존 원자적 Bern 전송으로 귀환하고, 정상 class profile HP를
복구하며 match/mercenary/자동 team party 권한을 남기지 않는 것을 확인했다.
[종료값 JSON](../../../out/GuidePersonal20261001/contracts-release-complete.json)에 결과를 기록했다.
이 검사는 G03의 별도 `--debug-teleport-contract-test` 전체 Mario 기대값 실패를 재검증하거나
해소한 증거는 아니다. 최종 이동 수정·실패 알림 연결을 포함하는 Client 전체 Product 빌드는
아직 대기 중이며 M 지도 입력·착지 화면은 사용자 확인 대상이다.

## G05. 최종 월드맵 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.

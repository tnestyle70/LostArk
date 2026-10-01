# 레이드 캐릭터 인계와 강화 이름표 겹침 수정 결과

## G00. 확인한 원인과 반영

베른에서 입장 승인을 받은 뒤 비동기 모델 준비 취소가 끝나기를 기다리는 동안
기존 레벨이 다음 프레임에도 갱신된다. 일회성 ENTER_ACCEPTED는 이미 소비했으므로
기존 replication이 목적지 PLAYER_SPAWNED를 먼저 소비할 수 있었다. 새 레이드에
spawn이 남지 않으면 캐릭터가 없고 카메라도 follow target을 얻지 못한다.
Server는 reliable 초기 묶음에 자신의 spawn을 포함하며 snapshot 병합은 그 순서를
보존한다. 이번 수정은 Client의 소비 인스턴스 수명 경계에 적용했다.

CClientReplication은 Initialize 성공 때 world inbound generation을 소유한다.
Update는 다른 세대의 큐와 후속 asset 준비를 소비하지 않는다. 처리 중 연결 종료나
Reset이 일어나도 추가 소비를 중단한다. 기존 disconnect 정리는 가드보다 먼저
실행하고 Reset_World에서 소유 세대를 해제한다. world ID가 같은 재입장도 구분한다.
Character Select·Bern·Valtan·Kouku·Development 계열의 초기화가 모두 승인 이후이며,
Reset 뒤 재초기화 없이 정상 소비를 요구하는 경로가 없음을 별도 리뷰로 확인했다.

장비 강화 창은 이름표 가림 영역이 중앙 패널442px에만 지정돼 있었다.
CMainApp::Register_UITextOccluders를 전체 WindowBg948px로 수정했다. 대기·성공·실패
배경도 이 rect와 동일하다. 실제 viewport 비율로 변환하는 기존 가림 경로를 사용하여
WORLD 이름표를 가리고 같은 WINDOW_ITEM_UPGRADE층의 강화 문구는 보존한다.
새 C++ 파일·project/filter·protocol·shader·렌더링 옵션 변경은 없다.

## G01. 완료한 검증

- `test_replication_world_handoff.py`의 생산 Initialize/Update/Reset·NetworkManager
  승인/codec/큐 추출 회귀는 Debug7/7·Release7/7 PASS, exit0이다. 수정 전
  `8b58818af`도 같은 fixture에서 컴파일에 성공하며 발탄/쿠크/동일월드 새 세대·
  Reset·handler close의5개 assertion이 실패한다. 같은 세대 정상 FIFO와 기존
  disconnect는 수정 전에도 PASS다. 전환을4프레임 지연해 초기 spawn 보존을 검사했다.
  graphics/navigation/presentation과 실제 socket 연결은 fixture로 격리했으며
  실제 Client 실행이나 화면 재현으로 기록하지 않는다.
- Client x64 Debug 정상 `/t:Build` exit0, Client.exe 링크·DLL 배포 성공.
- Client x64 Release `/t:ClCompile` exit0. 기존 EngineSDK·MainApp 인코딩 경고와
  Debug DirectXTK PDB 경고는 남았으며 전체 파일 인코딩을 변환하지 않았다.
- Portable package 도구22검사 PASS. plan-only는 payload2766개·직접Data2178개·
  CSO256개·numeric binding587개·protocol132로 성공했다.
- 이전 Guide source/게시본 재검사 PASS. Client·Server Guide 일치, NPC anchor11곳,
  아바타·물약·레이드·PvP SPACE_ENTER, 물약1명, 기존 대사·귀환 연결 보존을 확인했다.
- 관련 C++ UTF-8/BOM 없음·CRLF 보존, git diff --check PASS.

로그는 `out/GuideRaidFix20261001/`의 client-debug-build.log,
client-release-compile.log, package-tests.log, package-plan.log,
guide-data-verification.log다. 기존 Guide의 Debug Server 계약97개 PASS 증거는
09-27 GUIDE_AI_TOOL RESULT G11과 out/GuideNpcFix20261001에 보존돼 있다.
회귀 stdout과 실제 추출본은 같은 폴더의 replication-regression 아래
current.log/current.cpp, baseline.log/baseline.cpp, release.log/release.cpp다.
재실행은 x64 MSVC developer 환경에서 아래와 같이 수행한다.

```powershell
python Tools/Network/test_replication_world_handoff.py --output out/GuideRaidFix20261001/replay-debug --configuration Debug
python Tools/Network/test_replication_world_handoff.py --output out/GuideRaidFix20261001/replay-release --configuration Release
python Tools/Network/test_replication_world_handoff.py --output out/GuideRaidFix20261001/replay-baseline --baseline-replication out/GuideRaidFix20261001/replication-regression/ClientReplication.before.cpp
```

## G02. 아직 완료하지 않은 배포·화면 확인

현재 실행 중인 Release Client·Server의 링크 잠금을 해제한 뒤 정식 Product 빌드와
새 Release ZIP 생성·CRC/hash·launcher --check 검증이 필요하다. 이번 CPP 수정은
아직 실행 중인 기존 EXE에 적용된 상태가 아니다. 실제 Client/UI 실행과 반복 입장,
강화 창의 시각적 겹침 확인은 사용자가 수행한다.

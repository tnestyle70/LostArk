# 쿠크 레이드·공격 판정·원본 사운드 통합 결과

2026-09-19 현재: **main 병합, 소스 연결, 실제 저장본 설치, Gameplay/World/Map 게시, Release Product 빌드 완료. 폴더 선택 EXE가 있는 바이너리 전용 ZIP 생성 및 격리 설치 검증 완료.** 실제 Client 화면·청취는 실행하지 않았다.

## 통합한 실행 계약

- main `d2d3563b4d50ea545b1d25dd5a94923e9304e0d4`를 fast-forward하고 사전 미커밋 변경을 복원했다. revision/codec/packet ordinal 충돌 3개를 해결했다. 원본 byte backup과 stash를 보존하며 기존 main 메시지 번호를 유지했다. protocol은 93이다.
- Release 진입 콜라이더의 기존 PLAY_SEQUENCE가 Server raid 준비를 시작한다. 실제 참가자 1~4명 모두의 Action·Sequence·Gameplay revision 준비 ACK 뒤 공통 Server 시각으로 연출과 스폰을 진행한다. 솔로에서 가짜 플레이어 4명을 만들지 않는다.
- 보스 사망 후 자동 10초 이동은 제거했다. WAIT_GATE에서 main의 클리어/MVP·관문 투표 UI를 기다리고, 전원 진행 승인 후 다음 관문 연출을 시작한다. 재시작은 현재 관문 연출부터, 마지막 관문은 나가기/재시작 UI를 유지한다. 거절·투표 만료는 현재 대기를 유지한다.
- 1관문15개 → 2관문14개 → 3관문16개의 저장 Flow를 소비한다. 목록 끝에서 자동 반복하지 않고 보스 사망을 기다린다. 2관문 단독 패턴의 반대 보스는 Idle, 실제 멤버 둘이 든 Bundle은 함께 진행한다.
- 카드미로는 해당 문양 처치 위치에 개인 포탈을 생성하고, 망원경 담당자만 시야를 확장한다. 담당자 외 N-1명이 중앙으로 돌아오면 전원이 Gate2로 복귀하고 피자로 이어진다.
- CARD_DICE_BIND는 Duration 동안 살아 있는 한 명을 무작위로 제외하고 나머지만 속박한다. 솔로는 속박하지 않는다. 카드 추적은 같은 비속박 대상을 사용하며 종료·취소는 자신의 속박만 해제한다. 돌진 카운터의 비어 있던 judgementKind를 COUNTER_WINDOW로 연결하고 성공 후 그로기를 재생한다.
- P88/P91/P92/P93는 각각 Mario1~4 진입 창을 사용한다. 파티는 실제 입장 성공 다음 tick에 P33, 솔로는 실제 복귀 착지 후 P33이다. Parent의 긴 저장 duration은 유지한다. 마지막 tick에 큐된 입장도 완료 정리보다 먼저 commit한다.
- P33의 비진입자는 아이언메이든 (-7.07,1.32,934.43)에서 1.25m 간격으로 배치한다. 3~4인일 때만 한 명을 cage에 속박한다. 보스는 (5.96,1.30,950.59)에서 중앙을 보고, 무력화 성공은 P42, timeout은 전원즉사다. 자연 출구와 복귀 명령은 같은 pin한 귀환점을 사용한다.
- 갈고리와 즉사 칼날은 한 번, 일반 칼날은 반복한다. 즉사 칼날은 지정 보스 위치에서 아이언메이든으로 이동하며 접촉 즉사를 사용한다. 두 칼날의 이펙트는 모델 spin을 상속하지 않고 위치를 따른다. 새 world39는 데이터 정의이며 새 WModel은 추가하지 않았다.
- 원본 갈고리·즉사 칼날 선택, MAP 그룹 위치/yaw 수정과 World Preview를 연결했다. 쇼타임 추적 과녁은 보스 생성 위치에서 참가자별로 시작한다. 회전 응답 계수는 1→10이고 authored 시작 시각은 유지한다.
- latejoin admission에 현재 GateProgress 상태를 함께 보낸다. 관전자는 투표할 수 없다. WAIT_GATE의 첫 관문 화면 초기화, G1 재시작 때 기존 책/standing owner 반환과 실패 복원을 연결했다.

## 저장본·사운드·판정

설치본은 Action1750 / Sequence64 / Gameplay.world8797 / WorldSequences2129이다. 최신 디스크 hash 대조·백업·원자 교체 transaction `out/transactions/kouku-raid-3f48276546204843b1c25a1cab3c5bfd`에서 Data5개+새 WAV18개를 설치했다. 충돌0, 설치23개 파일 hash가 후보와 같다.

사운드는 원본487 event 중480개를 복원해 Action691곳+Sequence31곳, 총722곳에 연결했다. 새 WAV18개는227,088,088bytes이며 기존1,042개를 재사용한다. 사용자 최종 요청에 따라 ZIP에 사운드/Resources를 넣지 않고 신규18개 상대 경로만 `NEW_SOUND_PATHS.txt`로 전달한다.

공격은 원본 직접 shape193개(ring8개 포함)+저장 MAP impact31개, 총224개 후보를 설치했다. 동적 투사체17개 typed template은 기존 Server CombatObject 판정을 사용한다. 마리오 World collider는 normal24+hook30+instant8이며 hook30은 한 번의 갈고리 이동을 두 가시성 구간으로 나눈 판정이다. 이펙트 particle마다 중복 collider를 만드는 구조가 아니다. 일반 피해10% 및 과녁 반경 등 튜닝값은 원작 수치 복원과 구분한다.

원본 callback116개 중 기존 PURSUIT27·Albion3·현재 MAP impact로 표현한 원본14·비피해 NPC/FX11을 구분했다. 나머지61개는 원점/조건/반복 시각을 확정하지 못해 임의 판정을 넣지 않았다. 사운드 control-only/재생 원본 없음7 event 및 원본 clock/key가 모호한9 clip binding도 미확정이다. 별도1 clip은 trim 구간에 event가 없어 추가 불필요하고, P8 SCENE03A 소리는 parent P4에 이미 있어 중복하지 않는다.

## 검증과 게시

| 검증 | 실제 결과 |
|---|---|
| 최신 Product projection | 패턴80, Bundle8, Stage455, P35 독립 window81 |
| Debug Server/Shared fresh build 및 통합 실행 | 7,435 checks / failures0 |
| Release 진입 collider·ACK·관문 UI·latejoin 초기 snapshot | 660 checks / failures0 |
| Release Mario 실제 contact·party/solo·복귀·jump | 108 checks / failures0, 12경우 authoredEntry=1 |
| 카드미로 실제 1~4인 room | 문양 포탈·담당자 시야·N-1 귀환 failures0 |
| Protocol93 codec | 464 checks / failures0 |
| Client gate 정책·presentation ownership | 15 +42 checks / failures0, 관련 Debug/Release TU compile PASS |
| 설치기/Python | CAS15, raid projection7, blade2, sound7 tests PASS |
| 프로젝트 XML·diff | 8개 XML parse, git diff --check PASS |
| Release Product | 전체 `20260918T151246346Z-release-product.json` PASS; 최종 변경 포함 증분 `20260918T151534562Z-release-product.json` PASS |

실제 Gameplay/World/Map publisher를 실행했다. live Gameplay.bootstrap은38,926rows /13,282,173bytes이며 검증 후보와 SHA256 `2320b718e07e27182596fe29ae1adcc8e66f0b8e532b70bc48952ee5d9501d42`가 같다. World bootstrap2개도 byte 동일하다. Map publish 첫 시도는 파일 점유로 기존 파일을 보존한 채 실패했고 재시도 성공했다. runtime WorldSequences hash는 `ef0f39be224edcf400dab46d5569884c99eb10eb311330d1d06fa2cc926f8195`다.

쇼타임 Profiler 평균1913.148ms 중 HistoryUpdate1739.255ms(90.91%)가 반복 루트 pose 재계산에 쓰였다. checkpoint/pose cache 수정 후 실제 CModel 함수 반복조회는44299ms에서20.7249→0.001721ms,55637ms에서85.336→0.0016825ms다. 이는 전체 effect seek/GPU/FPS 측정값이 아니다. 사용자의 최종 화면·청취와 실제4개 Client 동시 플레이는 미실행이다.

## 배포 재수정: DataFiles·Navigation·ESC 커서 (2026-09-19)

기존 `LostArk-Release-20260919-EXE.zip`은 Client 모듈7개와 CSO141개만 설치하고
Data/DataFiles를 제외한 배포본이었다. 이번 요청에서는 그 ZIP을 전체 실행 배포본으로
사용하지 않는다. Git 동기화와 publish 성공, ZIP의 실제 전달 범위를 각각 확인한다.

### 확인한 원인과 실제 수정

- 8f15a3c35(PR409)에 다른 세션의 쿠크·쇼타임 변경이 통합됐다. Action1750,
  Sequence64, World8797, WorldSequence2129, protocol93을 한 묶음으로 사용한다.
- Navigation은 Bern·Character Select 지역파일12개와 Server manifest2개가 빠졌고,
  발탄 Server blocker는104개로 정본/Client101개와 달랐다. Server owner로 전체 게시한
  뒤 Client/Server 각각36개, SHA36쌍 동일, manifest 참조 누락0을 확인했다.
- Composition publisher의 WorldSequence v3 strict validator에서 기존 `colliderTracks`와
  `loopFullPresentation`이 빠져 있었다. Map publisher/Client와 같은 시간·shape·damage·
  binding·loop 제한을 연결했다. authoring 필드 삭제나 unknown-field 완화는 하지 않았다.
- ESC 옵션 팝업은 클릭을 먼저 소비하여 `Is_Clicked`가 자기 항목 선택을 거부했다.
  `SystemOptionWindowView.cpp`에서 popup 선택 후 하위 UI를 차단하도록 순서를 고쳤다.
  커서 지원125개와 SystemOption 이미지59개는 로컬에 있어 자산 누락은 원인이 아니었다.

### 실행한 검증

- Composition unittest49개 PASS, 실제 전체 source graph validate PASS. 현재
  WorldSequence326개 instance 원본 보존 및 collider/loop 실패 입력 거부를 확인했다.
- 실제 옵션 입력 함수 본문을 사용한 CPU probe는 이전 코드에서 선택 관련3건 FAIL,
  수정 후 선택·동일항목·외부클릭·hover·held 입력6건 PASS. 하위UI click 누수0.
  OS 커서 적용/렌더는 stub이며 실제 화면 판정은 수행하지 않았다.
- Release Product PASS(36.9초): Client OBJ1/EXE1, Engine/Shared/Server 재컴파일0,
  CSO/PCH 재생성0. 기존 C4819와 DirectXTK PDB LNK4099 경고는 남아 있다.
  증거 `out/BuildPipeline/runs/20260918T161133688Z-release-product.json`.
- Server owner: Kouku/World/Navigation/파괴/Gameplay/아이템/탈것/칭호/발탄보상 PASS 또는
  현재 입력·출력 hash와 일치하는 REUSED. Client owner 최종 재시도도 전체 PASS/REUSED.
- publisher 및 build 로그는 `out/RuntimeRepublish20260919/`, 옵션 probe는
  `out/CursorOptionClick20260919/`에 있다. `git diff --check` PASS.
- 새 설치 wrapper fixture: Data/runtime 백업, 최초/재설치, SHA/경로/중복 거부,
  잠긴 Server 파일에 의한 실패 시 Data/runtime 복구 PASS. Client/UI 실행0.

### 최신 main 통합과 최종 배포 기준

PR410의 main417b2b126을 충돌 없이 통합했다. VehicleCatalog와 탈것 Effect8개의
직접 소비 JSON9개가 추가되며 새 Resources 참조는 없다. 기존 쿠크34개와 ESC 옵션3개를
합쳐 Data 보충분46개를 배포한다. Resources/신규18WAV는 기존 Drive 경계를 유지한다.

통합 Release Product도 PASS(94.1초): Server OBJ20/EXE1, Client OBJ85/EXE1,
Engine/Shared/CSO/PCH 재생성0. 최종 코드 build 증거는
`out/BuildPipeline/runs/20260918T161822826Z-release-product.json`이다.
Client/Server owner도 통합 후 다시 실행해 전체 PASS/REUSED를 확인했다.

리뷰에서 collider damagePercent의 정수값 실수(0.0/10.0) 허용도 기존 소비자와 맞췄다.
finite·범위·정수값 검증은 유지하며 bool/fraction/NaN/Inf를 거부한다. 관련7개 test PASS.
시간 필드는 기존 gameplay projector와 같은 int-only를 유지한다.

우리 수정 PR411을 main에 병합한 뒤 `LostArk-Release-20260919-Full.zip`으로 확정한다.
최종 ZIP의 SHA·파일수·실제 설치 검증은 `out/RuntimeRepublish20260919/delivery-result.json`과
패키지의 `README_사용방법.md`를 따른다. 이전 EXE 전용 ZIP을 전체 실행 배포본으로 쓰지 않는다.

실제 화면·음향·4인 Client 동시 플레이는 사용자 확인 대상이다. 게시와 설치는 실행 중
Server 메모리나 도구 draft를 자동 갱신하지 않는다. Client/UI를 자율 실행하지 않았다.


## Release 초기 종료 진단과 실행기 실패 판정 (2026-09-19)

### 확인한 범위와 구현

사용자는 다른 PC에서 Full ZIP을 실행했을 때 흰 창이 뜬 뒤 즉시 종료된다고 보고했다.
그 PC의 실제 종료 코드·startup 로그는 아직 받지 않았으므로 원격 종료 원인은 미확정이다.
현재 로컬 데이터 검증이나 아래 dummy 성공을 다른 PC의 제품 진입 성공으로 기록하지 않는다.

기존 `Client/Default/Client.cpp`의 `WriteExitDiagnostic`은 `_DEBUG` 안에 있어 Release에서
`CMainApp::Create()` 실패가 exit 1로 끝나도 파일 기록이 없었다. Debug 전용 조건을 제거하고
reason·HRESULT·PID를 `Client/Default/ClientExit.user.log`에 기록한다. 초기 Engine 생성 실패에서도
쓸 수 있도록 `Get_CurrentLevelID()` 호출은 제거했다. 로그 경로는 실행파일의
`Client/Bin/Release` 위치에서 `Client/Default`를 해석하므로 실행기의 작업 디렉터리에 의존하지 않는다.

`Client/Private/MainApp.cpp`는 `ClientStartup.user.log`에 Engine, Rendering, Network, ImGui,
Fonts, Static prototypes, Effect Catalog, UI, Lobby의 시작·결과와 HRESULT를 남긴다.
Rendering·Effect Catalog 실패의 기존 상세 status를 보존하고 Network는 WSA 오류 코드,
Lobby는 기존 LevelTransitionService 상태를 함께 기록한다. HRESULT를 반환하는 초기화 단계는
원래 반환값을 기록·전파한다. Client.cpp의 UTF-8 BOM과 MainApp.cpp의 BOM 없음, 두 파일의
기존 비ASCII 바이트 및 CRLF는 유지했다. 진단 추가는 빠진 리소스를 대체하거나 실패 admission을
완화하지 않는다.

실행기 `out/RuntimeRepublish20260919/package-tools/LostArkLauncher.cs`는 시작 명령·작업 폴더·
Data/Resources root·PID와 10초 이내 종료 코드를 로그에 남기고 해당 실행의 Client 로그를
실패 메시지에 붙인다. 10초 뒤 계속 실행 중이라는 관측은 startup 성공으로 단정하지 않는다.
검토 중 누적 로그의 과거 `failed` 행과 정상 경로 문자열의 `failed`까지 현재 실패로 처리하는
결함을 재현했다. 실행 직전 로그 byte 길이를 저장해 이번 실행에서 추가된 내용만 읽고,
문자열 부분 일치 대신 HRESULT의 실패 bit를 검사하도록 고쳤다. `failed`라는 단어가 없는
HRESULT 실패도 이제 놓치지 않는다.

### 실행한 검증

- 실제 C++ 두 logger 본문을 가져온 `/O2 /DNDEBUG` 콘솔 probe의 컴파일·실행 PASS.
  Rendering `0x80004005`, Effect `0x80070057`, 원래 상세 문자열과 종료 reason/PID가
  실행파일 기준 `Client/Default`에 기록됐다. Engine·Client·UI 생성은 0회다.
  증거는 `out/ReleaseStartupDiagnostics20260919/logger_probe.compile.log`와 그 하위
  `Client/Default/ClientStartup.user.log`, `ClientExit.user.log`다.
- `Prepare-Launcher.ps1`로 WinForms 실행기를 컴파일한 뒤 실제 private `LaunchClient`를
  reflection으로 호출했다. 대상은 별도 콘솔 dummy이며 제품 Client와 Main/UI는 호출하지 않았다.
  exit 1·상세 실패·정상 exit 0·이전 실패 로그·새 실패 로그·정상 `failed` 문자열·단어 없는
  HRESULT 실패의 7개 시나리오는 수정 전 3 FAIL, 수정 후 7 PASS다. Data/Resources 환경 로그와
  한글·공백이 있는 fixture 경로도 확인했다. 이전 실패 내용은 새 실행 메시지에서 제외됐다.
- 실행기 검증 근거는 `out/RuntimeRepublish20260919/package-tools/launcher-diagnostic-probe/`
  의 `before-result.log`, `after-result.log`, 각 fixture의 `launcher.log`다.
- 변경 C++와 이 RESULT의 `git diff --check` PASS. 이 절의 probe는 제품 Release 빌드나
  실제 상대 PC 실행을 대신하지 않는다. 새 제품 빌드·최종 ZIP 반영은 배포 root의 결과로 확인한다.

### 아직 확인하지 않은 것

원격 PC의 누락 파일·D3D 장치 생성·font/shader/catalog admission 중 어느 단계에서 종료됐는지는
확정하지 않았다. 새 배포 실행 후 실제 `ClientStartup.user.log`, `ClientExit.user.log`와 실행기
로그로 구분해야 한다. Client 자동 실행·UI 조작·화면 캡처·시각 PASS는 수행하지 않았다.


## Bern 군단장 레이드의 쿠크 투표 UI 보정 (2026-09-19)

### 확인한 입력·입장 경계

`CRaidEntryPreviewView`의 기본 선택은 쿠크이며 `RAID_DEFS`의 쿠크 탭은
`RAID_ENTRY_TARGET::KAKULSAYDON`을 제출한다. Bern은 Render 뒤 intent를 한 번 소비하고
`IPlayerCommandSink::Request_RaidEntryPropose`로 NPC stable ID와 target을 보낸다.
Server prompt의 proposalId와 target을 받아 수락·거절 창을 열고 응답은 동일 proposalId를
`Request_RaidEntryRespond`로 돌려준다. `S2C_ENTER_ACCEPTED`가 온 뒤의 Level 전환은
기존 `CLevelTransitionService::Pump_ServerApprovedWorldTransfer`가 담당한다.
UI의 Mouse Claim은 gameplay 차단을 위한 것이며 `CUIInputRouter::Is_Clicked` 자체를
거부하지 않으므로 ESC 콤보와 같은 자기 클릭 차단은 이 입장 버튼에 없었다.

쿠크 투표여도 확인창이 항상 발탄의 “부활한 마수의 심장”을 표시하는 결함을 확인했다.
`RaidEntryPreviewView.h/.cpp`에 열린 투표의 target을 저장하고 같은 `RAID_DEFS`에서
레이드명을 찾도록 수정했다. 쿠크는 “한밤중의 서커스에 입장하시겠습니까?”로 표시한다.
파티원의 로컬 선택 탭이 발탄이어도 Server가 보낸 쿠크 target을 표시하며, 수락·거절·
취소·ESC 닫기에서 표시 target을 정리한다. 알 수 없는 target은 창을 열지 않는다.
Shared packet decoder의 기존 target/proposalId 검증과 Server 승인 경계는 유지한다.
두 C++ 파일의 기존 UTF-8 BOM 없음·CRLF를 유지했고 Data/Resources 계약은 바꾸지 않았다.

이 UI 보정만으로 다인 쿠크 입장 완료를 주장하지 않는다. Server의 기존 party transfer가
VALTAN_ARENA만 허용하던 별도 결함은 같은 작업의 Server 담당 변경과 검증으로 연결한다.
Character Select의 Debug O키 레이드 창과 매칭/파티찾기 버튼은 기존 시각 preview 범위다.

### 실행한 검증과 남은 확인

실제 `Render`, `Render_ConfirmStep`, `RenderText_ConfirmStep`, `Consume_Intent`,
Open/Close 본문과 `CUIInputRouter` 클릭 본문을 추출해 native Release 콘솔 probe를 실행했다.
입력 hit test에는 현재 두 UI JSON의 실제 버튼 rect를 사용했다. 창·DirectX·제품 Client는
생성하지 않고 텍스트 제출과 intent 결과를 관측했다.

- 쿠크·발탄 입장 클릭의 typed target, intent 한 번 소비, 눌린 버튼 유지 시 재발의 없음: PASS.
- 로컬 탭과 다른 Server target을 받은 확인창의 레이드명: 쿠크 수정 전 FAIL, 수정 후 PASS.
- 수락·거절의 동일 proposalId와 accepted 값, 창 닫기, 마우스 입력 claim: PASS.
- Server 취소 정리와 알 수 없는 target의 발탄 fallback 차단: PASS.
- 전체 12개 시나리오: 수정 전 2 FAIL, 수정 후 12 PASS. probe 컴파일 PASS.
  증거는 `out/RaidEntryVote20260919/before.events.log`, `after.events.log`,
  `before.compile.log`, `after.compile.log`와 재현 스크립트 `run_probe.py`다.
- 두 UI JSON parse와 변경 파일 `git diff --check` PASS.

제품 Release 빌드와 새 ZIP 반영은 배포 작업의 최종 결과로 확인한다. 위 검증은 실제
다인 Server 이동이나 Client 화면 확인을 대신하지 않으며 최종 시각 검증은 사용자 몫이다.

## Full v2 배포와 쿠크 UI 입장 최종 반영 (2026-09-19)

군단장 레이드 UI의 쿠크 투표는 이미 target을 전달했지만 `Transfer_PartyTo`의 마지막
target 검사에서 Valtan만 허용해 2~4인 쿠크 이동을 거절했다. KAKULSAYDON도 같은 기존
원자적 transfer에 허용하고, Shared의 `S2C_PARTY_TRANSFER_RESULT` writer/reader도 쿠크
실패 사유를 전달하도록 맞췄다. packet 크기·ID·protocol 93과 기존 Valtan 경로를 유지한다.
맵 입장과 레이드 시작은 구분하며 실제 Raid 준비·인트로는 기존 입장 collider가 시작한다.
Server의 실제 1~4인 수락·거절·이동과 실패 시 source 보존 검증은
`out/KoukuRaidEntry20260919/focused.run.log`에서 failures 0으로 확인했다.

Release Product 빌드는 `20260918T164956975Z-release-product.json`에 이어 Shared 메시지
보정을 포함한 `out/BuildPipeline/runs/20260918T165309256Z-release-product.json`까지 PASS다.
첫 빌드는 Server OBJ 2·Client OBJ 6과 양쪽 EXE, 마지막 빌드는 Shared·Server 변경분과
Client/Server 재링크를 반영했다. Engine·PCH·CSO 재생성은 없었다. 기존 코드 페이지와
DirectXTK PDB 경고가 있었고 컴파일·링크 실패는 없었다. 사용자 수동 빌드의 초기화 로그와
이번 제품 빌드 및 focused 검증을 구분하며 에이전트는 Client/UI를 시작하지 않았다.

첫 Full의 Data 보충분은 46개였고 EffectCatalog의 direct-authored 참조는 1,167개였다.
그 중 1,140개 참조 JSON이 보충분에 없었다. 수신 PC의 오래된 Data에서 시작 실패를
일으킬 수 있는 이 조건을 제거하기 위해 v2는 현재 catalog 참조 전체와 RaidEntry UI 2개를
포함한 Data JSON 1,188개를 준비한다. 전체 Data 디렉터리를 무조건 복사한 구성은 아니다.
원격 PC가 git pull 후 실행됐다는 사용자 보고는 Data 의존성과 부합하지만 해당 PC의
실제 실패 로그가 없으므로 어느 파일이 최초 실패했는지는 확정하지 않는다.

실행 ZIP의 구성은 Release runtime 458개(Client DataFiles 150, Server DataFiles 159 포함),
Data JSON 1,188개, 실행기·설치기·안내다. 최종 ZIP의 hash·크기·main commit과 무결성 결과는
`out/RuntimeRepublish20260919/delivery-result.json`으로 확인한다. 대상은 바탕화면의
`LostArk-Release-20260919-v2/LostArk-Release-20260919-Full-v2.zip`이며 이전 배포본을 보존한다.

차원술사 탑승 scale은 별도 [탈것 결과 G09](../../JS/09-14/2026-09-14_VEHICLE_ADDITIONS_RESULT.md)와
`Tools/VehiclePipeline/DimensionMasterRiderScaleRepair.receipt.json`을 따른다. Resources 6개는
실행 ZIP에 넣지 않고 같은 전달 폴더의 `Resources-Drive-Update`에 준비했다. 이 PC의 교정과
다른 PC의 Drive 동기화는 별개다. 받는 PC는 새 Client뿐 아니라 공유 Server도 갱신하고,
차원술사 수정 Resources도 적용한 뒤 사용자가 실제 입장·탑승 화면을 확인한다.

## 4인 검증 Flow 재설정·쇼타임 연출·Release lifecycle (2026-09-19)

### 실제 변경

사용자가 P29 내려치기의 크래시를 확인하고 Flow 교체·게시·새 ZIP을 요청했다.
조사 시 저장본·Encounter·Server bootstrap은 모두1750으로 일치했으므로 기존 Flow의
게시 누락은 아니었다. 현재 저장본을 백업하고 hash 재확인 뒤 필요한 필드만 원자 교체했다.
revision1751에서 Flow, 1752에서 P76 연출을 반영했고, 최신 GATE3와 나머지 컷씬까지
반영한 최종 revision은1753이다.

- GATE1: P1→P2→P6→P7→P47→P48→P58→P78→P79→P80→P81→P82→P83.
- GATE2: B1→B2→B3→B6→B7→P21→B10→B9→P27→P85→P86→P87→B4→P25.
- GATE3: P88→P91→P92→P93→P52→P46→P66→P76→P35.

기존 entry ID를 재사용하고 새 P76에는 고유 ID를 발급했다. 각 행 뒤1초,
마지막 행과 P76→P35는0초다. P29·P30의 정의와 다른 패턴 저작값은 보존했다.
마지막 Flow 이후 재시작 분기는 없고, 종료된 boss pattern은 기존 Brain의 Idle로 대기한다.
실제 보스 처치 후 클리어·전원 투표로 다음 관문을 진행하는 기존 계약을 유지했다.

P76이 미게시였던 원인은 단일5000ms stage에 animation2개가 있던 것이다. 원래 clip과
절대 재생 시각을 유지한4430ms/570ms stage로 분리했다. occurrence ID,570ms blend와
4개 카메라의 시간·위치 등 presentation 전체는 동일하다. 후보의 실제
validate_document/validate_publishable/projected_outputs를 통과했다. 근거는
`out/KoukuShowtimeFlow20260919/validation.json`이다.

### Server entry failed 조사와 수정

Client46764의03:15:15.604 KST terminal은 CLIENT_INVALID_SERVER_RESPONSE/WSA10055이며,
복구 source는 level-kakul-saydon.network-connection-lost다. 같은 시각 Server40796
session2는 SERVER_PEER_CLOSED이고 sendFailures0이었다. Lobby 문구는 공통 복구 표시다.
로그만으로 정확히 어떤 큐의65번째 패킷이 B2에서 실패했는지는 확정하지 않는다.

MainApp의 Kouku audition service include와 Update가 Debug guard 안에 있어 Release가
제품 레이드 owner에게 오는 result/lifecycle을 소비하지 않는 결함을 확인했다. 기존 Update를
NetworkManager dispatch 직후 공통 경로로 이동했다. 저작 UI guard, 실제 service의
request/revision/lifecycle 검증은 유지하고 네트워크 큐 용량을 늘리지 않았다.

### 수행한 검증

- 실제 service와 MainApp의 변경 전후 frame fragment를 사용하는 Release 콘솔 검사7/7 PASS.
  이전 fragment는64개 적체 후65번째 수신 불가, 수정 fragment는256개 소비를 확인했다.
  두 큐 소비, 요청 없는 제품 알림이 local Flow를 시작하지 않음, 기존 ACTIVE→COMPLETED도
  확인했다. 전송 boundary만 대역이며 Client/UI/Engine을 실행하지 않았다.
  증거: `out/KoukuReleaseLifecycle20260919/validation.receipt.json`, `run.log`.
- 정상 Release Product PASS,41.039초. MainApp OBJ1과 Client EXE1 갱신,
  PCH/CSO 재생성0. 기존 코드 페이지와 DirectXTK PDB 경고가 남았고 컴파일·링크 오류0.
  `out/BuildPipeline/runs/20260918T182442969Z-release-product.json`.
- 최초 빌드는 실행 중 Client/Server의 출력 점유 guard에서 중단됐다. 사용자가 둘 다
  종료했다고 알린 뒤 정상 빌드를 재실행했다. 실패했던 preflight를 빌드 PASS로 세지 않는다.
- 저장 Flow 구조·stable ID·관문 참조 검증과 P76 presentation 보존 확인 PASS.

공식 KoukuSaydon owner1753 및 Client owner 게시 PASS. 최종 통합 빌드와 ZIP 검증도 아래 증거로 완료했다.
실제4인 B2, 쇼타임 카메라·전투 연결 및 마지막 Idle·처치·투표 화면은 사용자 확인 대상이다.
P29의 내부 크래시 수정은 하지 않았고 요청대로 Flow에서 제외했다. IP는10.16.127.103을 유지했다.


## 컷씬 전체 게시·UI 억제·이펙트 상한과 종료 진단 (2026-09-19)

### 게시와 최신 흐름

P73/P74/P75/P77에 기존 저장 occurrence를 포함하는 durationMs 27000/35368/49083/11950만
추가했다. 기존 World·camera·sound·actor 입력은 그대로다. P75는 현재 저장된 카메라12개와
음향2개이며 배우 occurrence가 없어 새 배우 동작을 복원했다고 주장하지 않는다.
저장93개 중 게시85개/8bundle/461stage이고 조사한 컷씬 P36/P73/P74/P75/P76/P77은 모두 게시됐다.
source category는93개 모두 MECHANIC이므로 category enum으로 컷씬을 선별하지 않았다.
미게시 비컷씬8개(P3/P16/P20/P26/P32/P37/P45/P51)는 빈 단계 등을 강제 게시하지 않았다.

1753 source hash는90611844a94c821edc8c414977e2fcc8f894e6c64315e4d3d8103cc946e5525d다.
공식 KoukuSaydon owner의 product/map/world/gameplay와 Client owner의
composition.presentation/world/navigation을 게시·재사용했다. source, Encounter, bindings,
Server bootstrap revision과 Flow를 대조해 전체 목록을 갱신했다. 근거:
`out/KoukuPatternFlowReview20260919/publish1753.log`, `publish-client1753.log`,
`CURRENT_PATTERN_FLOW.md`.

각 Mario Parent의 MARIO_ENTER stage1~4는 기존 P33 후속을 시작하고 성공 시 P42로 이어진다.
solo는 실제 복귀를 기다리고 2~4인은 입장 직후 후속을 진행한다. Flow는 해당 체인의 완료를
기다리며 Mario 참가자 복귀까지 별도 동기화하지 않는다. 이번 요청에 독립 P33 행을 더해
같은 2페이즈가 중복 실행되는 구성은 만들지 않았다.

### 컷씬 UI와 배경

실제 cinematic pending 또는 소유 중인 시간트랙에 따라 제품 sprite·text·nameplate·
chat bubble·damage number·입력을 공통 억제한다. 기존 visible/open 상태와 연출 fade를
보존하며 종료·실패·Level 이탈 시 복원한다. 일반 combat follow/static 카메라는 제외한다.
숨은 damage event는 cursor를 진행해 종료 후 과거 숫자가 다시 나오지 않도록 했다.
실패 시 pending/카메라 owner가 남던 cleanup도 보정했다.

배경만 검고 배우·이펙트는 보인다는 사용자 증상은 원인 미확정이다. 조명·배경 visibility를
일괄 바꾸지 않고 `kouku.cinematic.transition`에 mapVisible/Total/Unknown, scene/gateScene,
light/camera owner, fade와 raid phase를 남긴다. 컷씬 실패는 `kouku.cinematic.failed`에 기록한다.
실제 UI router/header, sprite Render, cinematic predicate를 플랫폼 stub과 컴파일한 native
검사에서 hidden input, fade 유지, visibility 복원, held-click 재발생 방지, combat camera 제외가
통과했다. 증거 `out/KoukuCinematicUI20260919/probe.result.log`. 실제 화면 검증은 아니다.

### 이펙트 및 네트워크 원인 구분

B2 P10의 이펙트 occurrence는0개, P11은 V2 GROUP7개이며 blue3/red1의 Mesh/Decal8개와
laser 최대 Mesh30개다. leaf 모델·텍스처는 현재 PC에 모두 존재한다. P29는 V1 하나, 문서
16요소이며 conservative reservation particle141/mesh3/trail44/draw18이다. 이 데이터만으로
V1 scene128/particle16384 등의 상한 초과를 확정할 수 없고, B2에는 그 V1 budget이 직접
적용되지 않는다. 전체 상한/격리/종료 분기는 별도 audit와 pattern_effects.json에 기록했다.

로컬 .103/03:15:15의 Client10055와 Server orderly close는 동일 사건이며 Server sendFailures0,
maxSend956us였다. 원격 .181/03:07:43의10060 timeout/maxSend255115us/coalesced6은 별도
사건이다. reliable queue cap 도달 기록은 없고, 원격 수신 정체를 GPU 과부하로 단정할 수 없다.

NetworkManager의15개 결과/알림 큐는 기존64상한을 유지하고 overflow detail에 queue/depth/limit
및 실제 packet type을 기록한다. Kouku result/lifecycle decode failure도 별도 detail로 구분한다.
Release에 없던 Renderer 실패 파일 기록을 활성화하고 V1 admission/preparation, V2 group/asset/
mesh/texture/draw 경로와50ms 이상 prepare/spawn을 bounded EffectFailure 로그로 남긴다.
동일 실패는 반복 수를 남기며 로그로 렌더링·실패 정책을 바꾸지 않는다.

Server 기존 RoomPerf에 UnixMs, gate/flow/member pattern IDs, combat object live/replicated 수,
이전 loop lateness/reset을 추가하고 session과 같은 Diagnostics에4MiB×2 파일로 보존한다.
실제 Server CPP2개 Release 최소 컴파일 및 실제 writer native9/9 PASS.
`out/KoukuServerDiagnostics20260919/validation.receipt.json`. timeout/cap은 늘리지 않았다.

검은 배경과 특정 GPU 실패의 원인 수정 완료는 아직 아니다. 이번 배포에는 확인된 Release
lifecycle 결함 수정과 컷씬 UI 동작 수정, 다음4인 재현에서 구분 가능한 로그를 포함한다.


### 최종 통합 검증과 배포 완료

- 최종 Debug Product PASS, 29.540초. `out/BuildPipeline/runs/20260918T185333963Z-debug-product.json`.
- 최종 Release Product PASS, 115.380초. `out/BuildPipeline/runs/20260918T185529591Z-release-product.json`.
  Engine/Shared/Server/Client 정본 빌드·배포를 순차 완료했다. 기존 코드 페이지와 PDB 경고는
  유지됐고 컴파일·링크 오류는 없었다. 최종 Debug endpoint도10.16.127.103:7777이다.
- UI router는 컷씬에 걸쳐 누른 왼쪽 버튼의 물리 release까지 입력을 억제한다. 진입 때
  기존 Inventory/창/슬라이더/지도/quick-slot gesture를 취소하고 창 열림·위치를 보존한다.
  cursor lock의 포커스 해제 처리는 억제 밖에서 계속 실행한다. 실제 Inventory press/release
  block 검증은 옛 gesture의 swap/drop 방지와 새 drag의 정상 동작을 확인했다.
  `out/KoukuCinematicUI20260919/drag_probe.result.log`, `probe.result.log`.
- V1 object-local/global 렌더 실패도 Effect_Object.cpp의 기존 격리 분기에서 안전하게 파일에
  기록한다. logger 문자열 조립까지 예외를 격리해 기존 실패 반환을 보존했다. 상세 조사:
  `out/KoukuEffectCapAudit20260919/EFFECT_CAP_AND_FAILURE_AUDIT.md`.
- 변경 JSON/XML parse,1753 source/product/bindings/bootstrap·Flow 일치, stable ID·최종0ms,
  명시 컷씬6개의 게시 상태 및 git diff --check PASS. 실제 source category93개는 모두
  MECHANIC이므로 비어 있는 CUTSCENE filter의 통과를 게시 검증으로 사용하지 않았다.

최종 ZIP은 `C:/Users/user/Desktop/LostArk-Release-20260919-Flow-v3/LostArk-Release-20260919-Flow-v3-10.16.127.103.zip`이다.
99,158,817bytes, SHA256 `2217cd2b3ed299ee0e09d9c2f22271025ac9bf70ee535ecf39be9eb2231b56e8`.
Release runtime458개(Client DataFiles150, Server DataFiles159 포함), 직접 소비 Data JSON1190개,
EffectCatalog 참조1167개를 포함한다. action1753/sequence64/protocol93. Resources는 기존 팀
공유본을 쓰며 ZIP에 넣지 않았다. 전체 Flow목록·상한조사·한국어 진단안내를 함께 넣었다.

공식 runtime packager, 설치기 WhatIf 및 내부/외부 ZIP의 CRC·SHA256·현재 파일 일치 검사 PASS.
P76 camera4개 및 필수 Kouku 직접 소비 Data5종 포함을 확인했다. 설치기 검증은 현재 PC에서
실제 교체·backup·바로가기·Client 실행을 하지 않았다. `out/KoukuFlowDelivery20260919/`의
`delivery-result.json`, `final-package.log`, `final-install-check.txt`가 증거다.

사용자는 ZIP을 풀고 LostArk.exe에서 최신 Resources가 있는 기존 LostArk 폴더를 선택한다.
공유 Server도 최신 파일로 재시작해야 메모리에 적용된다. 실제4인 B2/쇼타임/Idle·투표 및
컷씬 배경·이펙트 GPU 표시는 사용자 확인으로 남는다. 검은 배경 원인·P29 내부 크래시가
수정됐다고 주장하지 않으며, P29는 요청대로 Flow에서 제외했다.


## G12. Release 4인 실측 수정과 UI #413 통합 (2026-09-19)

### 확인한 원인과 구현

- 현재4인 PID20876/43668/52564/54892, Server25564 로그8개를 stable SHA와 함께
  `out/KoukuSequenceRestore20260919/evidence/`에 보존했다. 이 실행의 terminal/recovery는0,
  send failure/reliable rejection/drop은0, world5 최대 tick은4ms다. 종료는 각 Client의
  close-requested와 Server orderly peer close가 일치한다. 이펙트 거부와 네트워크 종료를
  동일 원인으로 단정하지 않는다. 이전03:15 Client10055와 Release 미소비 큐 결함은 별건이다.
- 미로 P28은 실제 전원을 전송하고 area HUD MAZE를 설정하지만 망원경을 잡기 전에는
  runtime INACTIVE/role NONE이다. 기존 Flow 판정은 이 구간을 놓쳤다. 이번 로그에서
  미로 camera 진입04:29:31 뒤04:29:57에 P25가 시작되고04:29:58에야 WAIT_MINIGAME가
  됐다. 살아 있는 플레이어의 권위 area HUD MAZE도 대기에 포함해 정상복귀 state clear까지
  P25를 시작하지 않도록 수정했다. 이미 실행한 timer를 임의 pause하는 우회는 추가하지 않았다.
- Sequence P1/P4의 book/stage WORLD 각6개가 camera/scene보다2,852ms 일찍 종료됐다.
  P1 duration37,800→40,652, P4 duration38,851→41,703으로12필드만 고쳤다. Sequence65이며
  다른7개 Sequence에는 같은 gap이 없었다. 이 수정은 확인된 무대 말단 공백을 해결하며
  종료 뒤 장시간 검은 배경이 유지되는 모든 상황까지 실전 확인한 것은 아니다.
- 실제 거부는 gate2 intro의 보수 예약 light34>기존32, 창술사34630 동시·반복 사용,
  P48 카드24개×trail500이 기존12,288에 닿는 상황이다. 카드 정상최대는42개다.
  Kouku level 전용 hard/remote/owner budget을 적용하고 로드 준비는 최대지원치를 사용한다.
  무제한 생성은 하지 않으며 Bern 등 다른 Level의 기존 admission 수치는 유지한다.
- Bern 폭포 mist sprite의 local roll과 비균일 배치 scale1.275/.85/.85가 만드는 유효한
  shear를 XMMatrixDecompose가 거부해 전체 효과를 격리했다. billboard는 source quaternion을
  쓰지 않으므로 유한한 축 길이와 origin을 직접 사용한다. source NaN/Inf는 계속 거부한다.
- UI PR [#413](https://github.com/tnestyle70/LostArk/pull/413)를 브랜치에 fast-forward했다.
  MainApp 등의 기존 미커밋 수정은 보존했고 Kouku Render의 단일 충돌은 cinematic early return
  뒤 MVP/nameplate clipping을 유지해 합쳤다. 추가 G 키캡1772bytes를 이번 ZIP에 명시 포함한다.
  native 파일/도움말 메뉴는 창 생성에서 제거했고 AdjustWindowRect도 메뉴 없는 상태로 맞췄다.

### 예산과 회귀 근거

축 순서는 effects/particles/mesh/trail/afterimage/light/post/overlay/draw다.

| 범위 | 적용값 |
|---|---|
| Kouku hard |128 /49152 /4096 /32768 /2048 /160 /16 /64 /8192|
| Kouku remote |96 /40960 /4096 /24576 /1536 /144 /12 /24 /7168|
| Kouku owner |32 /8192 /2048 /4096 /1024 /32 /16 /32 /3072|

정상4인×34630 2회 잔상 중첩 + 카드42개 + intro34light + 관측환경18개의 보수 합은
69/35472/3228/21000/0/122/8/0/5998이다. 실제 admission 함수125회 검사에서 local/remote
모두 통과하고 각9축 초과, uint overflow, owner, pending 및 다른 Level 제한은 유지된다.
맵 복제조명을 포함한154 + 효과160 + 직접조명5 =319<Engine384, provider envelope220<256이다.
이 수치는 해당 재현 workload와 현재 authoring 기준이며 모든 미래 조합의 GPU 성능 보장은 아니다.
`out/KoukuEffectCapAudit20260919/REPRO_0431/scoped_budget_result.json`.

billboard는 실제 함수 수치230개 PASS: source와 같은 shear4건이 기존false→현재finite true,
기존TRS72건(음수scale/roll/pivot/cameraoffset 포함)은 행렬차0, NaN/Inf 거부.
`out/KoukuSequenceRestore20260919/billboard/run.log`. 화면/GPU 검증이 아니다.

### 3관문 실제 연결

P88(stage1)→P33→무력화성공P42→P91(stage2)→P33→성공P42→P92(stage3)→P33→성공P42→
P93(stage4)→P33→성공P42→P52→P46→P66→P76쇼타임연출→P35쇼타임→Idle다.
P33은24.326초/threshold1000의 무력화이며 실패·시간초과는 기존 전멸 조건이다.
각 P42 완료 뒤 다음 Flow를 시작한다. 4인은 Mario 입장 직후 남은 파티의 P33을 진행하고,
solo만 실제 Mario 복귀까지 먼저 기다리는 기존 권위를 유지한다.

### Release 진단과 빌드

Client의 `Release/Diagnostics/client-session-<pid>.jsonl`에 buildConfig, 최초 terminalReason/
terminalDetail/isTerminal, main-pump age/최대gap/1초이상stall count, coalesced count,
process private/working-set bytes와 사용 가능한 물리메모리를 추가했다. 60초 heartbeat와
최초·최대5초간격 stall event를 남기며8MiB rotation은 `.previous`를 보존한다.
실제 `Server entry failed.` 표시에서 `lobby.recovery.presented`를 기록하고 원래 recovery
source/HRESULT와 최초 terminal을 함께 보존한다. 실제 diagnostic class Release probe의
JSON/first terminal/회전/새 generation/메모리·tick·큐 보존8항목 PASS.
`out/KoukuSequenceRestore20260919/diagnostic_probe.receipt.json`.
Server session JSON에 PID/buildConfig/connection.closed, send실패에는 stack buffer로
partial/total frame byte를 추가했다. 기존 RoomPerf와 timestamp로 대조한다.

통합 Release Product126.316초 PASS, 최종 Server 진단 stack 보정 후 증분11.264초 PASS.
각 receipt `out/BuildPipeline/runs/20260918T195216445Z-release-product.json`,
`20260918T195254472Z-release-product.json`. Compile/link 오류0, 기존C4819/LNK4099 경고는 남았다.
최신 Debug 전체 빌드는 이 G12에서 실행하지 않았다. Endpoint는192.168.0.14:7777이며
Client fallback/Debug·Release VCX debugger/.user와 Server bind0.0.0.0을 확인했다.

추가 완료 검증: 실제 Release CGameRoom 회귀에서 수정 전 미로 조기 피자 시작6건을 재현했고,
수정 후27/27 PASS였다. 4인 Begin_CardMaze→return18/36tick 대기→37tick ownership clear→
실제 published P25 admission을 검증했다. Release solo의 최소2인 정책은 변경하지 않았다.
`out/KoukuSequenceRestore20260919/flow-trace-validation.json`.

공식 KoukuSaydon/Client owner 게시 모두 exit0 PASS, Server Gameplay.bootstrap의 RAIDGATE
3행 sequenceRevision65 일치. 첫 시도의 Encounter 일시 잠금 실패는 기존 파일 hash 보존을
확인한 뒤 공식 재시도로 해결했다. 실패 backup도 유지했다. 게시 parity receipt는
`out/KoukuSequenceTail20260919/final-publish.receipt.json`이다. Client/Server 화면 실행은
하지 않았으며 새 EXE·게시 Data의 적용은 사용자의 새 실행으로 확인한다.

최종 v4 ZIP 완료: `C:/Users/user/Desktop/LostArk-Release-20260919-Flow-v4/LostArk-Release-20260919-Flow-v4-192.168.0.14.zip`.
99175950bytes, SHA256 `51cbc99f7ad4849c7eccd209268fb38a8e71e8ea590284744c54936f416cf1d0`.
Release runtime458개, 직접 Data JSON1193개, UI G 키캡1장을 포함한다. 기존 Resources를
가진 LostArk 폴더를 선택하는 실행기를 사용하며 endpoint192.168.0.14:7777이다.
내외부 ZIP CRC/SHA, manifest 및 현재 파일 일치, 설치 WhatIf, 래퍼 재컴파일 PASS.
최종 Client/Server EXE SHA를 빌드영수증과 별도로 교차 대조했다. 로컬로그 수집 스크립트도
포함했고 프로그램 실행·종료·업로드 없이 사본 ZIP 생성만 검증했다.
`out/KoukuFlowDelivery20260919v4/delivery-result.json`, `package-build.log`.
실제4인 화면·GPU 이펙트와 재현되지 않은 접속 종료까지 해결됐다는 주장은 하지 않는다.

## G13. Bern 송신 정체와 전체 Effect admission 후속 수정 (2026-09-19)

이 절은 G12의 Level별 예산 유지와 Resources 일부 포함 배포 정책을 대체한다.
현재 실행 중인 v4와 이번 소스 수정·새 빌드의 적용 상태를 구분한다.

### 확인한 원인과 수정

Server PID53060의 Bern session4/6/8/10/12가 모두 약250ms 송신 정체 뒤 WSA10060으로
종료됐다. 마지막 session12는6,143 frame을 이미 보낸 연결이며 실패 당시 큐는
6,383-byte snapshot 한 개, reliable rejection과 snapshot drop은0이었다.
원본과 SHA는 `out/BernDisconnect20260919/server-0517-snapshot/`에 보존했다.
사용자가 제공한 Client28492 generation6의05:20:37 heartbeat는 이후 새 연결이며,
그 정상 수신을 앞선05:16:55 종료의 반증으로 사용하지 않는다.

ClientSession의 SO_SNDTIMEO250ms를 제거하고 nonblocking socket의 WSAEWOULDBLOCK은
100ms readiness 대기 뒤 동일 frame의 남은 위치부터 보낸다. 정체 시간만으로 종료하지
않으며 같은 socket의 recv도 readiness를 기다린다. FIN·실제 native 오류·명시 Stop은
구분한다. 큐128 frame/512KiB와 snapshot coalescing은 유지되며 reliable queue 포화까지
무조건 유지한다고 주장하지 않는다. 관측된5건은 그 포화 경로가 아니었다.

전체 Level·owner·local/remote Effect admission과 prewarm의 임의 개수 ceiling을 제거했다.
비용 telemetry, uint overflow와 유효한 문서·리소스 검사는 유지한다. Artist31930의
local-only sidecar에는 없는 요소9개, Lance34630 clip1에는6개가 있어 원격 재생 전체를
rollback했다. 해당 참조만 제거하고 효과 요소와 수명은 보존했다. 전체 catalog1167개를
대조했고 현재 연결된 local-only 참조206개가 모두 실제 요소를 가리킨다.

Engine presentation의 provider256/light384/post64/overlay64는 초기 reserve로만 사용한다.
Map·Kouku·Debug preview의 light truncation도 제거했다. Light_Manager는 shader의400개
배열 ABI를 유지하면서 유효한 조명을 여러 draw batch로 전부 전달한다. 임의 frame 개수
초과로 전역 Render가 실패하던 경로를 제거했으며 실제 GPU 오류는 그대로 실패로 남긴다.
기존 persistent scene light16개 계약과 문서 parser의 개별 유효성 범위는 별개다.

송신 정체·회복은 `Diagnostics/server-send-progress-<pid>.jsonl`에 peer/session/packet,
sent/total bytes, 정체 시간과 마지막 수신 경과를 기록한다.2MiB 회전과 반복 제한을 사용한다.
Renderer 실패에는 UTC, Client session의 `render.failed`에는 stage/HRESULT/deviceRemovedReason을
남긴다. `Collect-RuntimeDiagnostics.ps1`은 실행 경로·EXE/DLL hash와 최신 로그의 사본을
모으며 Resources를 읽거나 앱을 실행·종료·업로드하지 않는다.

공용 Effect의 외부 transform history, Artist history/clock, follow owner/model/bone/root
실패가 Release 파일 진단 없이 객체만 제거하던 사각도 보강했다. `V1.update.*`와
`V1.follow.*`에 asset/occurrence/handle/level/elapsed/sample/anchor/action tick/reason을
인스턴스당 한 번 남긴다. 정상 Level 퇴장·자연 종료·Stop은 제외하고 기존 재생 동작은
변경하지 않았다. 이 최종 Effect_PresentationService.cpp Release TU도 컴파일 PASS이며
`effect-runtime-diagnostics/RECEIPT.md`에 소스 hash와 컴파일 로그를 보존했다.

### 완료한 비시각 검증

- 실제 ClientSession과 SessionTransport TU Release 컴파일 및 native socket11/11 PASS.
  750ms 수신 중단 후16MiB byte 일치, FIFO, snapshot coalescing, 명시 취소와 reset 격리 확인.
  양수 partial-write offset은 플랫폼에서 강제 재현되지 않았고 관측 stall offset은0이다.
- 실제 Effect accounting 함수3,405 검사 PASS, 수정 당시 실제 Effect_PresentationService
  Release TU 컴파일 PASS. 원격·로컬,8개 Level, 정상 재사용 꼬리 중첩과 overflow를 확인했다.
- 실제 Presentation_Manager/Light_Manager CPU 구현50/50 PASS.600 provider,
  1,200 light,600 post,600 overlay,399/400/401/800/801 경계, receiver/shadow8조합,
  두 번째 batch 실패와 invalid provider rollback을 확인했다. Shader/VIBuffer는 mock이다.
- 진단 수집 ZIP17파일/오류0, payload hash/CRC 확인. 실행기 path probe10/10 PASS는
  실제 runtime path 함수를 사용했고 UI/Client는 실행하지 않았다.

세부 근거는 `out/BernDisconnect20260919/SERVER_RECEIPT.md`, `ENGINE_RENDER_RECEIPT.md`,
`effects/EFFECT_RESULT_CANDIDATE.md`와 각 source receipt에 있다.

### 보스 전체 표시 누락과 쇼타임의 별도 원인

실제 Product 전체 staging을 실행해 수정 전 `Invalid presentation string: worldSequenceInstanceId`
실패를 재현했다. 최초 행은 P73.presentation.1 CAMERA이며 P73~77의 CAMERA/SOUND57행이
고정 WORLD 좌표를 사용해 worldId와 worldSequenceInstanceId가 비어 있다. publisher와
재생기는 이를 정식으로 지원하지만 Reload_Product는 모든 WORLD 행에 World Object의
sequence ID를 강제했다. 한 행의 예외가85개 패턴 전체 commit을 막아 일반 이펙트가 빠졌다.
Server collider·damage와 독립된 카드/targeted 표시가 남는 사용자 증상을 설명하는 코드 결함이다.

WORLD이면서 worldId가 있는 행만 sequence identity를 연결하도록 수정했다. WORLD
EFFECT/LIGHT/COLLIDER는 앞선 Read_Occurrence의 필수 worldId 검사, 모든 named World는
필수 sequence 검사를 그대로 통과해야 한다. 빈 ID를 임의 sequence로 대체하지 않는다.
전체 staging 후보는 published camera114개와 실제 blendOutMs 적용 및 camera 비활성 두 경우
모두85patterns/8bundles/2fear/14targeted/407animations/5blendwindows를 통과했고 isolated0이었다.
WORLD Effect의 빈worldId, named WORLD Effect/Camera의 빈sequence는 계속 거부했다.
최종 실제 수정 소스에서 재추출한 전체 staging도 같은 결과였다.
`out/BernDisconnect20260919/kouku/full/RECEIPT.md`에 before/after와 최종 SHA를 보존했다.

05:47 실시간 로그의 showtime.presentation.1은 별도로 'Kouku source attachment has no
animation at its requested time' 실패가 반복됐다. 독립 source-boss pattern에는 animation
목록이 없는데 원본 muzzle에는 명시 SourceModelPreview가 있었다. 기존 sampler가 원본을
사용하지 않아 본 부착 실패로 총구와 함께 묶인 장판까지 중단했다. 빈 pattern animation과
명시 SourceModelPreview 조합은 기존 Sample_SourceAnchorWorlds를 Effect-local clock으로
사용하도록 수정했다. 일반 pattern의 authored animation lane은 계속 우선한다.
같은 muzzle를 사용하는 targeted definition3개가 해당되며, 일반612 V1 행 중 같은 조합으로
실패하는 bone attachment 행은0이었다. 임의 현재 pose나 identity matrix로 실패를 숨기지 않는다.

최종 실제 Make_SourceAnchorSampler의 before/after native17검사 PASS다. 독립 evaluator는
test double로 분기·effect-local clock·fit rate·cycle·기존 pattern 우선·invalid 입력을 확인했다.
별도로 실제 설치 MN_RPCT_05.wmodel의 rpct00_att_battle_28_05_loop_a(3.166667초)와
bip002-r-hand(bone140)를 확인했고 source1.097~3.166667초6개 pose가 유한했다.
최종 KoukuPlayer Release TU 컴파일도 PASS다. 근거는 `kouku/source_sampler_probe.log`,
`kouku/installed_muzzle_model_probe.json`, `kouku/compile.log`이며 GPU 표시 판정은 아니다.

Product 로드 성공은 기존 Client session JSONL의 `kouku.product.loaded`에 revision과
pattern count, 실패는 EffectFailure.user.log의 `Kouku.product.load`에 원인을 기록한다.
실시간 원본은 `out/DiagnosticBundles/20260919-054835-8b545350.zip`에 보존했다.

### 적용과 화면 경계

최종 Product Release 빌드는104.830초 PASS다. receipt는
`out/BuildPipeline/runs/20260918T205656146Z-release-product.json`이며 compile/link 오류0,
기존 C4819/C4244/LNK4099 경고는 남았다. Debug 전체 빌드는 이 후속 변경에서 실행하지 않았다.
최종 Client SHA256은 `57b2448498ab39c21fb192360d485ab8ce717058f5a0c94bc6708fec1f50ff0a`,
Server는 `42b114fe1f174fbc5b16a3ff0a1a82cfacd71d45bc2304515bf5315ddb15f5ee`다.
v5 ZIP은 `C:/Users/user/Desktop/LostArk/Release/LostArk-Release-20260919-v5-192.168.0.14.zip`이다.
101,394,117bytes, SHA256 `b6e956b7c106843e34313efcb9ee61780133b68f7439c44c332f9e5b24d04f36`.
runtime458개와 직접 소비 Data1727개(1721JSON/6animevents), 실행 도구4개를 manifest에
기록했다. V2 전체324개와 UI·Rendering·Animation 등의 직접 소비 경로를 포함하고,
Resources/PNG/ChangedData는0개다. 선택한 기존 LostArk에서는 Resources만 읽고 새 ZIP의
EXE/DLL/CSO/Data/DataFiles를 사용한다. 이전 v4의 changed-data 목록은 독립 실행본의 전체
필요 데이터 목록이 아니므로 재사용하지 않았다. `portable-data-closure.receipt.json`과
`root-zip-verification.json`에 참조·최종 binary SHA·ZIP 검증을 보존했다.
실제4인 GPU 표시와 종료 재현의 성공 판정은 사용자 실행 후 기록한다.

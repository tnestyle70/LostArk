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

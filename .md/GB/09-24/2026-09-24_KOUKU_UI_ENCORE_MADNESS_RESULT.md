# 쿠크 앵콜 진입·광기 UI 결과

## G01. 실제 반영

G3 처치 후 남아 있던 `ENTER_BINGO` 버튼을 제거했다. G3 완료이며 빙고 관문이 있는
상태에서는 관문 버튼과 투표 창을 숨기고, 기존 Server의 false-clear 5초 → 앵콜
Sequence → 빙고 전투 자동 전이를 소비한다. G3의 MVP 생략도 raid-state 패킷 도착
순서 대신 gate-progress의 currentGate/gateCount로 결정한다.

G3 진입 오라의 카운트는 화면 X=50%, Y=75.5%로 이동하고 기존 1.4배 글꼴을
0.7배로 줄였다. 안내 문구는 Y=72%, 기존 1.25배에서0.625배다.

F1 → Kouku UI Preview → Madness gauge position에서 화면 X/Y offset과 world height를
바꾸면 자신과 동료의 광기 게이지가 즉시 같은 설정을 소비한다. X/Y는1280×720 기준
픽셀이며 +Y가 아래다. `Save position`은 기존 `Data/UI/KoukuSaydon/KoukuHudModes.json`의
`madness.screenOffsetX/screenOffsetY/headOffsetMeters` 중 실제 편집한 필드만 병합한다.
신규 X/Y가 없던 문서는0을 사용하여 기존 위치를 유지한다. `Reload saved position`은
현재 저장본을 다시 읽고 실패하면 기존 preview를 보존한다.

Save는 최신 source를 읽어 같은 dirty 필드의 외부 변경만 거절하고, 독립적인 외부 축과
다른 HUD mode 편집은 보존한다. unique 임시 파일, sidecar 배타 lock, 교체 직전 source
비교, `ReplaceFileW`의 displaced backup 검증과 자기 변경 rollback을 사용한다. 성공한
최신 병합 위치는 자신과 동료 view에도 전파한다. 이번 작업에서 UI authoring JSON을
직접 교체하거나 사용자 Client를 Reload하지 않았다.

동료 게이지가 HP/madness 숫자만 가진 임시 state를 만들면서 카드 미로 mode/role을
버리던 원인을 수정했다. 모든 world-space 게이지가 같은 미로 표시 정책을 적용하며
실제 MAZE snapshot에서는 F1 수치 preview를 켜도 게이지를 숨긴다.

## G02. 검증 증거

실제 `DataJson` 구현과 `CKoukuMadnessGaugeView::Get_Position`, `Set_Position`,
`Save_Position`의 현재 함수 본문을 임시 콘솔 fixture에 그대로 넣어 실행했다.
그래픽 객체 생성 대신 위치 상태와 임시 source path만 제공했다. 결과는25개 assertion
PASS, failures0이며 다음을 확인했다.

- NaN/범위 밖 preview 거절과 정상 위치 입력.
- X만 편집한 동안 외부 Y/head 변경을 보존하는 병합과 unrelated mode 값 보존.
- 저장 JSON 재파싱, live 위치의 최신 병합값 적용, 실제 displaced backup byte 확인.
- 같은 필드 충돌 시 disk와 draft 보존, no-op Save의 무교체 refresh, 후속 저장.
- 손상된 source JSON의 거절과 원본 보존.

증거는 `out/KoukuUi20260924/madness-position-io-test.log`와 같은 폴더의 임시 C++/EXE다.
제품 Client 실행이나 화면 판정이 아니다. 소유 C++ 파일의 `git diff --check` PASS,
기존 UTF-8/BOM/CRLF 보존을 확인했다. 신규 C++/Data 파일은 없으므로 project/filter
추가 등록은 필요 없다. 통합 Debug/Release Product 빌드는 통합 RESULT를 따른다.

## G03. KillBoss 후 Sequence 조사와 남은 확인

Client KillBoss 패널은 typed command만 제출한다. `Apply_ServerGate`는 active raid에서
직접 gate presentation을 적용하지 않으며, `UpdateKoukuGateCompletePlay`는 Server의
CINEMATIC phase에서만 pinned Sequence를 재생한다. raid owner가 없는 kill 뒤 legacy
관문 진입은 Server 쪽 전이 수정 대상이며 통합 담당이 구현한다.

사용자는 새 빌드에서 G3 클리어→앵콜→빙고의 실제 연출, 오라 카운트 크기/위치,
F1 위치 조절/저장/재진입, 카드 미로의 자신·동료 게이지 숨김을 확인해야 한다.
에이전트는 Client/UI를 실행·조작하거나 화면 캡처하지 않았다.

## G04. standalone 관문 전이 테스트 추가

통합 담당의 Server 수정에 맞춰 기존 `Run_KoukuRaidIntegration`에2/3/4인×5전이×
성공/FAILED의30개 fixture를 추가했다. G1 clear ADVANCE→G2 intro, G2 clear→G3
intro와 G1/G2/G3 RESTART→동일 관문 intro를 검증한다. 실제 이전 boss placement와
death-notify를 사용하며 partial vote/READY 동안 player 위치·HP·사망 상태,
boss ID·HP·위치, next entity ID와 gate clear mask 보존을 확인한다.

최종 READY는 fixed tick 전에는 world를 바꾸지 않고, 성공 tick에는 destination의
게시된 intro ID로 CINEMATIC에 진입하며 legacy combat 위치로 곧장 teleport하지
않아야 한다. FAILED는 ABORTED 사유와 기존 world 보존 및 후속 tick 무변경을
검증한다. 기존 Bingo vote/Parent 반복 fixture는 유지하고 flag 이름만
`bGateVoteEntry`로 맞췄다.

G3 RESTART는 기존 Mario 진행 cursor를3으로 시작하여 준비·실패 동안3이 보존되고,
모든 READY 이후 CINEMATIC commit에서만1로 초기화되는 회귀 단언도 포함한다.

현재 이 추가 테스트의 diff 검사와 최종 v37 기준 Debug/Release Server·Client
ClCompile은 모두 PASS다. `Server.exe --kouku-raid-contract-test` 실행은 사용자 저장·종료
확인과 최종 Product 링크 뒤에 수행한다. 30개 fixture의 실행 PASS를 주장하지 않는다.

## G05. 후속 Save 별표 제거와 저장 상태 안내

사용자의 후속 요청으로 `WorldObjectTool.cpp`의 공통 저장 버튼과 목록 상단 버튼 두 곳을
항상 `Save`로 표시하도록 변경했다. dirty 상태 문구와 Save/자동 Publish 동작은 그대로다.
현재 파일의 다른 Object hierarchy 작업은 수정하지 않고 두 label 표현식만 교체했다.

표준 MSBuild의 ClCompile/SelectedFiles로 실제 WorldObjectTool.cpp를 Debug/Release에서
각각 컴파일하여 exit0을 확인했다. 로그는
`out/KoukuWorldObjectSaveLabel20260924-{debug,release}-compile.log`다.
diff 검사는 PASS이며 최종 EXE 링크와 사용자 화면 확인은 아직 수행하지 않았다.

첨부의 `Invalid or duplicate world object resource: world.object.resource.1`은
WorldSequenceDocument의 Validate에서 Save를 거절한 상태다. 저장 revision2214의466개
Object에는 중복 ID가 없고 해당 ID도 없다. 따라서 화면의 미저장 draft와 저장본을 구분한다.
공통 오류 문구는 ID뿐 아니라 모델 미지정·anchor·scale 등 여러 조건을 포함하므로
해당 메모리 draft를 보지 않고 정확한 위반 필드를 단정하지 않았다.

이 World Object Save는 성공하면 WorldSequences scope를 자동 게시한다. 일반 MapTool의
맵 배치 Save는 별도로 Area scope Publish가 필요하며 C++ Build가 이를 대신하지 않는다.
이번 별표 제거는 첨부 저장 오류를 해결한 변경이 아니다. 미저장 편집이 있으므로 최종
링크를 위한 사용자 종료보다 저장 오류 해결이 먼저임을 안내했다.

## G06. 시작 발판 광기 게이지와 1.3m 저장 확인 (2026-09-27)

### 실제 소스 반영

`Level_KakulSaydonArena::Update`가 각 플레이어의 Server snapshot 위치를 기존 Shared
`Is_KoukuArenaStartArea`에 전달하여 시작 발판이면 광기 게이지를 숨긴다. snapshot 수신
전에도 숨기며 관문으로 이동한 뒤 기존 게이지 표시를 적용한다. 본인과 동료 각각 판정하여
시작점에 남은 동료와 이미 이동한 플레이어를 구분한다. 미로·춤의 기존 숨김은 유지한다.

초기 접근 jump.2는 발판 영역 밖이므로 발판 검사만으로 최초 진입을 판정하지 않는다.
Server currentGate가 0이거나 수신 전이면 계속 숨기고, G1 인트로 종료 뒤 전투 관문이
활성화되면 표시한다. 보스 관문 없는 MARIO snapshot은 표시를 허용한다. F1 Mario2~4는
기존 mode=NONE 경로이므로 Server 승인된 player-only gate의 본인 표시도 보존한다.
Return to Start 뒤 currentGate가 남으므로 플레이어별 시작 발판 검사도 함께 적용한다.
Server gameplay와 데이터는 변경하지 않았다. 새 C++ 파일이나 project/filter 등록은 없다.

### 저장값과 자동 검증

사용자 저장 JSON의 `madness.feetOffsetMeters`는 `1.2999999523162842`다. 이는
`1.3f`의 정확한 JSON 직렬화이며 발 Transform origin의 월드 Y에 더하는 1.3m다.
화면 X/Y offset은 생략되어 0이다. 사용자 Save backup과 leaf 값 비교에서 높이 0→1.3m
외의 HUD 값은 보존되어 있었다. source JSON은 수정하지 않았다.

현재 `CDataJson`, `CProjectDataRoot`와 `CKoukuMadnessGaugeView`의 Load_Config,
Get/Set/Save/Reload_Position 본문을 사용하는 격리 console에서 사용자 JSON의 복사본에만
Save를 수행했다. 저장·새 인스턴스 로드 15개와 별도 새 프로세스 재로드 2개 assertion이
PASS했다. 실제 ProjectDataRoot도 repository Data를 resolve했다. 저장된 위치는 이후
생성되는 view에서도 동일하게 읽으며, GPU 투영 결과 자체를 실행 검증한 것은 아니다.
증거: `out/KoukuGaugeSavedPosition20260927/run.log`.

현재 `Level_KakulSaydonArena.cpp`와 `KoukuMadnessGaugeView.cpp`를 MSVC 14.44.35207의
Debug 옵션으로 out에 격리 컴파일하여 exit 0을 확인했다. 컴파일 전후 소스 hash 동일,
기존 C++ UTF-8/BOM/CRLF 보존과 JSON parse를 확인했다.
증거: `out/KoukuMadnessVisibility20260927/debug-compile.log`, `validation.json`.
Save 정본의 전후 SHA256은
`27e33cf895c86fcf91e5bde1ed44c102034c09a2e5dc702efeb8bf8ab12eb1fa`로 같다.

### 남은 실행 확인

사용자가 Debug Client PID 14032와 Server PID 66672로 검증 중이어서 표준 Product
링크·EXE 교체는 수행하지 않았다. 이번 compile은 설치된 EXE 갱신을 뜻하지 않는다.
다음 Debug Product Build와 재실행 후 시작 발판 숨김→G1 도착 표시, 시작점 복귀 숨김,
1.3m 실제 화면 위치를 사용자가 확인해야 한다. Client/Server/UI를 실행·조작·종료하지
않았다. 현재 사용자 실행본에 표시 수정이 반영되었다고 주장하지 않는다.

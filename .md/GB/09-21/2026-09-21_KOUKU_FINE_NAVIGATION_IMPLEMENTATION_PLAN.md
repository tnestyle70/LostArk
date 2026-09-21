# 쿠크 5개 공간의 0.5m 네비게이션 재베이크 구현 계획

## G00. 현재 상태와 사용자 범위

사용자는 1·2·3관문 전투장, 최초 시작 지점, 3관문 입장 전 공간을 다시 굽도록 요청했다.
마리오, 빙고, 카드 미로는 제외한다. 기본 격자는 524×800, 4m이며 기존 세부 격자는
Mario2/Mario3/Mario4/CardMiro 네 개다. 기존 기본 source/paint와 네 세부 격자는 보존한다.

현재 브랜치는 `codex/spider-pattern-fear-sound`이며 다른 작업의 C++·데이터·문서 변경이 있다.
현재 요청의 변경만 기록하고 자동 stage/commit 또는 무관한 변경 정리는 하지 않는다.
입력과 게시 파일의 시작 hash·원본은 `out/KoukuFineNavigation20260921/baseline.json`과
`before/`에 보존한다. 계획 시작 당시 Client와 Server는 사용자가 실행 중이었다.
최종 빌드 전 사용자가 저장 후 직접 종료했으며 자동 종료·UI 조작은 하지 않는다.

## G01. 실제 모델로 세부 격자 생성

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements`의 stable ID와
저장 transform, imported catalog의 Resources 상대 모델 경로, 현재 설치된 WModel 삼각형을 사용한다.
기존 `CNavGridBaker::Build/Save_Source`를 CPU 실행용 임시 호출부에서 사용한다.
제품과 다른 네비 런타임이나 영구 하네스 프로젝트는 만들지 않는다.

추가 격자 ID는 `StartFine`, `Gate1Fine`, `Gate2Fine`, `Gate3Fine`이며 모두 0.5m다.
사용자가 확인한 전투 범위는 1·3관문의 중앙 원과 2관문의 위쪽 포커판이다.
Gate2Fine은 상판 floor15의 실제 footprint에 여유를 둔 X[-4.5,20.5], Z[307,332.5]만 덮는다.
상판 구멍 아래의 실제 하부 바닥은 Big Saydon의 기존 생성 계약을 위해 높이를 보존한다.
F1 입장점에서 시작한 보행 연결 성분이 상판에만 머무르는 것을 검증하며 하부를 상판으로 메우지 않는다.
3관문 전투장과 고지대 입장 발판은 실제 삼각형의 보행 footprint가 겹치지 않지만 AABB가 겹친다.
따라서 두 공간을 Gate3Fine 한 격자에 담고 각각 실제 높이를 유지한다.
2관문의 위쪽 무대가 없는 곳을 임의의 HEIGHT 페인트로 메우지 않는다.
최초 시작 구역은 spawn과 jump.1~3의 세 발판을 X[-8,11], Z[-22,-6]에 담는다.
jump.3의 기존 명시적 이동은 Z=-24.779의 기본 격자에 착지하므로 이후 책·종이 통로의
기존 네비를 보존한다. 연속 보행 통로를 detail 경계로 잘라내지 않는다.
기존 동적 종이 다리와 런타임 support surface는 정적 바닥으로 만들어 덮지 않는다.

후보는 `out/KoukuFineNavigation20260921/candidate/Data/Navigation`에서 생성한다.
`.navregions`에는 기존 네 행을 보존하고 새 네 행을 추가한다. `.navsource`에는 실제 baker 결과를,
필요한 `.navpaint`에는 실측으로 확인된 보행/높이 보정만 기록한다.
새 저작 파일은 Client 프로젝트의 `None`과 기존 `96.DataFiles` 필터에만 등록한다.
새 C++ 제품 파일과 프로젝트 필터 재배치는 없다.

## G02. 실제 Server 소비와 이동 표시

`Tools/NavigationPipeline/Publish-ServerNavigation.ps1`의 같은 소스를 격리 후보 루트에서
실행해 Validate/Publish한다. base+detail footprint 비중첩, spawn/boss 높이와 이동 가능,
모든 파일의 파싱과 기존 영역 보존을 확인한다. 현재 `CServerNavigation`을 직접 사용하는
CPU 검사로 점프·입장 지점, 관문 내 경로, 바닥 밖 거부, 동적 support 전달을 확인한다.

Client의 `CNavigation`은 기본 격자만 로드한다. 새 서버 세부 격자와 4m Client 예측의
불일치를 막기 위해 `GameRoom_Replication.cpp::Broadcast_WorldSnapshot`에서 쿠크 월드의
세부 격자 안에서는 기존 `canPredictMove=false` 계약을 사용한다. 입력과 이동 명령은 유지하고
서버 snapshot 보간으로 표시한다. `Is_InSameDetailRegion`을 재사용하며 프로토콜·Engine API는
변경하지 않는다. 다른 월드의 이동 예측 정책은 바꾸지 않는다.

## G03. 설치와 종료 검증

후보 검증 후 최신 디스크 입력 hash를 재확인한다. navregions는 기존 stable region ID를
보존해 병합하고 새 파일과 함께 백업·원자 교체한다. 동시 저장으로 입력이 달라지면 재조사하고
확인되지 않은 새 저장본을 덮지 않는다. 공식 publisher로 Client/Server 실행 데이터를 게시한다.
실패하면 이 작업이 설치한 변경만 원복하고 다른 세션 변경은 유지한다.

JSON/XML parse와 `git diff --check`, 기존 제외 영역과 base의 byte 보존,
Client/Server 출력 일치, 해당 C++ 최소 컴파일 및 정상 증분 Product Build를 확인한다.
실행 파일이 점유돼 링크할 수 없으면 준비된 변경과 정확한 종료 필요 대상을 안내한다.
새 네비의 Server 로드는 Server 재시작, Client 바닥/디버그 표시는 레벨 재진입으로 확인한다.
디스크 게시, 실행 중 메모리 반영, 사용자의 최종 화면 판정을 구분해 RESULT에 기록한다.

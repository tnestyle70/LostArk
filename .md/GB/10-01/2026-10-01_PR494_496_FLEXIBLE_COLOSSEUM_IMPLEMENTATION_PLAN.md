# PR494~496 통합과 콜로세움 모집 시간

## G00. 통합 기준과 변경 범위

2026-10-01 사용자 요청에 따라 #494의 장비별 내구도·수리 UI, #495의 콜로세움 경기·결과 UI,
#496의 이동·Guide·용병·워터팡·재질 변경과 기존 쿠크 인형 Collider·칼날·카드 미로 수정을 통합한다.
원본 작업 폴더의 미커밋 변경은 보존하고 별도 통합 worktree를 사용한다.

기준은 #496의 `7013e90ad8c0f79284f794e933a396c4383fe052`, #494의
`1b858eafe17678e6e997285a5ae9de5e643c7b6c`, #495의
`115e3d3d353608ab996fb96eb750deb8458cc8f0`다.

## G01. Server와 Shared 통합

장비 내구도는 inventory item이 전달한다. 기존 sourceWorld admission과 원자적 경기 이동은 유지한다.
독립 PR의 패킷 번호 충돌을 해소하고 통합 protocol을132로 올려 이전 바이너리 혼용을 거절한다.
모집 시작 시 Server가10초 deadline을 정하고 대기 인원과 시간을 복제한다.4명이 먼저 모여도
동일 deadline을 유지하며 마감 때 현재1~4명을 입장 순서에 따라 양 팀에 교대로 배정한다.
취소·연결 종료·초기 송신 준비 실패는 기존 참가자와 방을 보존하거나 해당 큐 항목만 정리한다.

## G02. Client 제품 UI

`RaidEntryPreviewView`의 기존 QueueWait 화면 아래에 남은 시간을, 오른쪽에 현재 인원 N/4를 표시한다.
`Data/UI/Colosseum/QueueDialog_Layout.json`의 stable marker rect를 사용한다.
시간은 Server snapshot과 수신 후 경과 시간으로 표시하고 Client가 팀이나 경기 시작을 결정하지 않는다.
공용 `RaidGateProgressView::Render_AssemblyCountdown`에 문구를 전달할 수 있게 확장하여
기존 발탄·쿠크의 흰색 안내와 노란색 초 표시를 재사용한다.
#495의 실제 참가자 외형·도열·HUD·결과·승리 컷신과 기존 용병 state 소비자를 함께 보존한다.
기존 파일을 확장하므로 신규 C++ project/filter 등록은 필요하지 않다.

## G03. 쿠크와 배포 검증

쿠크 RESULT의 통합 지도와 최신 디스크 diff를 대조하고 관련 hunk와 게시본만 반영한다.
각 최종 JSON/XML parse, `git diff --check`, Debug/Release Product Build,
NetworkProtocolHarness와 콜로세움·내구도·쿠크 관련 Server 계약 검사를 실행한다.
실행한 결과만 RESULT에 남기고 Client 화면·다인 플레이 검증은 사용자 확인으로 구분한다.

최종 Release Product receipt와 같은 소스·Data/DataFiles로 기존 portable builder를 실행한다.
Resources는 외부 팀 리소스를 읽으며 ZIP에는 넣지 않는다. ZIP CRC·manifest SHA256·launcher
`--check`로 실제 실행 경로를 확인하고 원본 작업 폴더의 편집 draft를 자동 Reload하지 않는다.

## G04. 실제 테스트에서 추가된 우선 회귀

카드 미로는 Release에서 혼자 검증할 때 Q 타격이 콜라이더를 통과해도
`CKoukuCardMazeRuntime::Plan`의 Debug 전용 1인 분기 때문에 시작을 거부했다.
방에 인간이 정확히 한 명인 경우의 기존 관찰자·사냥꾼 겸임을 Debug/Release 공통으로 연결한다.
다인 방에서 사망자 때문에 한 명만 살아 있는 경우의 규칙은 유지한다.
실제 Q command decode와 handler, fixed tick, box 파괴, 후속 Q, snapshot 역할·시점 bit·문양,
랜덤 탈출 경로를 1/2/4인으로 검사한다. 편집기 Sequence 개수 3/5 기대값은 이 시작 거부와 별개다.

이동은 동기 GPU pixel readback의 `CopySubresourceRegion -> Map(flags=0)`이
입력 프레임의 주 스레드를 막는 경로를 제거한다. 기존 CPicking에 요청 ID와
`DO_NOT_WAIT` 완료 확인을 추가하여 요청 당시 pixel을 그대로 보관한다.
입력 해제 뒤에도 유효한 요청은 한 번 처리하되 새 클릭, 후속 명령, UI/capture,
캐릭터 교체와 만료는 이전 요청을 취소한다. 제품 Client는 실행하지 않고 생산 함수와
headless D3D11 WARP를 사용해 미완료 GPU copy, 정확한 pixel, 취소·stale reply를 검사한다.
기존 Engine/Client 파일 확장이므로 제품 project/filter 추가는 없다.
`Tools/MovementRegression`의 검증 프로그램은 도구가 생성·직접 컴파일하며 제품에 등록하지 않는다.

사운드는 연속 WORLD 시각 갱신에 남은 250ms 전진 차이의 강제 seek를 없앤다.
명시 scrub과 역방향 이동은 유지하고, 기존/수정 후 생산 함수와 실제 FMOD no-output mixer에서
첫 WAV의 Play/Stop 수명과 재생 cursor를 비교한다. 실청 완료로 대신 기록하지 않는다.
사용자가 보류한 Release 서버 상태 문구는 이번 추가 수정 대상에서 제외한다.

## G05. 워터팡 첫 등장과 가이드 후속 요청

워터팡 AI 첫 등장 때 주 스레드에 모이는 직업 bundle·외형·NPC 모델·물총 준비를
Maharaka Loader의 기존 준비 단계로 이동한다. Server와 Client는 같은 NPC archetype 목록을 소비한다.
선택 class만 미리 준비하던 경로와 실제 첫 spawn에서 읽는 경로를 대조하고,
준비 후 첫 spawn에서 이미 준비된 asset을 사용하는지 검사한다. 약 6초라는 사용자 관찰과
실제 cold load 호출 근거를 구분하며 존재하지 않는 프레임 시간 측정을 완료로 적지 않는다.

NPC 외형은 실제 설치 WModel의 오른손 본과 preScale/basis를 측정하고 기존 CPart_Equipment로
물총을 붙인다. 모든 AI 외형의 부착 준비·무장 상태·visibility와 실패 시 정리를 검사한다.
새 C++ 파일 없이 기존 로더·복제·NPC/장비 경로를 확장한다.
이름은 Server가 슬롯별 `Waterpang AI 1`부터 부여하고 기존 최대 20명 계약은 유지한다.
원문 `1~99`는 표시 번호 형식으로 해석하며 99명 스폰으로 확대하지 않는다.

스퀘어홀로 항구에 이동한 뒤 Guide가 따르지 않는 문제는 베른 내부 이동과 실제 출항의
상태 전환 및 경로 추적을 확인한다. 사용자가 직접 조정하겠다고 한 콜라이더·Guide ID·문구는
변경하지 않는다. 기존 브랜치와 원본 미커밋 변경은 유지하며 통합 브랜치만 병합한다.


## G05. 워터팡 대기 중 최초 준비와 NPC 물총

`CLoader::Ready_For_Maharaka`에서 Server AI 정본의 Guard/Lance 직업, 모코코 12종 의상,
NPC 8종과 물총 모델·재질을 기존 prototype 서비스로 준비한다. `EquipmentPresentationService`의
같은 model admission을 preload와 snapshot 적용이 공유하며 이미 준비한 prototype은 다시 읽지 않는다.
로딩 취소는 각 모델 준비 사이에 확인하고 실패는 기존 Level rollback 범위를 따른다.

`CMaharakaWaterpangPresentation`이 기존 물총 prototype 준비 함수를 공용화한다. NPC 표시체는
실제 설치 8종 WModel에 모두 존재하는 `bip001-r-hand`에 기존 `CPart_Equipment`를 붙인다.
NPC에는 prop3가 없으므로 Guardian의 실제 watergun_idle에서 측정한 prop3→right-hand 상대
TRS를 프로젝트 부착값으로 사용한다. 각 NPC의 최종 손 basis .01을 확인하고 같은 cm 단위 물총을
소비한다. snapshot의 armed/visible 상태와 NPC의 갱신된 pose가 표시를 결정하며 새 combat authority는 없다.

기존 C++ 파일만 확장하며 신규 project/filter 등록은 없다. 최소 검증은 8개 실물 rig·basis·총 크기,
roster와 preload 대상 일치, 생산 호출순서와 취소/visibility 연결, 최종 Product compile이다.
실제 화면과 프레임 체감은 사용자 확인으로 남긴다.


## G06. Visual Studio 작업 폴더 protocol132 동기화

사용자가 VS에서 실행하는 Desktop/LostArk의 현재 변경을 안전 백업과 이름 있는 stash로 보존하고,
기존 branch를 병합 완료 main b83d646be로 fast-forward한다. local Guide·Profiler 작업과 기존
Kouku 변경을 복원하며 source와 published bootstrap은 각각 실제 정본에 따라 충돌을 해결한다.
동일 VS 설치의 정상 Product Build를 Release와 Debug에 순차 실행해 Shared/Engine/Server/Client를
모두 갱신한다. protocol 숫자와 packet 소비 계약, 실제 VS target 및 새 산출물을 함께 확인하며
제품 Client/UI는 실행하지 않는다. 원본 safety stash와 out backup은 삭제하지 않는다.

## G07. 인간 준비 수와 용병 선택 계약 교정

2026-10-01 추가 사용자 확인은 전투 4대4, 인간 1~4명의 입장 순서에 따른 교대 배정,
양팀 각각 다섯 직업의 선택 후보, 직접 고용한 용병만 파티·전투 참가라는 계약이다.
3인 입장은 왼쪽 인간2명·오른쪽 인간1명이므로 각각 용병2명·3명을 고용한다.
기존 전투 인원 제한과 교대 배정은 이미 이 계약이며, 인간 없는 팀의 용병4명을
자동 참가자로 올리는 분기만 제거한다. 다섯 직업과 열 후보 배치는 유지한다.

`Server/Private/GameRoom_Colosseum.cpp`의 `Transfer_ColosseumMatchTo`는 모든 용병을
미선택 후보로 stage하며 빈 팀의 파티도 유지한다. 기존 `Handle_ColosseumRecruit`의
동일팀·거리·생존·중복 command·4명 제한 검증을 통과한 용병만 참가자로 commit한다.
`Try_StartColosseumEntry`는 양팀 파티가 정확히4명일 때만 ENTRY_COUNTDOWN을 시작하고,
기존90tick을300tick으로 바꿔 모집 완료 뒤10초를 보장한다. Server 30Hz가 시간 정본이다.
그 뒤 기존8.6초 도입과 전투 직전10초 창살 countdown은 유지한다. 따라서 현재 후보는
모집 완료 후10초와 도입 뒤10초를 각각 갖는다. 두 구간을 한 구간으로 합치는 변경은 포함하지 않는다.

`Client/Private/Level_Development.cpp`의 `Update_ColosseumMatch`는 같은 match ID의
MATCH_FOUND가 보관한 인간 roster 수를 준비 표시의 분모로 쓴다. 인간의 arrival index는
입장 순서0..N-1이며 고용 용병은 이후 팀 자리다. 이 범위의 Server bReady만 분자로 세어
1/1·2/2·3/3·4/4를 표시하고, combat Participants와 iExpectedPlayers를 인간 준비 수로
오해하지 않는다. 원격 캐릭터·외형 준비 및 typed LOAD_READY 장벽은 기존 경로를 유지한다.
Shared wire, protocol132, 새 H/CPP·project/filter 등록 변경은 없다.

인간1명은1/1 준비 후 후보 선택을 위해 입장할 수 있으나, 인간 없는 상대팀에는 기존
동일팀 고용 명령을 제출할 주체가 없다. 그 팀을 자동 충원하지 않으므로 양팀4/4가
되기 전에는 모집 상태에 머문다. 단독 경기 자동 시작을 구현했다고 기록하지 않는다.

`ServerGameplayContractTests_ColosseumMatch.cpp`의 실제 원자 입장 fixture에서1~3인
모두 자동 선택0명, 1인은 모집 유지, 2·3인은 직접 고용 후8명·300tick과 기존 후속 phase를
검사한다. 기존4인 fixture도300tick을 검증한다. Debug/Release Product 후 기존
`Server.exe --colosseum-match-contract-test`를 실행하고 UI count·실제 화면은 별도로 확인한다.
Client EXE의 c0000409/subcode7 종료는 이 표시·모집 변경으로 해결됐다고 주장하지 않으며,
실제 종료 stack·진단과 캐릭터 admission을 별도로 추적한다. 후보와 적용 기준 hash는
`out/MotionAuditFollower20261001/colosseum_candidate`에 보존한다.

## G08. 입장 컷신의 좌표 경계와 최종 배포 검증

`ColosseumIntroCutscene.h::Vector`는 JSON 배열 길이3을 검증하고도4회 접근했다.
이는 팀 인원4와 관계없는 XYZ 입력 및 출력 배열의 범위 초과다. 이미 검증한 배열 크기로
반복 범위를 정하고4인 이름 행·팀 슬롯은 유지한다. 실제 DataJson과 배포된 camera JSON으로
수정 전 범위 초과와 수정 후 좌표 로드를 대조한다. 서버 계약 검사만으로 이 Client 파서를
검증했다고 기록하지 않는다. 새 제품 H/CPP나 project/filter 등록은 필요하지 않다.

입장 배우는 Server가 확정한 양팀 participant를, 승리 배우는 Server가 확정한 winningTeam의
실제 참가자를 소비한다. 정상 경기에서 양팀4인·승리팀4인이며 미고용 후보를 배우에 넣지 않는다.
다른 세션이 수정하는 승리 컷신의 최신 파일·데이터·검증을 확인한 뒤 동일 Desktop 저장본으로
Product Debug/Release를 다시 빌드한다. 앞선 좌표 수정 전 receipt와 중단한 ZIP stage를 최종
수정본의 증거로 사용하지 않는다. 실제 화면 재현은 사용자 확인과 따로 기록한다.


### G08. 승리 배우 준비와 인원 회귀

현재 승리 JSON은 이미4자리이며 원본 curve에4m 후퇴·0.5m 상승을 적용한151개 key를
사용한다. 이를 다시 확장하지 않는다. 입장은 Server participant의 실제 Character를,
종료는 그 중 winningTeam의 실제 Character를 arrivalIndex/2 자리로 소비한다.
미고용 후보와 identity 불일치 표현체를 제외하고 이탈한 자리를 다른 배우로 채우지 않는다.

`ColosseumMatchView.cpp::IMPLEMENTATION::Has_PresentWinner`는 유효한 승리 자리와
실제 Character/Transform이 하나라도 있는지 확인한다. `Sample_Presentation`은
Build_Actors 뒤 이 조건이 거짓이면 카메라·배우 override를 정리하고 다음 frame에 다시
시도한다. Server 결과 시각을 멈추거나 새로 시작하지 않으므로 귀환 시점은 유지한다.

`Tools/LpkPipeline/test_colosseum_cutscene_roster.py`는 생산 배우 선택·정리 함수 본문을
최소 모의 presentation 타입으로 컴파일해 인원 변화, 양팀 승리4자리, 미선택 후보 제외,
정확한 identity 조인, 늦은 배우·이탈 및 visibility 복원을 검사한다. 렌더링 화면이나
실제 서버 입장 성공을 이 검사로 대신하지 않는다. 새 제품 C++/project 등록은 없다.


## G09. 단독 입장의 상대 팀 자동 배정

사용자가 2026-10-01에1인 경기만 상대 팀 용병4명 자동 배정을 명시했다. G07의 자동 배정
금지는 이 경우에 한해 대체한다. 인간이1명이고 해당 팀에 인간이 없는 경우에만 기존
Transfer_ColosseumMatchTo staging에서 다섯 후보 중 앞4명을 참가자로 확정한다.
본인 팀은 인간1명+직접 고용3명, 상대 팀은 자동 용병4명이며,2~4인에서는 자동 배정0명을
유지한다. 시작 장벽은 계속 양팀 각각4명·300tick 입장 안내 뒤 INTRO/COUNTDOWN이다.

Server 기존 player·party·arrivalIndex 계약을 사용하고 추가 가짜 session이나 Client 권위를
만들지 않는다. 모든 staging 및 spawn 준비가 성공한 뒤에만 transfer commit한다. 자동 상대
선정과 데이터·party·combat participant 반영은 한 transaction에 들어간다.

실제 Server 계약 검사에1~3인의 모집·ENTRY_COUNTDOWN·INTRO·COUNTDOWN·ACTIVE·
120초 종료·typed 결과 귀환과 정상 Bern profile 복구를 연결하고 기존4인 검사를 유지한다.
새 protocol/제품 파일/project 등록은 없다. 실행 중인 사용자 Client/Server는 조작하지 않는다.
고유 out 산출물로 후보를 컴파일·검사하며, 현재 정식ZIP의 무결성·4인 사용 가능 여부와
단독 지원을 추가한 다음 배포 후보를 구분한다. ZIP에 담기지 않은 수정을 담겼다고 기록하지 않는다.


### G10. 4인 준비 종료의 실제 font 예외와 준비 화면 격리

2026-10-01 08:28:24 Client PID62428의 CXX_TERMINATE에는 `Character not in font`가 남았다. 동일 EXE/Engine PDB로 해석한 호출은 `CLevel_Development::Render_PartyInviteText -> CGameInstance::Measure_Text -> SpriteFont::MeasureString -> FindGlyph`다. RECRUITING 문구의 U+00B7 두 개는 설치 YoonGasiIIM과 소형 파생 폰트에 없고 defaultCharacter도 0이다. AI나 loader 실패로 대체 설명하지 않는다.

- `Engine/Private/CustomFont.cpp`: Initialize에서 저작 defaultCharacter가 없을 때 실제 존재를 확인한 ASCII `?`를 fallback으로 지정한다. 대체 glyph도 없으면 기존 Initialize 실패 계약을 사용한다. Measure와 Draw가 같은 SpriteFont fallback을 소비한다. null text를 격리하고, 측정은 SpriteBatch Begin 전 수행해 실패가 열린 batch를 남기지 않게 한다.
- `Client/Private/Level_Development.cpp`: 모집 문구는 지원되는 ASCII 구분자를 사용한다. 준비·컷신 동안 world nameplate/HP/chat/party/interact text를 기존 cinematic 상태로 억제한다.
- `Client/Public/ColosseumLoadingView.h`: 실제 portrait가 준비되기 전 class illustration을 먼저 보이는 경로를 제거하고, 실제 Server identity와 일치하는 portrait가 준비됐을 때 표시한다. 재질·렌더링 옵션은 별도 실측 없이 변경하지 않는다.
- 검증: 실제 설치 font와 DirectXTK에서 수정 전 예외를 재현하고 수정 후 Measure/Draw 및 미지원 Unicode를 확인한다. Product Release/Debug 컴파일과 기존 인원 계약 검증을 수행한다. 4개 실제 Client 화면 검증은 사용자 실행 결과로만 완료한다.

기존 파일만 수정하므로 vcxproj/filters 등록 추가는 없다. Resources 원본과 사용자 렌더링 옵션은 교체하지 않는다.

G10 최종 범위 정정: 사용자의 추가 지시로 우선 배포에는 모집 문구1줄(`·` 두 곳→`|`)만 적용한다. 공통 font fallback·HUD·portrait 후보는 out에 보존하고 제품에서 제외했다. 완료 증거와 이후 ZIP 정본은 RESULT G15를 따른다.

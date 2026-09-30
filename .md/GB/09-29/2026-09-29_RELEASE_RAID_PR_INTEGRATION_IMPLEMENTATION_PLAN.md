# Release 레이드 검증·PR 통합·실행 ZIP 구현 계획

## G00. 기준선과 전달 범위

발탄 저장본 기준은 `e3f7ff1c5f3681e823619b05f2f172e7c85fddcb`, main은
`d944b0123c98c2ada7ad4c5b30d1ee3e7f1e15ff`다. 시작 시 working tree는 clean이며
기존 관련 편집 세션은 idle이었다. `codex/release-integration-20260929`에서 통합한다.
사용자는 열린 PR 전체 병합, 발탄 4인 Release 체력 기믹 반복·피해·콜라이더·사운드·
유령 부활부터 사망, 쿠크 입장부터 빙고 완료 검증과 기존 형식의 ZIP 생성을 요청했다.
Resources는 Drive로 공유하므로 ZIP에 포함하지 않는다. 기존 ZIP은 완성된 후보가
검증되기 전까지 유지하고, 최상위 실행 파일과 외부 Resources 선택 흐름을 보존한다.

## G01. PR과 통신 계약 통합

#475 `5bd57e2c0a94e9cdaba05880b5c772699b8b136a`,
#476 `6265d0d18874827ad7dbec4267793fd29bd5387e`,
#477 `49873bd756d7bfb68819ce01b9d1b8c16874b546`,
#478 `a3db26e8e7cbff37bc7691a01707393bbc2b61b3`를 검토·병합한다.
발탄 protocol119와 상점·마하라카의 독립 protocol118은 서로 호환되지 않으므로
통합 protocol120을 사용한다. packet ID의 순서를 보존하고 serializer/read/validation을
함께 검사한다. 생성 bootstrap 충돌은 병합된 저작 데이터와 정식 publisher로 해소한다.
새 C++ 파일은 현재 제안하지 않으며 기존 프로젝트·필터 등록의 병합 결과를 검사한다.

## G02. 레이드 소비자 검증과 누락 보완

기존 Server contract와 NetworkProtocolHarness를 사용해 게시 데이터를 읽는 실제
simulation·세션 계약을 검증한다. 발탄 체력 구간 반복, 컷씬 동기화, phase 전환,
유령 부활·사망 및 쿠크 관문·빙고·완료 경계를 확인한다. 이펙트가 있다는 이유만으로
피해를 임의 추가하지 않고 실제 damaging owner·contact·sound timing과 참조를 대조한다.
재현된 누락은 같은 기능의 기존 runtime에서 보완하고 관련 focused 회귀를 실행한다.
사용자 렌더링·패턴 튜닝은 보존한다. Client/UI 실행·육안 판정은 수행하지 않는다.

## G03. Release와 병합·배포

정식 Product Release Build를 실행하고 필요한 하네스도 Release로 빌드·검증한다.
JSON/XML parse, git diff --check, 게시 데이터와 resource reference를 확인한다.
통합본을 PR로 검토 가능하게 push하고 검증한 commit을 main에 병합한다.
각 기존 PR의 ancestry 및 GitHub 병합 상태를 확인한다. 기존 portable builder로
`C:/Users/user/Desktop/LostArk-Release-20260929.zip`을 생성·검사한다. 기존 20260923 ZIP은 보존한다.
실행한 자동 검사와 사용자 화면 미검증을 RESULT에서 분리한다.

## G02-1. 네 세션 발탄 연속 회귀

기존 `ServerGameplayContractTests_ValtanLifecycle.cpp`의 `Run_ValtanLifecycle`에
실제 게시 Catalog/World/Navigation과 네 `CClientSession`을 사용하는 연속 사례를 추가한다.
네 클래스의 실제 Join과 typed G 입장 후 컷씬·등장 휠윈드, 각 체력 구간의 ordered loop
한 순환과 재시작, 130/115/105/80/65/30/15 기믹, Death→Respawn의 40줄 회복,
유령 반복·최종 실제 플레이어 스킬 처치와 네 세션의 동일 snapshot/clear 전달을 확인한다.
검사 대상은 진행·복제 계약이며 장시간 관찰을 위한 플레이어 무적과 체력 문턱 이동은
테스트 fixture에서만 설정한다. 보스 stage·시간·순서·분기·재생 데이터는 변경하지 않는다.
독립 화면·네트워크 PC·실제 음성 청취 성공으로 기록하지 않는다. 기존 CPP 등록을 재사용하며
새 제품 파일이나 별도 검증 프레임워크는 없다. Release 통합 빌드 후 기존
`--valtan-lifecycle-contract-test`에서 실행한다.

## G04. Release Server 권위 수치 저장과 네 세션 반영

F1 Balance의 숫자는 Server가 보낸 stable domain/ID/field와 numeric revision을 기준으로
편집한다. 네 세션 중 어느 사용자의 저장도 typed patch로 Server에 제출하고, Server는
기존 bootstrap 수치 열만 변경한 후보를 기존 Catalog parser로 전체 검증한다. 저작 원본과
게시 데이터의 최신 저장본을 CAS로 확인·백업·원자 교체한 후 모든 방이 같은 tick에서
새 수치를 소비한다. 이전 gameplay/presentation revision과 실행 중 패턴 순서·시간·기믹
ledger를 보존하고 별도 numeric revision으로 충돌을 검출한다. 플레이어·보스 HP와
자원은 기존 비율, 사망은 0을 유지하며 이후 판정에는 새 피해·무력화 수치를 사용한다.
무력화는 미소비 boss 메타데이터 대신 실제 SET_STAGGER_GAUGE 값을 노출한다.
새 ServerBalanceNumericStore H/CPP와 필요한 focused 테스트는 Server 프로젝트와
filters에 등록하고 Release 컴파일, 잘못된/오래된 patch 거부, 저장 실패 보존,
네 세션의 동일 적용과 ghost HP profile 보존을 기존 contract runner에서 검증한다.


## G05. PR484와 현재 저장본의 후속 Release 통합

기준 main은 23008e0ec이며 PR483은 이미 병합됐다. 사용자는 PR484, PR482와 현재
Desktop 작업의 통합·병합 및 같은 Release ZIP 재생성을 승인했다. 원본 Desktop의
Movie/사운드/발탄/쿠크/아이템 변경 164개 파일을 hash와 함께 별도 관리 worktree에
보존했다. 임시 backup·retired 파일은 보존하되 소스 커밋과 배포에서 제외한다.
PR482의 기능과 수치는 이미 최신 저장본에 포함되어 있어 후속 Movie 편집과 새 게시본을
유지하면서 ancestry를 병합한다. 이전 numeric receipt를 새 bootstrap에 붙이지 않는다.
PR484의 캐릭터 저장, 상점과 나가기 투표를 현재 배틀 아이템 사용 경로에 병합하고
충돌하는 독립 protocol122/123 대신 통합 protocol124를 사용한다.

## G06. 루가루와 마리오 상하 이동 공

Retail monsters의 MINIBOSS_LUGARU maximumHp만 일반 몬스터 대표 체력476056의
5배2380280으로 변경한다. base MonsterProfiles만 변경해 Retail에 가려지지 않게 한다.
상하 이동 공13개는 실제 저작 placement·scale·Y curve를 World publisher가 전달하고,
Server가 같은 clock의 접촉을 판정한다. 기존 Mario 폭탄의 launch/knockdown helper와
1320 고정 피해를 재사용한다. 반복 접촉은 contact latch로 제어하고 Client Transform을
피해 권위로 사용하지 않는다. 기존 WorldBootstrap/room 경로를 확장하며 새 C++ 파일은
추가하지 않는다. World schema 및 관련 publisher/reader를 같은 변경에서 검증한다.

## G07. EXE 실행 중 여러 캐릭터와 상태 유지

사용자는 후속 지시에서 EXE 종료 뒤 개인 JSON을 다시 읽는 영속 로스터를 철회했다.
하나의 process-session 안에서 여러 캐릭터의 stable local ID, nickname, 외형,
인벤토리·장비·실링·골드·칭호를 유지한다. 캐릭터 선택으로 돌아오기 전에 현재 Server
snapshot을 해당 캐릭터에 저장하고 다시 선택하면 Bern 입장 후 typed 복원 요청·결과로
Server에 적용한다. 실패하거나 아직 응답이 없으면 이전 캐릭터 상태를 보존한다.
EXE 종료 시 session roster는 사라지고 다음 실행은 빈 로스터다. 기존 개인 JSON을
읽거나 쓰거나 삭제하지 않고 Git/portable ZIP에도 개인 상태를 포함하지 않는다.
Debug/Release Lobby는 존재하는 session roster 카드만 준비하며 최초 빈 로스터를 위해
4개 캐릭터 모델을 미리 로드하지 않는다. 생성용 모델은 기존 생성 화면의 진입 경로가
필요할 때 준비한다. 최초 Lobby에 Bern 로딩 화면을 다시 표시하지 않는다.
외형 serialize의 RGBA trailing comma를 수정해 session 카드와 실제 캐릭터에 적용한다.

## G08. 전투 소비와 최종 배포 검증

실제 Server fixture로 파괴폭탄의 투척·접촉·유효한 GROGGY 창의 갑옷1/2 파괴와
PART_BROKEN 전송을 검사한다. 회오리 수류탄·성스러운 부적·시간정지물약의 기존
사용/효과/수량·쿨타임 계약을 재실행한다. 발탄4세션은 phase3 전환 때 ending 없이
유령 부활 후 최종 사망에서만 ending으로 진행하는지, 쿠크4세션은 입장부터 빙고
완료까지 기존 lifecycle/raid/bingo 계약으로 확인한다. 화면·음향·실제4PC는 사용자 확인이다.
최신 정본을 공식 publisher로 게시하고 Release Product와 필요한 focused harness를
빌드한다. JSON/XML parse와 diff check 후 통합 PR을 push·merge하고 portable builder로
기존 ZIP을 백업한 뒤 원자 교체한다. ZIP CRC/manifest SHA/launcher --check와 실제
포함 EXE·CSO·Data 세대를 확인한다. 기존 Resources는 외부 참조로 유지한다.


## G09. 지정 패턴의 무력화 요구량 열 배

사용자가 명시적으로 확인한 대상은 스킬 무력화 피해가 아니라 패턴의 요구 무력화량이다.
발탄의 무력화 패턴, 쿠크1관문 무력화 패턴과3관문 마리오2페이즈의 실제 Server 소비
정본을 찾아 현재값의10배로 설정한다. 모든 스킬이나 전역 Retail 배율에 전파하지 않는다.
회오리 수류탄의 현재 최대 게이지1/3 기여 정책은 보존한다. 기존값·변경값·대상 stable ID와
게시된 소비값은 RESULT에 기록하고 정본 publisher로 Gameplay/Composition을 갱신한다.


## G10. 배포 후 미실행 회귀와 실패 분류

배포한 protocol124 Release Server와 현재 소스를 기준으로 남은 쿠크·발탄·아이템·입장·
통신 검사를 수행한다. 사용자 Client/Server가 실행 중이므로 종료하거나 UI를 조작하지 않는다.
headless runner의 listener 이전 분기를 확인하고 테스트마다 격리 DataFiles·TEMP를 사용한다.
numeric 저장 검사는 복사본만 수정하며 배포 ZIP과 실행 중 Server의 원본 Data는 보존한다.
Debug 조건으로 제외된 Release 검사는 assertion 수와 함께 표시하며 실행되지 않은 검사를
PASS로 대신하지 않는다. 실패는 실제 제품 결함, 최신 protocol/저장값과 어긋난 fixture,
검사 호출 환경 문제로 구분하고 재현 근거를 확인한 최소 범위만 교정한다.
기존 NetworkProtocolHarness의 protocol122 기대값 11개는 통합124 계약과 비교해
packet ID·부정 입력 검사를 보존한 채 최신 기대값으로 갱신하고 전체 검사를 다시 실행한다.
새 제품 C++ 파일은 추가하지 않으며 기존 등록과 harness 프로젝트를 사용한다.

## G11. 통합 병목의 증거와 재발 방지 계약

직전 두 Release receipt와 MSBuild diagnostic/binlog, source snapshot, shader 보존 증거를
대조한다. branch 이름, 실제 파일 재작성, bytes/줄바꿈, 수정시각, compiler command와
tracking/toolchain을 분리하여 재컴파일 원인을 판정한다. 2273개 입력의 SHA256·mtime을
보존한 동일 tree 전환을 먼저 검증했다. 두 거대 Mesh/AnimMesh effect의 include·pass·
variant 컴파일 비용을 조사하되 Client 전체 경과를 단일 shader 시간으로 기록하지 않는다.
컴파일 비용과 게임 GPU 비용을 구분하고 현 상태의 재사용과 향후 shader 분리 최적화의
검증 기준을 명시한다. 구현하지 않은 shader 최적화를 완료라고 기록하지 않는다.
사용자가 명시한 AGENTS.md와 gotchas.md에 source/cache/merge/publish/build/package의
세부 절차와 금지 우회, 정확한 증거·한계 및 남은 최적화 항목을 기록한다.
검증과 문서를 PR로 main에 병합한다. 코드 수정이 필요한 경우 실제 영향받는 최소 대상만
컴파일하며 사용 중인 제품 EXE/DLL을 덮어쓰는 build 또는 shader 전체 재빌드는 수행하지 않는다.

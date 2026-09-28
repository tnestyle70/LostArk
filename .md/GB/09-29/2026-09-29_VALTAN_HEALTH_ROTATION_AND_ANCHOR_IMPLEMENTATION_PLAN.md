# 발탄 체력 기믹·반복 순서와 부채꼴 회전 구현 계획

## G00. 현재 저장본과 변경 경계

기존 이펙트·HUD·게시 작업 위에 추가된 요청을 처리한다. 현재 디스크 저장본 기준 반영과
publish/build는 사용자에게 승인받았다. 저장된 돌진 이펙트와 패턴 타이밍을 보존하고
stable ID/필드별 후보를 최신 저장본에 병합한다. 교체 직전 hash 확인, 원문 백업,
원자 교체와 자기 변경 rollback을 유지한다. Client/UI는 자동 실행하지 않는다.

## G01. decisionModel과 게시 경로

`Valtan.gameplay.json`의 자동 전투를 `HEALTH_BAR_ROTATIONS`로 전환한다.
기존 scriptedSequence의 전체 시연 순서는 보존한다. 자동 전투는 등장 이후 체력 기믹을
먼저 처리하고 현재 phase/체력 구간의 ORDERED_LOOP를 순서대로 반복한다.
selectionSets는 기존 WEIGHTED_POOL/candidates와 새 ORDERED_LOOP/patternIds의 tagged union이다.
같은 패턴의 중복 순번은 유효하다. 구간 진입 때 순번을 초기화하며 loop 끝에서 처음으로 돌아간다.
Server가 선택·피해·전환을 소유하고 Client는 게시된 자료를 표시한다.

체력 기믹은 130 전멸, 115 외곽 파괴, 105 피자, 80 3시 파괴, 65 버러지,
30 9시 파괴, 15 발악 순서다. 발악의 기존 사망·유령 부활 연결을 보존하고 유령은
40줄/5개 반복 패턴을 소비한다. 8개 구간의 patternIds는 사용자 지정 순서이며
과거 legacy 체력 기믹 5개는 AUDITION_ONLY로 남겨 중복 자동 발동을 막는다.

`valtan_tuning_pipeline.py`는 source 검증과 Product projection을 담당한다.
`Publish-GameplayBalance.ps1`는 기존 PATTERNROTATIONSTEP/WINDOW와 PATTERNSEQUENCE
행에 새 mode를 게시한다. 별도 bootstrap이나 두 번째 전투 경로를 만들지 않는다.
phase 경계는 outer-break/struggling trigger와 ghost profile health까지 검증한다.
`ValtanPatternTree`와 `ValtanPatternFlowDocument`는 새 mode를 읽고 저장할 때 유지한다.
실패하면 기존 문서와 실행 데이터가 남아야 한다.

## G02. Server 순서와 중앙 이동

`GameplayCatalog`의 mode와 rotation row 검증을 확장하고 `ValtanBrain`의 기존 HP queue와
rotation cursor를 사용한다. 유령은 `GameRoom_BossSimulation`의 기존 finale loop를 사용하되
phase3 ORDERED_LOOP의 다섯 패턴을 공급한다. 체력을 한 번에 여러 구간 넘긴 경우에도
기믹을 높은 줄부터 한 번씩 처리하고 나중 구간의 반복 패턴을 실행한다.
80줄 terrain의 기존 TO_ARENA_CENTER motion과 ownerHitChain의 내부/외부 돌 시차를 확인한다.
새 C++ 파일은 추가하지 않아 vcxproj/filter 신규 등록은 없다.

## G03. 무적과 표시

130줄 패턴의 기존 invulnerableWhileRunning을 유지하고 blocked hit에 기존 INVINCIBLE flag의
0 damage 이벤트를 보낸다. packet validator는 이 조합만 예외로 허용하며 무적 이벤트에
damage/stagger/기믹 피해가 섞이면 거부한다. CombatHUDViewModel은 표시하되 DPS에 합산하지 않고
MainApp은 파란 `무적` 텍스트를 그린다.
바훈투르는 다른 담당자의 기존 50% 피해 감소·전멸 차단·30초 지속을 유지한다. 이번 완전 무적 추가는 제거한다.

## G04. 부채꼴의 최종 quad 회전

stage004의 팬 3개는 fixed-axis sprite에서 최종 quad가 emitter yaw를 버린다.
기존 B_Root 부착을 유지하고 followEmitterAxisRotation을 해당 occurrence에만 연결한다.
실제 import 본 축을 socket rotation `[0,90,-90]`으로 수평 정렬해 기존 CCW90 방향을 유지한다.
무기 본 10개, 저장 scale, 다른 정상 이펙트는 그대로 둔다. 같은 asset을 쓰는 피자·3시·9시에
공통 적용한다. 원점 particle matrix만 보지 않고 실제 설치 모델의 비원점·이동·회전 입력과
최종 quad의 pivot/전방을 검사한다.

## G05. 검증과 전달

후보 projection과 malformed mode/phase/순서 거부, Server HP queue/loop/counter/finale,
바훈 보호·130 hit/packet/HUD, 최종 quad 수치를 각각 검증한다. 최신 저장본에 원자 병합한 뒤
정식 Gameplay/Composition publisher와 PublishCandidate를 실행한다. Debug Product build와
관련 native contract를 실행하고 source/candidate/설치 generation hash를 맞춘다.
실행한 항목만 RESULT에 남긴다. 화면 및 조작 판정은 사용자가 재실행 후 확인한다.
실제 Resources 추가가 생긴 경우만 GBResources에 경로와 hash를 보존해 전달한다.

## G06. 입장 연출과 기존 등장 휠윈드

사용자 정정에 따라 Stage_Boss의 기존 G 입력을 유지한다. HEALTH_BAR_ROTATIONS의
scriptedSequence에 optional entranceCinematicPatternId를 선언하고 자동 입장 시 이 컷씬을
한 번 실행한 뒤 기존 encounter introPatternId의 등장 휠윈드를 실행한다. 이후 G01의
체력 기믹·반복 순서로 진행한다. 명시적 Play All의 70개 occurrence는 보존한다.
Server Catalog는 PATTERNSEQUENCE의 optional 마지막 cinematic ID를 검증하고,
Brain은 boss의 입장 소비 상태로 컷씬과 기존 등장 휠윈드를 구분한다. Client 저장과
typed patch는 저작한 cinematic ID를 보존한다. 보스 입장 G 처리 자체는 변경하지 않는다.

바훈투르는 사용자가 다른 담당자의 기존 구현 유지를 명시했으므로 이번 완전 무적
추가만 원복한다. 기존 50% 감소·전멸 차단과 130줄 발탄의 무적 표시를 유지한다.

## G09. 유령 부활 체력바와 생성 간격

동일 primary BOSS_VALTAN entity가 phase 3으로 전환되어도 HUD는 ghost profile의 40줄을 소비한다. HP와 최대 HP는 계속 Server snapshot 값이다. MainApp은 최대 HP 변경을 새 체력 단계로 받아 감소 연출 cache를 초기화한다. 기존 막대 색 순환·배치·피격 표현은 보존한다. 실제 Apply_Boss와 전체 Room 부활 완료 시점을 검증한다. 보조 유령은 소멸 후 5000ms 대기와 최대 1개, 삼각 포탈은 시작 간 10000ms를 finale의 optional 필드로 게시한다. 기존 자산 수명은 유지한다.

보조 유령의 finale pool은 휠윈드·4연속·2페이즈4방향·십자돌 네 종류다. 메인 40줄의 다섯 반복 패턴과 구분한다. PATTERNFINALE은 보존하고 optional PATTERNFINALEINTERVAL 행이 두 간격을 전달한다. legacy 미지정은 기존 재생성·7900ms를 유지한다.

## G10. 80줄 이하 반복의 워프 추가

사용자 최종 정정에 따라80→65,65→30,30→15의 ORDERED_LOOP 마지막에 VALTAN_WARP를 추가한다. 앞선9개 순서는 유지하고 Dash→Warp→CatchBreath로 순환한다. 기존3시·9시 지형파괴와65줄 기믹, 주/보조 유령 목록, Play All 저장순서는 바꾸지 않는다. 기존 워프10stage와 target rush 경로를 재사용하고 실제 선택·완료·다음 loop 진입을 검증한 뒤 재게시한다.

## G12. 실제 아레나 입장 파서의 finale 간격 호환

Client/Private/EncounterPatternReference.cpp의 Validate_PatternFinale는 실제
Level_ValtanArena::Ready_CinematicCamera에서 쓰는 읽기 전용 입장 검증이다.
에디터의 ValtanPatternTree와 별도 소비자이므로 같은 optional 간격 필드를 허용한다.
required 5개를 유지하고 auxiliarySpawnIntervalMs/portalSpawnIntervalMs 두 필드만
추가 허용한다. 명시 값은 1..600000 정수이며 누락은 기존 문서 계약을 보존한다.
정본 VALTAN_GHOST_FINALE는 현재 네 패턴과 legacy 여섯 패턴의 순서를 검사한다.
generic finale의 동적 child 계약, 순환·중복·존재 및 invulnerability 검증은 유지한다.

기존 ValtanEncounterReferenceContractTests.cpp에 전체 문서 Load와 malformed 입력 시
이전 상태 보존 검사를 추가한다. 새 제품 C++ 파일이 없어 프로젝트 등록은 필요 없다.
수정 전 원문과 수정 후 전체 Load에 같은 실제 입장 문서를 넣어 오류 재현과 해소를
확인하고 이어지는 전체 camera document Load와 controller Initialize를 실행한다.
Data를 수정하지 않고 Debug Product 빌드로 Client EXE를 교체한다.

## G13. 게시 완료 후 수동 패턴의 Client parity 차단

Client/Private/ValtanPatternTree.cpp의 split source→Master 파생값 계산을 Python
compile_pattern의 동일 계약과 맞춘다. 수동 시연 패턴은 Server 자동 반복 제한을
사용하지 않으므로 maximumConsecutiveUses=0이며, 일반 자동 후보만 repeatPolicy.limit를
소비한다. source 반복 입력을 바꾸거나 strict parity 검사를 제거하지 않는다.
실제 전체 CValtanPatternTree::Load를 최신 제품 OBJ로 실행해 VALTAN_FIST_IN_OUT의
변경 전 오류를 재현하고, 수정 후 전체 source/Product 연결을 검사한다. 수동 owner의
파생 필드와 malformed Product 거부를 함께 확인한다. 신규 C++ 파일은 없다.
사용자가 시작한 Full DataOnly 게시 완료와 실행 중 Client의 파일 점유를 구분하고,
새 Client 링크에 필요한 종료 시점만 안내한다.

## G14. 전체 게시의 World destruction stage aim 계약

Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1의 stage exact-property 검증에
현재 Client/Server/Gameplay publisher가 소비하는 optional aim을 같은 계약으로 연결한다.
전체 67패턴/328stage를 비교하여 빠진 필드를 확인하고, 중첩 policy와 endMs 및
responseScale의 타입·범위 검증을 유지한다. unknown 필드 허용이나 원본 aim 제거로
우회하지 않는다. 실제 전체 -Mode Validate 후 정식 Full DataOnly를 재실행해
모든 domain의 완료 결과를 확인한다. 이미 성공한 domain은 기존 캐시 규칙을 사용한다.

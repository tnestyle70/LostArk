# 쿠크 Logic Box Detail 수치 튜닝 구현 계획

## G01. Logic와 연결 Result 편집

현재 `Render_LogicBoxDetails`는 시간과 Result ID만 편집하고 수치 editor는 Resources에만 있다.
기존 `Render_LogicDefinitionValues`를 Logic Box와 각 Success/Fail/Timeout Result 아래에서 사용한다.
같은 프레임의 여러 정의가 서로의 미적용 값을 지우지 않도록 Box Detail별 stable Logic ID draft를
보존한다. Apply는 기존 `Set_LogicDefinitionValues -> Commit_Candidate` 검증 경로를 사용한다.
공유 정의 수치를 바꾸면 같은 정의를 사용하는 패턴에 함께 적용됨을 화면에 표시한다.

## G02. Duration 접촉 틱과 카드병정

`AREA_OVERLAP`의 optional `repeatIntervalMs`를 0 또는 34..600000 ms로 확장한다.
0은 기존 종료시점 판정을 유지하고 양수는 수명 중 접촉한 플레이어에게 반복 Result를 적용한다.
일반 불과 인형·공은 각 Duration의 Success에 FIXED_DAMAGE와 MADNESS_GAUGE_ADD_PERCENT를
연결하고 피해/광기는 Result, 틱은 Duration에서 조절한다. 서버 소비와 bootstrap은 Server 담당이 연결한다.

`CARD_RAIN_SOLDIERS`에는 `soldierCounts`(CLUB/HEART/DIAMOND, 각0..32/합계1..64),
`spawnRadiusMinM`/`spawnRadiusMaxM`(0..100m, min<=max)을 추가한다. 생략은 기존1/1/1·3..6m다.
Client codec, Python projection, PS bootstrap, Server nav 생성이 같은 typed 계약을 사용한다.
카드낙하는 기존 double `randomScaleMin`/`randomScaleMax`와 반경을 그대로 노출하고 소수 보존을 검사한다.

## G03. 변경 파일과 검증

기존 `KoukuSaydonActionWorkbench.h/.cpp`, `KoukuSaydonCompositionDocument.h/.cpp`,
`project_kouku_saydon_composition.py`를 수정하므로 제품 project/filter 추가 등록은 없다.
신규 독립 Python test에서 기본값, 부동소수점 유지, 잘못된 값 거부와 projection을 확인한다.
Product 빌드·서버 소비 계약은 root의 통합 실행으로 검증한다. Client/UI 실행과 rendering 옵션 변경은 하지 않는다.
정본 Data JSON은 root가 최신 저장본 stable ID 필드 병합으로 적용하며 이 작업은 직접 쓰지 않는다.

## G04. 실제 본 접촉과 World 소유권

BOSS와 WORLD의 BONE Collider는 설치된 모델의 실제 본 행렬에 occurrence의 local TRS를 먼저
합성한다. 합성된 중심과 전방을 XZ로 투영한 값만 Server worldTrack에 저장하고, region의
center/yaw는 0으로 두어 offset을 두 번 적용하지 않는다. Client wire도 같은 순서로 합성한다.
기존 TARGET_YAW는 본 위치와 소유 오브젝트 yaw를 유지한다. WORLD는 기존
CWorldSequencePlayer가 샘플링한 CWorldSequenceObject/CModel을 읽으며 별도 모델 경로를 만들지 않는다.

반복 AREA_OVERLAP은 종료 한 점 대신 BOSS/WORLD의 연속 track을 소비한다. 인형 양쪽 입
b_mouth_f/b_mouth_b는 기존 World Object collider의 설치 WModel/clip sampler를 재사용한다.
정확한 WORLD occurrence에 묶인 전체 수명 반복 광기 접촉이 있을 때만 authoredMadness를
투영하여 기존 자율 aura의 중복을 막는다. combatBody를 실제 가진 동일 WORLD이면
ownerWorldOccurrenceId를 함께 보내 조기 파괴 후 접촉이 남지 않게 한다.

cross direction의 leaf child는 AREA_OVERLAP/ENTER_AREA와 HP·광기 Result만 허용한다.
각 실제 actor/clone의 Server ledger가 실제 root motion 뒤 contact를 평가한다. 이동·후속패턴 등
행동 소유권을 바꾸는 child Logic은 계속 거부한다.

## G05. 빙고 세 번째 폭탄 판정

BINGO_COMPLETED_LINES Duration은 threshold 기본 3, 범위 1..10으로 실제 세 번째 폭탄의
폭발 사건에 있는 가로·세로 완성 줄 수를 판정한다. Success에 PLAYER_INVULNERABILITY
Result를 연결하며 durationMs는 1..600000, 요청 기본값은 30000이다. 마지막 BINGO_DETONATION
Trigger는 Server가 보존한 판정 결과에 따라 보스 피해 또는 파티 전멸을 처리한다.

추가 C++ 변경은 기존 WorldSequencePlayer.h/_Objects.cpp와 Level_KakulSaydonArena.h/.cpp의
optional bone pivot 전달 API뿐이다. 신규 C++ 파일이 없어 프로젝트·필터 추가가 없다.

## G06. 게시기의 sampled yaw 검증 일치

Gameplay balance가 include하는 KoukuBootstrapRows의 BOSS_CURRENT worldTrack은 실제 본의
샘플링된 planar yaw를 허용한다. 각 key의 quaternion은 finite/unit 검증을 유지하고 visible,
scaleMultiplier [1,1,1], identity baseline, contact 시작/끝과 일치하는 clock은 계속 강제한다.
이는 TARGET_YAW의 기존 identity key와 BONE의 회전 key를 모두 같은 저장 계약으로 수용한다.
기존 result tuning test target에서 실제 PowerShell 검증 블록을 실행해 회전 key 허용과
잘못된 quaternion/scale/hidden/baseline/clock 거부를 확인한다.

## G07. Python bootstrap 용량 경계 검증

Server 담당이 확장한 131072행/67108864바이트 정본 계약을 기존 Python admission test class에서
검증한다. 행 상한·상한+1·잘린 행 수 검증을 유지하며 byte 상한·상한+1, stat 이후 파일 증가 시
bounded read의 거부, Shared 상수 일치를 추가한다. 실파일 용량 test는 작은 cap으로 mock하되
실제 배포 상수 64MiB/131072와 Shared의 일치를 따로 검증한다.

## G08. disabled 관문 배치 fixture 정합

현행 저장/게시본은 G1 Kouku도 disabled이며 room 생성만으로 보스가 나타나지 않는다.
해당 Debug fixture는 G1을 명시적으로 build/activate한 뒤 Saydon과 별개로 idle을 유지하는지
검증한다. enabled placement 거부는 복사한 입력으로 계속 검사한다. 실제 all-despawn은
G1/G3 모두와 재생 owner를 제거하고 ABORT를 내며 Esther는 보존해야 한다. 제품 로직/배치
데이터는 변경하지 않고 기존 KoukuProduct fixture의 이 구간만 교정한다.

## G09. standalone 관문 전환 fixture의 현재 관문 설정

Spawn_GatePlacement 뒤 실제 Debug spawn과 같은 Note_GatePlacementRaised를 호출한다.
투표 전 source gate 및 해당 clear bit가 초기화됐는지 검증하여 G2/G3 restart가 초기값
0을 G1으로 해석하는 잘못된 fixture를 막는다. 기존 준비·부분 READY·실패 보존·intro 검증을
유지하며 제품 로직과 저작/게시 데이터는 변경하지 않는다.

독립 재검토에서 G08의 충돌 목적지는 playerSpawn placement가 아니라 Client Get_DebugGates와
Server KOUKU_GATES가 쓰는 실제 F1 좌표 (-2.45,1.32,740.37)로 대조한다.

## G10. 빙고 Flow의 명시적 특수 Parent 참조

BINGO Pattern Flow의 optional bingoSpecialPatternId를 Client codec/Flow UI/삭제 참조에
연결한다. 미지정 기존 초안은 계속 읽고 저장할 수 있다. 실제 BINGO Flow 게시에는 지정이
필수이며 같은 BINGO 보스·actor의 자식이 있는 Parent와 closure 전체의 단일 DETONATION을
검증한다. 일반 entries·반복 시작 ID·기존 Bundle 실행 의미는 유지한다. Python raid projection은
동일 필드를 전달하고 Server 담당은 RAIDBINGOSPECIAL supplemental row와 runtime 소비를 잇는다.
P107은 기존 flatten Parent이므로 raw child 구조는 source publisher가 검증하고 runtime는 실제
펼친 scope/trigger를 검사한다. 기존 파일만 수정하므로 project/filter 추가는 없다.


## G11. 3관문 HP 반복 Flow 후보

현재 revision2346을 읽어 공통 반복 10개를 P52,119,59,116,118,39,38,117,43,40으로
연결한다. 180→155,155→125,125→90,90→80,80→55,55→0의 여섯 구간에 같은 순서를
반복하며 기존 G1의 entryGroups/repeatUntilHealthBars/PATTERN_END 계약을 사용한다.
각 임계값 사이에는 P88, P91, P76→35, P92, P93을 한 번씩 둔다. 마리오 P33은
MARIO_ENTER followup이 이미 소유하므로 Flow에 다시 넣지 않는다. P88/91/92/93의 내부
자식 순서는 일반 HP 구간과 다른 입장창의 저작 내용이므로 보존한다.

P116의 기존 start0/100ms 방향 추적 occurrence만 기존 typed BOSS_TRACK_TARGET Trigger로
연결하고 34ms로 저장한다. 공유 DURATION65는 다른 소비자가 있으므로 바꾸지 않는다.
새 Logic ID는 최신 nextLogicOrdinal에서 할당하며 같은 typed 정의가 있으면 재사용한다.
실제 Data를 쓰지 않고 out/KoukuHealthFlow20260925의 callable candidate와 receipt로 전달한다.
노란 장판의 MAP 좌표 및 시작 공3개·음성은 원본 근거를 조사하며 미확정 clip을 임의 연결하지
않는다. 제품 C++·빌드·게시·UI 실행은 이 후보 작업 범위에 없다.

## G12. Logic이 참조하는 단일 Effect 그룹과 Collider Apply

현재 revision2354의 P59 화염파동 Collider에 수평0m·높이3m·500ms·ballistic/force를
입력하면 실제 제품 Workbench도 Apply=false, Dirty=false로 거절한다. 값 자체가 아니라
`Commit_Candidate`의 `Remove_SingletonPresentationGroups`가 다른 P47의
selectedEffectGroupId가 가리키는 단일 Effect 그룹을 지워 전체 문서 검증이 실패한다.

`Client/Private/KoukuSaydonActionWorkbench.cpp`의 기존 정리 함수에서 Pattern의 실제
Logic occurrence가 참조하는 selectedEffectGroupId와 fixedSelectionGroupId를 모아 보존한다.
참조되지 않는 UI 편의 그룹만 기존대로 단일 멤버일 때 해제한다. 누락된 대상·빈 그룹의
검증은 유지하며, 실패한 후보는 기존 draft를 보존한다. 새 런타임·데이터 필드는 없다.

기존 `BossCompositionDocumentContractTests.cpp`에 관련 회귀를 추가한다. 실제 P59 Apply,
Dirty 전환, scratch Save/Reopen의 수평0·높이3·force 유지, 다른 Pattern의 의미 있는 그룹
보존과 비참조 singleton 정리·진짜 누락 거부를 검사한다. 기존 CPP만 수정하므로
vcxproj/filters 추가는 없다. 실행 중 Client는 종료·Reload하지 않으며 제품 링크가 잠겨 있으면
격리 컴파일 결과와 제품 반영 대기를 분리한다. 갈고리·마리오 렌더링 저장값은 읽기만 한다.

사용자의 일괄 적용 요청은 GATE3의 P59에 있는 동일 불기둥 원통 resource356 Collider28개와
연결된28개 접촉창 전체다. 기존 공용 피해 Result를 바꾸지 않고 전용 Result를 연결해
수평0m·상승3m·ballistic=true·force=true, 기존 이동시간이 없는 경우 UI 기본500ms를 저장한다.
다른 패턴·갈고리 placement·렌더링 옵션은 보존하며 최신 bytes 재확인·백업·원자 교체를 쓴다.

## G13. Mario 사망과 Encore 전환의 정상 수명

`GameRoom_KoukuAudition.cpp`의 solo Mario entrant 사망은 정상 기믹 실패로 완료해야 한다.
HP0을 전체 audition 오류로 분류해 보스까지 제거하는 분기를 교정하고 corpse 귀환·명시적
Revive·동일 보스 보존을 기존 KoukuProduct 계약에 검사한다. session 이탈·실제 구조 오류의
정리 경계는 유지한다.

Encore 종료는 현재 고정 tick으로 판단하지만 `Enter_KoukuRaidCombat`은 이전 committed tick을
사용해 1틱 이른 진입으로 거절한다. `GameRoom.h`, `GameRoom_GateProgress.cpp`,
`GameRoom_KoukuRaidFlow.cpp`의 기존 함수에 명시 tick을 전달하여 admission과 start 시계를
일치시킨다. 기존 KoukuRaid 계약에서 실제 Tick의 종료직전·종료시점, 1~4인 전환을 검사한다.
기존 C++ 파일과 테스트를 사용하며 project/filter 등록 변경은 없다.

추가 순서 검증에서는 command drain의 revive가 다음 Prepare보다 먼저 처리될 수 있다.
`GameRoom_PlayerCommands.cpp`는 목적지 검증을 마친 HP0 solo entrant의 성공적인 부활 직전에
해당 기믹을 실패 완료하여 죽음 관측을 놓치지 않게 한다. 동일 Mario 회귀에 즉시 부활 경로를
추가하고 다음 Flow와 같은 보스를 보존하는지 확인한다.

## G14. 피자 방향·조커 대상 표시·공유 핑

사용자는 마지막 저장본 기준으로 P25 피자 중앙 이동 때 몸을 시계 방향90도 더 회전하도록
요청했다. 기존 resetBossYawDegrees126.5를216.5로 바꾸고 중앙 이동 위치·소환·Collider는
보존한다. 조커 P13의 시야 Effect는 사용자가 저장한 파1빨2 노란 시선의 실제 asset과
placement 값을 확인해 교체하며 기존 Effect lifetime/loop 계약을 사용한다. 데이터는 root가
최신 bytes에서 요청 필드만 병합하고 backup·freshness·원자 교체 후 공식 publisher를 사용한다.

조커 머리 위 과녁은 망치별 Server retarget와 기존 snapshot의 target entity ID를 소비한다.
Client arena는 실제 remote/local character에 같은 effect.world.target_reticle을 부착하고
대상 교체·패턴 종료·Level 종료 때 해제한다. 타격·대상 선택을 Client가 재판정하지 않는다.

사용자 후속 지시에 따라 선택은 P13 시작0ms와 첫·둘째 망치 복구 뒤6266/12560ms의 총3회다.
typed `BOSS_RANDOM_TARGET` Trigger를 추가하고 기존 Server random-alive 선택·snapshot을
재사용한다. 첫 선택 뒤에는 생존 후보가 둘 이상이면 직전 표적을 제외한다. 기존 stage별
retargetOnEnter4개를 비활성화해 최종 전멸 구간에서 네 번째 선택이 발생하지 않게 한다.
Logic120은 P21/P58/P85가 이미 사용하는 placeholder이므로 전용 Logic546을 추가한다.

Ctrl+좌클릭은 기존 local ping/과녁 동시 표시에서 하늘색 위치 핑만 남긴다. Controller가
typed command sink로 요청하고 Shared packet→Server room admission/broadcast→Client 수신
presentation으로 연결한다. 같은 room의 모든 참가자가 sender 포함 동일 Server 위치를
보며, 다른 room으로 전파하지 않는다. stale sequence·비정상 좌표·인증된 player/world와
일치하지 않는 요청을 거부하는 좁은 계약 검증과 변경 TU/Product 컴파일을 수행한다.
새 파일이 필요하면 해당 vcxproj/filters를 함께 등록하며, Client/UI는 사용자가 확인한다.

## G15. 빙고 바닥 반전·새 한 줄 보상·이난나 보호

사용자 최종 정정에 따라 아직 보상에 사용하지 않은 가로·세로 빨간 줄1개를 새로 완성할
때마다30초 무적이다. 사용한 줄은 다음 보상 집계에서 제외하고 이난나30초 무적으로도
빙고 블랙홀 전멸을 피할 수 있어야 한다. 기존 첫 폭탄30초·후속20초·세 번째 MARK의
special 진입과 P107 이동→첫 clip→메두사1회→블랙홀 loop→폭발 시계는 이미 일치해 보존한다.

CKoukuBingoRuntime은 nonred cell을 폭탄 십자 범위에서 XOR하고 가로·세로10줄만 완성 시
빨간색으로 승격한다. 현재 empty→red 직접 전환과 대각선 승격은 교정한다. 기존 whiteMask는
일반 해골 표시 존재, redMask는 그 subset인 wire 계약을 유지한다. 내부 consumedLineMask와
threshold 단위 소비를 추가해 보상에 쓴 빨간 줄을 다시 세지 않으며 부족한 소비는 상태를 바꾸지 않는다.
최종 threshold는1이며 같은 폭발에서 완성된 여러 새 줄도 모두 사용 처리하고 같은 시점에
한 번만30초를 적용한다. 다른 새 줄이 완성되면 그 시점부터30초로 갱신한다.
실제 폭탄 폭발 직후 pinned special Parent의 기존 BINGO_COMPLETED_LINES threshold와
OnSuccess PLAYER_INVULNERABILITY30000을 소비한다. 기존 third-bomb Parent가 같은 보상을
중복 적용해 시간을 연장하지 않도록 ledger/Duration의 보상 상태를 연결한다.

이난나 소환 승인이 성공하면 기존 iInvulnerableEndTick를30초로 max 갱신해 일반피해와
빙고 전멸에 사용한다. Bingo detonation의 성공·실패 모두 유효 무적만 존중하고 전역 damage arbiter와
다른 encounter wipe는 변경하지 않는다. 새 Shared protocol/별도 buff runtime은 만들지 않는다.
기존 Server Bingo/Esther 계약에서 반전·줄 승격·소비·30초 만료·실패 경계를 검증하고,
변경 Server TU와 소비자를 정상 증분 Product 빌드한다. 최신 Data2356의 전용 Logic525 threshold와
표시 이름만1줄에 맞춰2357로 저장·게시하고 무관한 데이터와 시각 효과 정본은 보존한다.

폭탄은 중심과 상하좌우의 최대5칸이며 판 밖의 칸은 제외한다. 영향받는 모든 nonred 칸의
반전을 먼저 완료한 뒤 한 번만 줄을 승격해 중간 상태에서 잘못된 빙고가 생기지 않게 한다.
사용자 예시인 a2~a5 검정에서 a1 폭발→a1 검정/a2 빈칸/빨간 줄 없음, 이어 b2 폭발→a2 검정/
a행 전체 빨강을 직접 회귀에 고정한다.

## G16. 앵콜의 플레이어 시점 고정과 화면 앞 배우 재배치

P97은 `kouku.bingo.encore.camera.1`의 절대 CameraTrack과 별도 WORLD 배우
`world.sequence.instance.kouku.bingo.encore.saydon`을 같은 원본 공간에서 재생한다.
현재 Level은 진입 시점을 캡처한 뒤 매 frame 절대 shot으로 덮어써 다른 장소로 카메라가 이동한다.
카메라만 중지하면 원본 배우가 기존 장소에 남으므로 카메라와 배우를 같은 변경으로 연결한다.

제품 앵콜은 각 Client의 진입 eye/look/up/FOV를 고정한다. 배우의 기존 샘플 world에
`ViewAuth(t) * Scale(tan(FovHeld/2)/tan(FovAuth(t)/2), 같은 값, 1) * CameraWorldHeld`를
후합성하여 원본 화면상의 접근·동작·크기를 유지한다. 원본 애니메이션과 카메라 데이터는 보존하고
Server actor/gameplay 좌표는 바꾸지 않는다. 화면 기준 유리 Effect는 기존 camera anchor를 유지한다.

`ValtanCinematicCameraController.h/.cpp`에 검증된 두 pose 사이의 presentation 재배치 계산을
추가하고 `WorldSequencePlayer.h/_Objects.cpp`의 optional post-transform callback을 기존
Object sample 마지막 단계에만 연결한다. Level_KakulSaydonArena가 Encore stable ID와 제품
실행 수명을 소유하며 일반 시퀀스·원본 authoring preview는 기존 경로를 유지한다. 첫 frame,
seek/restart·자연 종료·중단·F6·disconnect에서 캡처와 override의 수명이 일치해야 한다.

기존 Camera 계약 검증 경로에서 서로 다른 플레이어 pose/FOV/roll과 원본 track 전체를 대조해
화면 투영 보존·고정 view·잘못된 pose 거부를 확인한다. 기존 C++ 파일만 수정하므로 프로젝트
등록은 추가하지 않는다. 정상 증분 Product 컴파일·링크와 기존 빙고 전환 계약을 유지하며,
실제 Client 화면은 사용자가 직접 확인한다.

실제 Server가 pin한 앵콜은 Sequence composition의 P10/revision171이며 Action P97은 같은
연출의 복사본이다. Server CINEMATIC 시작 전에 view를 캡처하고 카메라 행18333ms가 끝나도
전체 Sequence21322ms의 종료·빙고 gate commit까지 유지한다. BINGO 최종 엔딩에는 적용하지 않는다.

추가 실측에서14개 부착 V1 track 중 일부 spark가 WORLD simulation을 사용한다. birth root에만
보정을 넣으면 수명2~3초의 기존 파편이 과거 카메라 기준에 남는다. Effect_PresentationService,
Effect_Object/Effect_Playback의 기존 CPU evaluated-frame 경로에 optional presentation post를
연결하여 simulation은 원본 좌표를 유지하고 매 frame 최종 draw 좌표에 현재 보정을 적용한다.
모델과 모든 파편이 같은 현재 카메라 기준을 사용하며 asset·shader·Server 좌표는 변경하지 않는다.

## G17. 카드미로 평타 쿨타임의 HUD 슬롯 격리

카드미로 평타는 typed W wire slot을 재사용하지만 실제 키보드 W는 막혀 있다. Server는
평타 전용 reserved cooldown ID에400ms만 기록한다. MainApp의 아이콘·원형 표시는 Q만
표시하는 반면 숫자 텍스트 루프가 같은 필터를 누락해 평타의400ms를 W의1초로 표시한다.
CombatHUDViewModel의 공통 visible-slot 정책을 두 소비자가 사용하여 MAZE에서 Q만 표시한다.
일반 class HUD의 W는 보존한다. 기존 ClientPresentationPrimitiveContractTests에 정책 회귀,
ServerGameplayContractTests_CardMaze의 실제 명령 fixture에 연속 평타가 Q·일반 W cooldown을
변경하지 않는 검사를 추가한다. 제품 Server/Shared/Data 변경이나 새 파일 등록은 필요 없다.

사용자의 추가 패턴 요청은 파랑 정확히1명·빨강 정확히2명일 때만 보호, 기분나빠 및 빙고
복제 패턴의 불뿜기 기준 지속 피해, 무지개 댄스 초반 정상 콜라이더 기준 전체 피해·넉백과
사운드 누락 복원, 십자 화염 폭발의 기존 Effect 연결이다. 해당 데이터는 실제 저장본과
소비자를 조사한 후 아래에 정확한 stable ID·필드 변경 범위를 기록하고 원자 병합한다.

## G18. 파1빨2 영역별 정확 인원 무적

P11의 Logic7~10은 모두 INVULNERABILITY_ZONE Logic101을 사용하며 현재는 Contains만
검사해 인원 제한이 없다. presentation24/26/27은 파랑(Logic7/9/10),25는 빨강(Logic8)이다.
V2 원형 decal은 asset 내부에 Z+2.5m offset이 있으므로 현재 Collider의 저장 중심·반경2m를
Effect occurrence 원점으로 옮기지 않는다. Logic101의 threshold를1로 설정하고 빨강 전용
동일 kind 정의를 threshold2로 추가하여 Logic8만 연결한다. 기존 위치·시각·gaze 판정은 유지한다.

기존 JSON/Client/Server iThreshold 필드를 재사용한다. INVULNERABILITY_ZONE은0일 때
기존 인원 제한 없음,1~4는 각 연결 Collider별 정확한 생존 Is_Judgeable 인원이다. Client
문서 validation/serialization과 Workbench 편집, Python projection/validation, 필요한 PS와
Server catalog validation을 같은 계약으로 확장한다. read codec·bootstrap row16·protocol은
기존 필드를 그대로 쓴다. 새 파일·프로젝트 등록은 없다.

Server protection prepass는 각 활성 영역의 생존 플레이어 점유를 먼저 모은 뒤 정확한 수가
일치한 영역의 플레이어만 해당 pattern 실행의 보호 집합과 기존 보호 pulse에 등록한다.
인원 부족·초과·퇴장·사망을 매 tick 다시 판단하며 별도의 지속 무적을 부여하거나 기존
이난나·빙고 iInvulnerableEndTick를 지우지 않는다. 같은 원의 부족 인원을 다른 원과 합치지 않는다.
기존 actual LogicRuntime fixture로 파랑0/1/2,빨강0/1/2/3,사망·영역이탈·보호범위와
기존 threshold0·다른 pattern 독립성을 확인한다. Data 원자 병합은 root 단독으로 수행한다.

## G19. 무지개 전 구간과 분신·십자 불의 저장 연결

P38 무지개 Effect 두 개는 각45도5줄+135도5줄을 생성해 총20줄인데 Collider는10개다.
첫 Effect의 초반5개에 저장된 피해10%·수평4m·상승2m·강제 ballistic Result538과 Collider
두께·길이·정상 사용자 배치를 기준으로 누락 family를 보완한다. source-wave 시작은 첫 Effect
5159~5959/6859~7659ms, 두 번째10700~11500/12400~13200ms이며 각1초 접촉창을
실제 발생에 연결한다. 기존 이펙트의 방향·크기를 바꾸지 않는다. 사운드는 기존 pattern 및
동일 원본 이벤트의 실물 리소스와 타이밍을 확인하고 누락 연결만 복원한다.

기분나빠 P50/53/54/55는 Effect3203~7482ms 동안 불이 남지만 Collider/ENTER_AREA가
첫1초에 끝난다. 해당 source Effect 생존시간과 기존 불뿜기의 반복접촉/피해 계약을 사용해
전체 지속창을 연결한다. Bingo P108은 같은 애니·Effect만 있고 Collider/Logic이 없으므로
같은 기준으로 연결한다. 십자 화염의 Bingo P112도 Sound만 있어, 기존G3 P40 Effect/Collider를
동일 actor·애니·좌표 공간을 검증한 뒤 복원하고 불은 현재 불뿜기와 같은 원본 자산/지속 피해를
사용한다. 다른 사용자 occurrence 배치·FXAA·WORLD 및 G12~G15 값은 보존한다.

변경은 fresh source의 stable ID와 해당 필드만 병합한다. 의미상 변경 목록·새 ID·연결 수·창
시각을 검증하고 backup·revision/hash 재확인·원자 교체 후 공식 Composition/Gameplay publisher를
한 번씩 실행한다. 게시 Encounter와 실제 bootstrap까지 Collider/Logic/Result를 대조하고
관련 기존 접촉 Runtime 회귀·정상 증분 Product Build를 수행한다. Client화면은 사용자 확인이다.

## G20. WORLD 본 Collider의 같은 계산 재사용

사용자는 G16~G19 실행본을 먼저 준비하고 Publish 병목도 이어서 줄이도록 요청했다.2358의
공식 run(root,publish)를변경없는wrapper로한번측정한결과464.356초중369.431초(79.6%)가
_project_world_bone_collider_track의240회호출이었다. 문서validation은5.451초이며기존
BOSS bone과object collider에는session memo가있다. WORLD bone만매번pose cache를
비우고같은고정tick좌표를다시만들고있었다. 측정정본은out/KoukuPublishProfile20260925/
profile-publish-20260925T134940434596Z.{json,txt}다.

project_kouku_saydon_composition.py의기존함수를thinmemo wrapper와원래계산body로분리한다.
기존_publication_memo를재사용하고root·고정된WorldSequence문서내용·world정의/occurrence/
Collider전체값을key로한다. 외부의native model은기존publication input snapshot/hash pin과
최종freshness guard안에서만재사용한다. mutable standalone입력은값변경을놓치지않는다.
반환값은deepcopy로격리하고실패는캐시에남기지않는다. 프로세스간영속cache·검증생략·새
publisher경로·관문데이터물리분할은이최소수정에추가하지않는다.

test_project_kouku_saydon_composition.py의기존테스트에같은geometry의중복build방지,
시간/위치/회전/scale/bone/world/sequence변경시miss,호출자반환값변경격리,실패재시도,
동일metadata의native내용변경최종거절을연결한다. 기존모든parent/closure/followup/공용Logic
검사와전체출력admission·원자publish·rollback은그대로수행한다. source2358게시와Gameplay
게시가끝난뒤에만제품Python을변경하며새게시의두제품출력이이전게시와byte동일한지대조한다.
동일wrapper로시간·memo hit/miss와worldbone실제build횟수를다시측정해속도향상을확인한다.

## G22. 3관문 LUT01 연결만 복구

사용자는 LUT01을 먼저 복구해 비교한 뒤 환경광을 판단하도록 명시했다. 최신 RenderingProfiles
저장본의 stable profile ID와 kouku.ps.environment.48 region ID로 과거 rev80의 LUT 연결 네 곳만
복구한다. compare.gate3의 qualityOverride와 g1.base/g3.dark/source-rendering의 region48에
Map/Lighting/KoukuSaydon/lv_lut_midnightc_lut_01.dds를 연결하고 document revision만 증가시킨다.
노출·환경광·직접광·Bloom·Mario FXAA 및 다른 사용자 튜닝은 변경하지 않는다.

fresh source/runtime hash를 기록하고 후보를 공식 Rendering publisher로 검증한다. 최신 hash
재확인, 백업과 원자 교체로 source를 반영하고 공식 publisher로 runtime을 게시한다. 실패하면
자신이 쓴 바이트가 유지된 파일만 복구하며 동시 저장을 덮어쓰지 않는다. 최종 source/runtime
의미 일치와 다섯 scalar 변경만 존재하는지 확인한다. 데이터만 변경하므로 EXE 빌드는 필요 없다.
실행 중 Client의 Reload Runtime과 LUT 비교 화면은 사용자가 직접 확인한다. 이동 시 외곽
현상은 별도 읽기 조사로 분리하고 이번 LUT 비교에 피격·호버 shader 변경을 섞지 않는다.

## G23. 세이튼 호버 판정과 외곽선

09-26 사용자는 이동 중 깜빡임이 노란 피격이 아니라 호버임을 확인했다. 현재 전체 pose AABB
피킹은 몸 사이 빈 공간도 선택하고, mesh별 기본 draw 직후의 depth-write 없는 hull 외곽선은
뒤의 바닥 draw에 덮인다. Mesh/Model의 공유 CPU geometry와 현재 skin palette로 삼각형
교차를 확인하고 실제 표시되는 body/weapon/hat만 후보로 사용한다. CNpc hover는 기존
DEFERRED_OVERLAY 단계에서 visible silhouette stencil과 4px screen offset으로 그린다.
기존 depth·다른 stencil bit·노란 hit·LUT85와 모든 렌더 옵션을 보존한다. Product build와
실제 모델 ray 및 GPU 수치 검사를 수행하며 최종 화면 판정은 사용자가 한다.

## G24. Bingo Board 크기 저장과 폭탄·망치 표시

F1 Bingo Board 버튼 아래 Bomb Size, Hammer Size와 Save를 기존 WorldObjectTool 문서/저장/
WorldSequences publisher에 연결한다. 실제 planted 폭탄은 bingo_bomb.original, 망치는
bingo_hammer resource scale을 소비한다. 별도 개인 설정이나 두 번째 저장 정본은 만들지 않는다.
크기는 presentation 배율이며 폭탄의 다섯 칸 판정은 유지한다. 망치 크기를 바꿔도 고정 보드
선행 경고의 위치·범위를 함께 확대하지 않도록 해당 effect offset/scale을 역보정한다.
현재 경고는9m root에 Y=-5.23666668과 object scale1.8이 합쳐져 바닥 아래로 내려가므로
실제 바닥/원본 emitter 높이를 대조한 뒤 해당 경고 occurrence만 올린다.

머리 위 폭탄과 planted 모델·심지의 실제 renderer/수명/본 연결을 검증하고 확인된 실패만
수정한다. 현재 planted WORLD3000ms와 Server fuse4000ms 불일치는 Server 시계로 맞춘다.
새 모델 런타임을 만들지 않고 기존 CModel/WorldSequence를 사용한다. latest stable-ID 데이터
병합·백업·원자 교체·정식 domain 게시와 JSON/C++ 검증을 수행한다.

## G25. 빙고 보호 표시와 마지막 클리어 순서

블랙홀 전멸의 보호 분기에서 기존 invulnerability pulse/contact를 폭발 tick에 게시하여
파1빨2와 같은 Client 무적 폰트 경로를 연결한다. 보호 만료·사망·비참가자 경계를 유지한다.
빙고 실제 balance overlay와 bootstrap/HUD의77줄 연결을 대조한다. 정상 Raid의 P9 final
Sequence→관문 clear→MVP 수명을 보존하고 F1 단독 보스 처치의 즉시 clear 우회를 교정한다.
12,846ms 시퀀스는 사용자 대상 확인과 실제 원본/WANM segment 대조 후에만 변경하며,
원본 배우 다섯 개와 통짜 Sound를 가진 WORLD 행 전체를 임의 복제하지 않는다.

사용자 확인 대상은 최종 클리어 P9다. 원본 A/B/C 혼합·얼굴 제어가 구워진 WANM을 유지하고
4개 배우의 animationTracks를 source clock이 연속인25개 구간(11/4/4/6)으로 나눈다.
시작 시각·sourceStartMs·displayName만 기존 편집 계약에 연결하며 전체49,083ms 재생과
52,042ms Server 종료, 사운드·카메라는 자동으로 늘리지 않는다. Duplicate는 해당 배우의
후속 animationTracks만 이동하며 다른 배우·WORLD transform·사운드까지 ripple하지 않는다.
ClipLoop도 분할 구간의 유한 반복이 아니라 기존 baked clip loop이므로 동일 기능으로 설명하지 않는다.

머리 위 폭탄은 marked WORLD instance로 planted와 같은 CModel resource와 원본 심지 Effect를
사용한다. Server MARKED carrier의 replicated 위치만 추적하고 PLANTED·사망·퇴장 때 해제한다.
준비 완료 전에 모델을 한 번 만들고 포기하던 경로 대신 기존 준비/실패 재시도 계약을 사용한다.

## G26. 실제 WORLD 클립 편집과 손 뻗기 경계 교정

G25는 기존49초 baked WANM의 시각 구간만 나눴다. 사용자가 요구한 원본 클립 교체/trim 편집과
13초 경계 연속성 검사가 빠졌다. 원본 SCENE01B의 B 키는12966.862ms에서 같은12초
evt2_atpain01의 source7633.528→7009.368ms로624.160ms 되감긴다. 현재baker는 새 키를
바로 선택하고 installed WANM의389→390 frame 오른손회전은15.718도다. 원작화면까지
같이 끊겼다고 단정하지 않는다.

World 배우·placement·Motion 소유는 유지한다. WORLD 상세 위 Edit Animation Clips 및
Animation 행 진입으로 기존 Object owner의 실제 모델 clip catalog를 연다. clipName을 직접
교체하고 sourceStartMs/sourceEndMs, 배속·범위반복·timeline 경계·분할/복제를 편집한다.
sourceEndMs=0은 기존 native 끝 의미로 생략 호환하고, 명시 끝은 시작보다 커야 한다.
codec/Map·Composition publisher/Client sample/본 Collider 투영이 같은 범위를 소비한다.
loop는 선택한 source 구간만 반복하며 hold는 그 구간 마지막 pose를 사용한다. 임의 전체
Character 이전이나 두 번째 renderer, global camera/audio ripple을 추가하지 않는다.

손 뻗기 교정은12,900~16,400ms만 독립3,500ms clip 후보로 만든다. 기존 source clock의
불연속을 양끝 pose/속도가 연결되는 단조 clock으로 바꾸고 원본 A/B/C 및 얼굴·머리·팔
SkelControl을 같은 장면 시각으로 보존한다. 새 clip만 설치 WModel 후보에 추가하고 기존
geometry/rest/material/모든 기존 clip bytes를 보존한다. 총 P9 길이와 사운드·카메라·다른 배우는
유지한다. 반복 횟수는 사용자가 source 범위와 timeline window로 조절하며 자동 연장하지 않는다.

실행 중 편집은 유지하고 out 후보와 최소 compile/구조·범위 샘플/경계 bone 변위 검증을 먼저
완료한다. 최신 디스크 stable 필드·hash를 재확인해 승인된 시점에만 source/Resources를
백업·원자교체하고 정식 domain 게시한다. 실제 EXE 잠금은 마지막 제품 링크 단계에서만 처리한다.

## G27. 종속 Parent 시간 편집과 전방 분신의 개별 회전

09-26 사용자 편집 중 기분나빠와 알비온 Stage 변경이 `Looping Parent needs ordered complete
independent children on the same boss`로 거절됐다. 실제 rev2362에서 P52는13009ms이며
순차 Parent P92.pattern.8도13009ms를 별도로 저장한다. Set_PatternDuration와 Stage 변경은
자식만 수정하고 Commit_Candidate는 전체 문서를 검사하므로 완전재생 참조의 시간이 낡아
편집이 실패한다. 이는 publish 실행 전 오류이며 publish cache 최적화가 원인으로 입증된
상태가 아니다. 순차 Parent도 Looping Parent라고 표시하는 진단 역시 수정한다.

기존 child의 실제 lifetime이 바뀌면 같은 candidate 안에서 그 child를 완전재생하는 순차/
loop Parent의 occurrence duration과 뒤쪽 행 시작, Parent 종료를 갱신한다. 의도적으로
고정한 일반 Parent window와 repeat 행 및 무관한 패턴은 변경하지 않는다. common lane과
board의 종료 계약도 실제 소비 기준으로 확인하며, 충돌·범위 초과는 원래 draft를 보존한다.
검사를 삭제하거나 부분 저장으로 모순을 게시하지 않는다. 실제 편집 함수와 문서 codec을
통해 기분나빠/알비온 축소·증가, 반복 Parent, 실패 보존을 검증한다.

전방_ P50은 CROSS_DIRECTION_CLONES의 자식이다. 현재 자식 Logic은 접촉 판정만 허용하고
Server 역시 접촉만 실행하므로 회전 guard만 제거해서는 동작하지 않는다. 사용자가 선택한
동작은 전방 배우 한 명만 공격 시작에 자기 위치에서 가장 가까운 플레이어를 향해 회전하는
것이다. 본체인지 분신인지와 무관하게 해당 배우의 Server yaw만 변경하고 다른 세 방향과
Parent 기준축은 보존한다. 기존 Logic/Server authority/복제/Client preview 경로를 확장하고
실제 소비 테스트와 최소 컴파일 후 최신 저장본을 기준으로 해당 필드만 반영한다.

## G28. 최신 칼날·아이언 메이든 저작값과 실제 판정 연결

사용자는 일반 칼날 Effect X offset=-0.42, 보스에서 아이언 메이든으로 가는 즉사칼날의
전체 scale=(2,2,2), 세이튼 정면의 아이언 메이든 배치와 같은 플레이어 포박 위치를 요청했다.
마지막 Save는 진행 중인 publish 때문에 거절됐다고 정정했고 EXE 종료 및 명시한 값의 반영을
승인했다. 이전 미저장 메모리를 추정 복원하지 않고 최신 디스크와 이 명시 필드를 병합한다.

WORLD2268에서 즉사 모션의 첫 key만2이고64ms 다음 key는1.0002023으로 남아 재생 직후
크기가 돌아간다. 해당 모션의 모든 scale key만2로 맞추며 일반칼날의 다른 scale 곡선은
보존한다. 일반 Effect offset은 이미저장된-.419999987을 요청값-.42로 맞춘다. IronMaiden은
WORLD resource가 아니라P33.presentation.2의 MAP V1 Effect이며 해당 위치와Logic89의
플레이어 포박 위치가1.67m 다르다. BossLogic90 FACE_CENTER의 실제 +X 앞 축을 사용해
현재 보스→cage 거리를 유지한 정면 위치에 표현·포박·Mario 복귀점을 일치시킨다.

현재 P33.world.5와 사용자 추가 world.7은 시작 위치·회전·시각이 다른 두 즉사칼날이며
같은 Motion을 공유한다. 하나를 임의 삭제하지 않는다. 각 시작에서 같은 cage에 도달하는
끝점을 기존 WORLD 모션/Collider 투영으로 만들고, 공유 경로의 서로 다른 배치를 분리할
필요가 있으면 기존 Object의 별도 Motion으로 연결한다. 실제 최종 TRS와 Server hit shape를
대조하고 포박 해제 성공 시 중지, 도달 접촉 때 실패 플레이어 사망을 기존 Server 권위로
검증한다. 별도 Client 사망 판정이나 하드코딩한 두 번째 이동 런타임은 만들지 않는다.

종료된 Client가 시작한 Kouku domain publisher가 별도 프로세스로 남아 있으므로 그 완료를
확인한 뒤 최신 문서 hash를 다시 읽고 백업·원자 교체·공식 domain publish를 진행한다.
G26 손 뻗기와 이번 WORLD 필드가 같은 문서에 있으므로 하나의 최신 candidate에 병합한다.


G26~G28 구현·설치·게시·빌드의 실제 완료 상태와 검증 경계는 같은 날짜 RESULT의
`G26~G28 최종 설치·게시·제품 검증`을 따른다. 사용자 Client 화면 확인은 별도다.


## G29. 세이튼 등장 안내 음성 원본 연결

사용자는 세이튼 등장 시 “여러분들, 뿅망치 살인마가 등장합니다. 과연 누가 살아남을까요?”가
동시 재생에서도 들리지 않는다고 보고했고 원본 음성 추가를 승인했다. 현재 Action의 P9에는
해당 자막과 spawn/idle SFX만 있으며 P8의 castvox/shotvox는 다른 기합이다. 원본 SCENE02A의
subtitle cin.37081_29_01와 sound event scene_midnightc_ed_appeargiantsaton을 대조한 뒤
Wwise bank2755353966/event3575728479의 Korean media120512111을 추출했다.
11.552초 PCM 음성의 자동 전사에서 두 문장이 일치한다. media828459866은 별도 SFX층이다.
최근 변경으로 삭제됐는지는 입증하지 않았으며 기존99665081 출현 SFX를 대사로 대체하지 않는다.

정확한 원본 WAV를 Resources/Sound/KoukuSaton에 설치하고 CharacterSoundCatalog의
KoukuSaydon event에 단일 Korean variant로 등록한다. 현재 저장본의 P9에 독립 SOUND 행을
추가하며 원본 시작50ms와 전체11553ms(PCM11,552.018ms의 올림)를 보장한다. 종료11603ms가 기존 pattern 수명을
넘으므로 optional durationMs만 확장해 기존 animation/FX/camera/subtitle 시각을 보존한다.
동시 bundle의 저작/Preview 길이는 기존 두 배우를 유지하며 더 긴 P9 lifetime을 소비한다.
Server Flow는 실제 Stage 완료 receipt로 다음 entry를 진행하고, 기존 natural-completion
presentation tail이 재생 중인 대사를 끝까지 유지한다. Flow 진행을1653ms 늦추는 수정은
아니다. 새로운 C++/Server 판정 경로나 프로젝트 파일은 필요하지 않다.

최신 source hash/revision을 재확인하고 필드 병합·백업·원자 교체한다. 실제 media 길이와
소리 유무, 전체 document 및 게시 projection, bundle lifetime과 실제 SOUND consumer의
event resolve/재생 범위를 확인하고 공식 Kouku/Gameplay publish로 반영한다. 실행 중
Client/Server의 메모리 상태와 설치 파일을 구분하고 사용자 편집을 강제 Reload하지 않는다.

## G30. 쇼타임 추적 종료의 폭탄 판정과 사각 예고 고정

P35의 네 BOSS_TRACK_TARGET은 회전만 소유하며 노란 장판을 생성하지 않는다. 별도 Effect를
삭제하지 않고 유지한다. 폭탄 본체/심지는 단일 V1 Effect이며 파란 폭발과 편집 그룹을 이룬다.
기존 OBJECT_OVERLAP은 static WORLD 모델을 요구하므로 이번 V1을 새 모델 경로로 옮기지 않는다.

BOSS_TRACK_TARGET Duration에 선택적 bombPresentationOccurrenceId,
bombExplosionPresentationOccurrenceId, bombSectorPresentationOccurrenceId 및
bombSectorRadiusM/bombSectorHalfAngleDegrees를 추가한다. 세 occurrence는 같은 패턴의 고정
MAP 본체/폭발 및 따라가는 BOSS 부채꼴 Effect를 가리킨다. 누락된 참조나 중복 소유, 다른
mechanic의 값 혼용을 거절한다. 네 occurrence는 개별 Logic 정의로 분리해 독립 편집한다.

원본 부채꼴은 반경11m/전체45도다. shader local -Z와 source decal roll180을 상쇄한
semantic +Z를 사용하고 저작 yaw90 및 비균등scale2.2/1.25를 보존한다. 게시 단계에서
BOSS_CURRENT 타원 부채꼴의 radiusX24.2/radiusZ13.75/half22.5와 local offset/yaw로 전달한다.
현재 표시는 원형 반경이나 육안 각도만으로 근사하지 않는다.

Python projector는 본체와 폭발을 기존 finite targeted CombatObject visual로 투영하고 원래
무조건 재생하던 두 occurrence를 제품 정적 lane에서 제외한다. 서버 supplemental
PATTERNTRACKBOMB는 원래 body birth, 두 visual ID, 폭발 수명, MAP 위치와 타원 부채꼴을
기존 tracking trigger에 exact join한다. Update_KoukuPlayerTargets가 본체를 원래 시각에
생성하고 추적 [start,end)의 마지막 방향으로 종료 시 한 번 판정한다. 안이면 본체/심지를
제거하고 밖이면 같은 위치의 원본 파란 폭발과 참가자 전멸을 확정한다. 기존 wire의
spawn/despawn/snapshot을 재사용하며 Stop/재시작은 정리하고 자연 종료의 폭발 tail은 유지한다.

사용자가 해제한 사각 예고/폭발20개 행의 그룹은 재생성하지 않는다. 기존
anchorPresentationOccurrenceId의 고정 BOSS birth 공유를 Effect에도 확장하여 예고 생성 시
보스 기준 위치를 world에 고정하고 짝인 폭발이 같은 birth anchor를 소비하게 한다. 원래 MAP
예고인 한 쌍은 그 저장 위치를 보존한다. 부채꼴 추적과 사각형 고정을 혼동하지 않는다.

변경 파일은 기존 Client CompositionDocument/ActionWorkbench/PresentationPlayer,
Python projector, PowerShell bootstrap writer와 Server Catalog/Brain/LogicRuntime/
GameRoom_BossSimulation 및 대응 focused fixture다. 새 C++ 파일이나 프로젝트 등록은 없다.
사용자 편집 중에는 후보를 준비하고 컴파일·저장/재열기·실패보존·게시 projection 및 네 번의
안/밖/종료시각/무적·실드/Stop 회귀를 확인한다. 최신 저장본 병합은 백업과 hash 재확인·원자
교체를 사용하며 현재 미저장 쇼타임 편집의 적용 기준을 최종 교체 때 확인한다.

## G31. 저장된 빙고 레이저의 판정과 3관문 반복 연결

사용자가 빙고 레이저 편집을 저장하고 Client/Server 종료를 확인했다. 최신 Action2385의
P123 앵콜세이튼 레이저와 동일 Effect를 쓰는 기존 GATE3 P62를 기준으로 실제 원본 beam의
폭·길이·방향·수명과 현재 collider를 대조한다. 표시 Effect TRS와 사용자가 저장한 sound,
stage 길이는 유지하고 collider 및 연결 Logic만 맞춘다. 바람 방구의 기존 대미지·날아가는
Result 설정을 재사용하며 사용자에게 요청받지 않은 대미지나 높이를 추정하여 추가하지 않는다.

GATE3에는 기존 gate/boss binding이 맞는 P62를 여섯 일반 HP 반복 구간에 각각 한 번
연결한다. 현재 기믹의 시작/끝과 HP 임계값을 유지하기 위해 각 일반 구간의 기존 마지막
항목 바로 앞에 stable entry를 넣는다. BINGO P123의 다른 보스 placement를 GATE3에 직접
연결하지 않는다. 기존 빙고 Complete Play의 board-start, 반복 시작, 독립 폭탄/blackhole,
최종 클리어 경로를 코드·게시 데이터와 좁은 실제 Server 실행으로 확인한다.

이 작업과 G30은 사용자 마지막 저장본에 필드 병합한다. 구 실행 Client가 새 G30 필드를
읽지 못했던 중간 반영은 후보로 회수했으며, 사용자의 레이저 Save 완료 후 한 번 재적용한다.
정식 composition/Gameplay publish와 Debug Product 빌드 뒤 실제 설치/게시 revision 및
Server 실행 검증을 기록한다. 사용자 Client 화면은 자율 실행하지 않는다.

## G32. 게시 연출의 빈 선택 참조와 실패 재시도 병목

사용자 단일 Client의 5-frame 캡처에서 Presentation.Prepare가 프레임마다
1219~1417ms이고 CameraShots.Load가208~225ms다. 같은 PID32052 로그는
`Invalid presentation string: anchorPresentationOccurrenceId`를 반복한다.
제품 projector의 선택적 anchor 기본값은 빈 문자열인데 runtime Text reader가
이를 필수 비어 있지 않은 ID로 읽어 전체 Product를 거절한다.

PresentationPlayer의 선택 참조 reader는 누락과 빈 문자열을 동일한 미사용으로
허용하되 타입·길이·NUL 및 실제 참조의 kind/소유/시각 검증은 유지한다.
Run admission이 준비되지 않은 동안에는 카메라 파일을 읽지 않는다. 동일
run/source/draft 준비가 실패하면 실패 상태를 보존하고 매 프레임 전체 Product를
재파싱하지 않는다. 새 run identity 또는 명시적 reload/reset은 다시 시도할 수 있다.
이전 유효 presentation은 로드 실패로 교체하지 않는다.

현재 게시 Product를 실제 native consumer로 검증하고 빈 참조·누락·잘못된
참조와 반복 실패를 좁게 확인한다. Debug Product Build, diff check와 실제
캡처 수치를 RESULT에 기록한다. 사용자 옵션·저작/게시 데이터·그래픽 품질은
변경하지 않으며 FPS 회복 화면 판정은 사용자의 새 실행으로 구분한다.

## G33. 참가자의 진행 중 WORLD와 Complete Play 준비 경합

네 Client 모두 Product2386의115개 패턴을 읽었으나 Player4는 첫 리소스 준비에서
활성 WORLD를 이유로 실패를 회신했다. MainApp은 false를 PREPARE_FAILED로 보내고
Server는 참가자 한 명의 실패로 전체 준비를 취소한다. Level은 Server WORLD 큐를
먼저 재생하고 같은 프레임 후반에 Complete Play 준비를 호출하므로 늦은 참가자는
입장 시 받은 연출과 준비 검사가 겹칠 수 있다. 당시 활성 stable ID는 기록되지 않아
어느 WORLD였는지는 단정하지 않는다.

Level_KakulSaydonArena.cpp의 새 준비 시작 분기에서 활성 WORLD가 있으면 기존
true/ready=false 계약의 대기로 반환한다. 문서 reload, 파일 읽기, cache 해제,
강제 Stop 없이 Level의 기존 Update가 현재 연출을 완료하게 한다. 상태에는 활성
WORLD의 stable ID를 제한된 개수만 표시한다. 재생이 끝난 다음 호출에서 기존 최신
게시본 reload와 리소스 준비·revision 검사를 그대로 수행한다. 무한 재생을 임의로
중단하거나 실제 데이터 오류를 준비 완료로 위장하지 않는다.

기존 C++ 한 파일의 변경이며 새 파일·프로젝트 등록은 없다. 실제 native 준비 함수로
busy 상태의 pending 반환과 상태 보존, 반복 호출, idle 뒤 기존 준비 경로 진입을
확인한다. 실제 EXE 링크는 점유 여부를 확인한 뒤 수행하며 사용자 GUI를 실행하지 않는다.

## G34. 빙고 종료 손 뻗기의 원작 클립 선택지

사용자가 지목한 장면은 P9 빙고 종료 후 쿠크·세이튼 엔딩이다. continuous는
유지하고 같은 12900~16400ms 창의 원작 동작을 별도 native clip으로 추가한다.
기존 Character의 kouku.bingo.ending.saydon1에는 원본 Matinee slot 혼합과
SkelControl이 이미 포함되어 있다. 30Hz tick387~492의 P/Q/S key를 시간만
0~105로 바꾸어 복사하며 quaternion 재정규화나 원본 시계 보정을 하지 않는다.

기존 build_bingo_ending_actors.py에 --original-hand-reach 후보 생성 옵션을
추가한다. kouku.bingo.ending.handreach.original은 3.5초,168채널이고 기존
continuous 및 모든 WModel section payload는 그대로 보존한다. 같은 이름에
다른 내용이 있으면 거절하고 같은 내용의 재실행은 추가0개가 되어야 한다.
새 C++ 파일·프로젝트 등록·제품 빌드는 필요하지 않다.

현재 WORLD2277의 handreach는25133ms이며 원래 창보다12233ms 늦다.
Sequence177의 음향·자막·카메라 시각과 원본 SCENE01B event를 대조한다.
이번 추가는 native 선택지만 제공하고 사용자의 저장된 WORLD 시계는 바꾸지
않는다. 후보의 byte 동일성·실제 모델 parser·모든 key와 중간 시각 pose를
검증한 뒤 최신 hash 재확인, 백업, 원자 교체로 요청한 모델 추가를 설치한다.
실행 중 메모리 모델과 디스크 catalog, 실제 화면/청취 판정은 구분한다.

## G35. 원래 WORLD의 손 뻗기 구간 한 번 Duplicate

사용자는 G34의 native 선택지 추가가 아니라 기존 WORLD 복원 후 원래 손 구간을
한 번 Duplicate하라고 정정하고 즉시 반영을 명시했다. 다른 패턴과 게시 결과는
변경하지 않는다. pre-continuous WORLD2268의 saydon1 애니메이션 시간과 원래
transform keys를 복원한 다음 기존 Duplicate_Track과 같은 동작으로12967~16333ms
구간을 바로 뒤에 한 번 복제한다. 다른 배우·카메라·사운드는 유지한다.
현재 source/runtime이 같은 최신2277임을 확인하고 stable template 하나만
필드 병합한다. 공식 Map WorldSequences Validate 후 백업·hash 재확인·원자 교체와
WorldSequences만 Publish하며 Composition/Gameplay publisher는 실행하지 않는다.

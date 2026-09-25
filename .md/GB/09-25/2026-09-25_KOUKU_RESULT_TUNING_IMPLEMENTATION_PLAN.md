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

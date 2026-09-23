# 쿠크 Collider 상세·상승 원통 구현 계획

## G00. 현재 기준과 목표

현재 HEAD는 `ff868f0ade1d5b06dc2a19e9716512e311aab76f`, 브랜치는
`codex/kouku-gate3-bingo-flow-0923`이다. 기존 미커밋 Composition과 다른 작업 문서는 보존한다.
화염파동 연출의 기존 설치 상태는 09-22 FLAME_WAVE PLAN/RESULT를 따른다.
이번 목표는 Collider 편집 UI와 실제 판정의 수명·이동·반복 접촉·로직 연결이다.

## G01. Workbench의 Collider 전용 상세

`KoukuSaydonActionWorkbench.h/.cpp`의 Collider 선택은 Pattern 이름·부모 편집을 건너뛴다.
상단은 1 Lifetime, 2 Motion / shape, 3 Trigger, 4 Logic 선택으로 구성한다.
Start ms, Lifetime ms와 Position offset, Rotation, Scale, Anchor, Bone target, Bone을
유지하고 end position/end size를 추가한다. 이 공통 필드는 1~4 선택에 관계없이 항상 표시한다. Cylinder preset은 기존 생성 경로를 재사용한다.
Apply는 임시 입력 전체를 검증해 commit하고 실패하면 기존 draft와 입력을 보존한다.

## G02. 문서와 게시 계약

`KoukuSaydonCompositionDocument.h/.cpp` occurrence의 STATIC/LINEAR motion은
기존 시작 offset/scale에서 종료 offset/scale까지 수명에 따라 선형 보간한다.
CYLINDER는 반경과 높이가 있는 수직 원통이다. 기존 정적 문서는 같은 기본 동작을 유지한다.
ENTER_AREA의 repeatIntervalMs는 접촉 중 플레이어별 반복 간격이며 0은 기존 정책이다.
MAX_HP_PERCENT_DAMAGE의 pushHeightM는 상승/착지 호의 높이를 설정한다.
기존 Composition projector와 gameplay publisher가 같은 필드를 검증·투영한다.

## G03. 실행 소비자

기존 BOSS_LOGIC_REGION과 KoukuSaydonLogicRuntime에서 보간과 높이 판정을 소비한다.
Client Collider preview도 같은 시간의 위치·크기를 표시한다. Server가 접촉과 피해를
확정하고 기존 knockback snapshot으로 플레이어 비행을 표시한다. 낙사 비허용 비행은
navigation/collision 경계 안으로 제한한다. 카드 병정의 패턴 종료 정리 조건도 실제
summon owner와 room 정리 경계를 조사해 요청한 지속 수명에 맞춘다.

## G04. 검증

기존 focused contract에 원통 높이·시작/중간/끝 보간·플레이어별 반복 틱을 추가한다.
JSON 왕복과 잘못된 입력의 보존, publisher 투영을 검사하고 Debug Product Build와
`git diff --check`를 수행한다. 기존 H/CPP를 확장하므로 제품 프로젝트 신규 등록은 없다.
Client/UI 실행과 최종 화면 판정은 사용자가 수행하며 RESULT에 별도로 남긴다.

## G05. 상세 회귀 수정과 복제

Lifetime/transform 필드를 section 조건 밖으로 이동하고 Collider preset 7개와 Timeline lane 6개의
반복문을 구분한다. Box Detail에 Set Group/Ungroup과 Duplicate를 표시한다. 그룹은 기존
동일 BOSS 기준 Collider 2개 이상 조건을 유지한다. 동일 시각 복제는 기존 timeline clone의
stable ID·Logic 소유 연결 재매핑을 사용하며 새 Collider만 선택해 바로 편집한다.
단일 Duplicate는 현재 상세값 Apply가 성공한 뒤 실행한다. 그룹 복제는 staged geometry를
보존하며 원본 그룹과 독립된 새 group ID를 발급한다.

## G06. 실제 화염파동과 개별 그룹 편집

P58의 현재 두 Effect occurrence와 실제 2/3/4/5열 element 배치를 읽어 28개 정적 Cylinder,
8개 행 그룹과 ENTER_AREA 창을 생성한다. 각 행은 Once를 공유해 경계에서 중복 타격하지 않는다.
기존 사용자 BOX는 보존한다. Collider의 anchorPresentationOccurrenceId는 같은 Pattern의
fixed BOSS Effect를 가리키며, projector가 그 start를 captureStartMs로 Server에 전달한다.
Server는 Effect 생성 때의 boss basis를 미리 고정하고 이후 각 행의 접촉 창을 켠다.
그룹 상세의 Edit member와 Back to Group으로 원래 group ID를 유지하며 개별 상세를 연다.

## G07. 저장하지 않은 편집값의 Server 미리보기

Workbench Play Preview와 Play Pattern은 Apply된 현재 메모리 draft 및 staged geometry를
불변 snapshot으로 복사한다. 별도 숨김 준비 프로세스가 기존 projector와 같은 Kouku row emitter로
임시 encounter/presentation/gameplay rows를 만든다. authoring과 설치 데이터는 수정하지 않는다.
기존 Gate 승인과 resource 준비 후 raw Kouku rows를 제한된 chunk로 Server에 전송한다.
sourceRevision 외 SHA256과 request sequence로 서로 다른 미저장 편집값을 구분한다.
Server는 현재 non-Kouku 데이터와 합친 후보를 기존 parser로 검증하고 run에만 pin한다.
피해·반복 접촉·상승·착지·root motion은 기존 GameRoom fixed tick 소비자가 그대로 처리한다.
새 프로토콜은 Debug audition에 한정하고 크기·순서·checksum·world 변경·실패 rollback을 검증한다.

## G08. 같은 실행본의 Client 표현과 검증

Client의 기존 presentation/animation binding parser는 임시 JSON도 받아 stage한다.
Server가 승인한 hash와 run epoch가 일치할 때만 해당 표현을 활성화한다. 같은 sourceRevision만으로
캐시를 재사용하지 않는다. Sound는 같은 Server 시계의 presentation lane으로 시작·정리한다.
정식 실행은 설치된 Product를 사용하고 임시 실행은 정식 Publish generation을 바꾸지 않는다.
원본 emitter와 임시 emitter의 row 일치, malformed/reordered/stale chunk 거부, 실제 Server
collider/knockback contract와 변경 C++ 최소 compile/link를 확인한다. Client 화면은 사용자가 확인한다.

## G09. 정식 Publish 반복 비용과 소비자 정합성

단계 로그로 Pattern action, Map action과 fingerprint 비용을 구분한다. 같은 게시 작업의 native
WModel 내용 해시와 동일 후보 검증을 재사용하고 종료 직전에는 실제 내용을 다시 확인한다.
projector가 실제로 읽은 JSON·텍스트·native 파일과 없는 optional 입력, 도구 코드 및 두 출력의
정확한 해시를 검증 증명 파일에 남긴다. 후속 Validate는 이 전체 read set이 그대로일 때만
재투영을 생략하고, 증명이 없거나 손상·변경됐으면 기존 전체 검증으로 돌아간다.
World의 Parent 필드는 Gameplay와 같은 계약 검증을 공유해 새 필드로 인한 rollback을 해소한다.
Map 성공 receipt를 임의 생성하거나 입력 검사를 생략하지 않는다.

## G10. 패턴별 실제 피해·상승·반복 연결

현재 저장본의 팡파레·조커찾기·2/3회 내려치기·노란 장판·레이저·불뿜기를 stable ID로
조사한다. 이미 연결된 접촉 창과 사용자 Effect transform을 보존하고, 빠진 연결과 원본
망치 본의 접촉 위치를 보완한다. 조커찾기의 카드 뒤집기용 본 anchor는 피해용 원통도
공유하며 뒤집기 Result를 변경하지 않는다. 피해 Result는 해당 타격 전용으로 복제해
무관한 패턴의 공유 값을 바꾸지 않는다. 도넛의 innerRadius와 기존 Collider를 재사용한다.

불뿜기는 사용자 확인에 따라 고정 HP 피해500을 적용한다. 초기 반복 간격은500ms이며
Collider 상세에서 편집한다. 기존 RESULT 계약에 FIXED_DAMAGE와 damageAmount를 추가하고
Document → projector → 공유 bootstrap emitter → Server parser → 기존 피해 처리로 연결한다.
Box Detail과 일반 Result 편집에서 비율 피해와 고정 피해를 구분하고 저장 왕복을 보장한다.

주사위의 동일 문양 카드는 피해0, 다른 문양 카드는 최대 HP50%이며 색상은 문양 비교를
대체하지 않는다. 기존 Server 카드 상태·접촉 판정을 사용한다. BGM 전환과 관문별 낙사·부활은
별도 담당 문서에서 원인·검증을 기록하고 통합 빌드와 최신 저장본 병합을 이 작업에서 수행한다.
후보는 out에 준비해 먼저 검사하고 최종 교체에는 최신 디스크 저장본, writer lock, 해시 재확인,
백업과 원자 교체를 사용한다. 실행 중 Client의 미저장 draft를 자동 Reload하지 않는다.

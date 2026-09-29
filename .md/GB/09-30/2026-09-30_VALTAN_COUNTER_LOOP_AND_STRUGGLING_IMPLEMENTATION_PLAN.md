# 발탄 카운터 반복·발악 돌진·부위 파괴 표시 구현 계획

## G00. 현재 기준과 파일 경계

시작 HEAD는 `50dcd986dc8139815b8ad9b5263fe3ec91498eff`, 브랜치는
`codex/release-regression-20260929`다. 사용자와 다른 작업의 미커밋 변경을 보존한다.
Client/UI 실행과 화면 판정은 하지 않는다. 최종 publish와 제품 빌드는 통합 작업이 소유한다.

현재 `VALTAN_TRASH`의 `RETRY_EXHAUSTED`, `CATCH_SLAM`, `EXECUTE_TAIL`은
카운터가 없어도 TIMEOUT 완료된다. 기존 재충전·재조준 stage로 연결하여 살아 있는
대상이 있으면 다시 카운터 기회를 주고, 전멸했으면 기존 targetless 재시도 대기를 사용한다.
성공 완료는 기존 COUNTER_HIT → GROGGY 뒤에만 발생한다. 포획 피해·전원 포획 처형은 보존한다.

## G01. Valtan.gameplay.json과 저작 생성기

위 세 stage의 default/TIMEOUT edge를 `RECHARGE_WAIT_02`로 바꾼다.
기존 helper audition 조각은 자기 안에 존재하는 stage만 참조하며 실제 반복 helper는 같은 loop를 쓴다.
`author_valtan_phase_two_mechanics.py`가 다시 생성해도 같은 계약을 유지한다.

발악 `STEP_01`의 기존 NONE aim/yaw를 유지하고 `STEP_02`의 중앙 보간을 기존 FORWARD 6m/500ms로 바꾼다.
`STEP_03`의 기존 중앙 이동과 이후 발악·죽음·유령 전환은 보존한다.
별도 이동 runtime이나 신규 C++ 파일은 추가하지 않는다.

## G02. 실제 Effect transform과 경고 제거

발탄 body root는 -90도, 발악 root portal의 snapshot source basis도 -90도다.
현재 notify-012의 local `[6,0.5,0]`은 owner 기준 왼쪽 6m가 된다.
동일 basis와 geometry 방향을 유지하고 해당 16개 root emitter의 위치만
`[0,0.5,-6]`로 바꿔 owner 앞 6m에 놓는다. 왼손 notify-011은 보존한다.
발악 underfoot의 `growing-warning` element만 비표시로 바꾼다. 1500ms 폭발·피해·밀치기는 유지한다.

## G03. WorldHealthBarView와 Valtan feedback

녹색 PNG와 성공 text를 reference pixel 기준 위로 올린다. PART_BROKEN 처리에서
같은 snapshot의 실제 제거된 armor mask와 아직 알리지 않은 mask만 파편/text를 발생시킨다.
일반 피해·중복 packet·같은 부위의 새 event sequence는 성공 표시를 재시작하지 않는다.
reset에서 feedback mask와 시계를 함께 비운다. 전투 판정은 Server에 유지한다.

사용자의 후속 위치 요청은 PNG만 현재 위치에서 왼쪽24px·위24px 더 이동한다.
1280×720 reference 좌표에서72×72 PNG 폭의1/3만큼 옮기며, 성공 text와 발생 조건은 유지한다.

## G04. 검증

최신 원본 hash 재확인·백업·원자 교체로 stable ID/필드를 병합한다.
기존 strict join/Product projection과 Trash 회귀를 수정된 계약으로 검사한다.
실제 기존 Server fixture에서 반복 회피/부분 포획 뒤 재진입과 카운터 후 완료를 확인한다.
효과 transform은 owner yaw별로 앞 방향/6m를 수치 검증한다. 화면상 위치·포탈 진입과
경고 제거는 사용자가 새 Server/Client에서 확인한다. project/filter 추가는 없다.

## G05. 편집기 소비자와 공통 무력화 교차 검토

`ActionCompositionGraphModel`과 `ValtanPatternTree::Build_PreviewStagePath`는 동일한 세 retry edge만
인정한다. 그래프에는 실제 화살표를 보존하고 미리보기는 한 회차에서 멈춰 종료와 반복을 구분한다.
기존 native graph 계약에 게시 Trash 로드·반복 경계·카운터 종료·임의 cycle 거절을 함께 검증한다.

공통 무력화 계산은 Valtan/Kouku boss에 한정하고 다른 boss의 authored `iStaggerDamage`는 보존한다.
기존 typed DAMAGE/COUNTER/STAGGER Result는 독립 채널이다. STAGGER는 같은 cast의 독립 subhit ordinal로
HP 차감 전 damage share를 받아 armor 계산 후 /1000을 사용하고 HP를 차감하지 않는다. DAMAGE/COUNTER는
무력화를 중복 지급하지 않는다. 기존 Numeric sandbox에서 40000→41000→40000 Save+Apply를 검증한다.


## G09. 자동 입장의 중복 휠윈드 제거

사용자 G 입장 후 컷씬이 끝나면 기존 legacy 등장 휠윈드와 저장된 health rotation의 첫
일반 휠윈드가 연속으로 실행됐다. 09-29 HEALTH_ROTATION G06과 기존 Lifecycle 기대도
이 이중 경로를 명시하고 있었다. Trash 수정 전후 Brain의 rotation 선택은 같으며 이번
반복 stage 변경으로 입장 순서가 바뀐 것은 아니다.

HEALTH_BAR_ROTATIONS 자동 입장의 authored cinematic 선택에서 cinematic과 legacy intro의
소비 완료를 함께 표시한다. 컷씬 뒤 pending 체력 기믹을 기존 우선순위로 처리하고 정상 체력이면
저장된 rotation[0]부터 시작한다. legacy 진입·명시적 audition·F1 전체재생·체력 window·
추적 시간·패턴 수치와 Trash retry는 변경하지 않는다. Lifecycle의 실제 4인 G 입장에서
CINEMATIC → WHIRLWIND → DASH_CHARGE를 첫 세 occurrence로 검사한다.

## G10. 버러지 반복 snapshot의 Client 수용

서버의 동일 patternSequence 안에서 retry가 이전 stageIndex로 돌아가지만 Client의
Apply_NetworkState는 이를 stale로 거절하고 있었다. 서버 재시도 성공만으로 화면 반복이
검증됐다고 보지 않는다. Client 담당자는 Trash/TrashCatchIf의 실제 반복 계약에 한해
기존 forward tick 판정으로 최신 actionStartTick을 수용하고, 일반 패턴 역행·다른 pattern·
과거 snapshot 거절을 유지한다. 구체 적용과 CPU 검증 결과는 담당자 확인 뒤 RESULT에 기록한다.


## G12. 첫 순환과 부위 파괴 돌 소환 회귀 차단

사용자가 재개한 범위는 자동 컷씬 뒤 첫6개 순환, Trash 카운터 반복, 부위 파괴의 잘못된
돌 소환 제거다. 첫8개 occurrence를 필터 없이 CINEMATIC → WHIRLWIND → DASH_CHARGE →
HIGH_JUMP → FOUR_SLASH → CROSS → DASH_CHARGE → WHIRLWIND로 검사하고 전체 성공 전투에
ENTRANCE_WHIRLWIND가 한 번도 삽입되지 않는지 기존 Lifecycle fixture로 검사한다.

Valtan.gameplay.json의 VALTAN_PART_BREAK/PART_BREAK_RECOVERY에서
valtan.part-break.cardinal-rocks 이벤트만 제거한다. author_valtan_phase_two_mechanics.py의
같은 생성 경로도 제거한다. 회복 애니메이션과 기간, 실제 PART_BROKEN 사건에서 재생되는
420627/420628 갑옷 파편은 유지한다. 해당 회복의 돌 생성 사운드와 생성기 연결도 제거한다.
기존 땅구르기사자후·십자돌 패턴과 공유 이펙트 리소스는 변경하지 않는다.

PNG는 실제 성공 전용으로 바꾸지 않는다. 남은 갑옷이 있는 파괴 가능 창에서 표시하고,
갑옷이 모두 제거되면 다음 돌진/추가 폭탄에서도 표시하지 않는 기존 mask 소비를 확인한다.
실제 파괴 폭탄으로 part state와 armor durability가 함께 갱신되는지 검사하고 불일치가
확인된 경계만 수정한다. 파편은 각 실제 제거 mask에 한 번만 발생해야 한다.

원본 필드별 백업·freshness 확인 뒤 수정하고 split projector와 gameplay publisher로 게시한다.
사용자의 무력화 최대치50000과 무관한 저장값은 보존한다. Debug/Release Product Build와
관련 Server native 검증을 실행하며, Client/UI 화면 검증은 사용자가 수행한다.


G12 추가 확정: 제거한 spawn의 전용 combat object/visual/sound도 함께 제거한다. strict join은
owner 없는 companion을 허용하지 않는다. projector는 managed owner에서 사라진 object를
Product에서 제거하도록 기존 replace/remove 함수를 사용하고 unmanaged legacy는 보존한다.
갑옷은 서버가 생성한 실제 destruction_bomb 명중만 차감한다. 일반 공격 HP·무력화는 유지하며
스킬 번호만 폭탄과 같아도 갑옷을 깎지 않는다. 다른 보스의 기존 partDamage는 유지한다.


## G13. 마력구의 독립 무력화 경로 완결

최종 native 검증에서 VALTAN_STAGGER_SLOT/CHANNEL이 아직 ACCUMULATED_HEALTH_DAMAGE
10000을 사용한다는 누락을 발견했다. 이 HP 누적 response를 제거하고 기존 ENTER/EXIT의
SET_STAGGER_GAUGE, 공통 raidStaggerMaximum, STAGGER_BROKEN과 기존 Groggy follow-up을
사용한다. 실제 HP 피해와 무력화를 분리하며 일반 확정 적중은 기존 raw 피해/1000 경로,
회오리는 게시 item의 최대치/3 경로를 그대로 소비한다. 사용자 저장 최대치50000은 유지한다.
새 패킷, 별도 무력화 계산기 또는 병렬 전투 runtime을 추가하지 않는다.

CHANNEL의 기존 +0.5m 높이와 성공/timeout 복귀는 유지한다. 높이를 HP response에만
허용하던 Server catalog, publisher, split source, Client reference/tree 검증은 ENTER/EXIT가
닫힌 무력화 window도 수용하도록 같은 계약으로 맞춘다. source와 생성 recipe를 함께 고친 뒤
정상 split publish 및 전체 domain publish를 수행한다. 실제 HP0 적중·회오리·공통최대치·성공
한 번·높이 복원·Groggy 전환을 기존 native fixture에서 확인한다.

광역 contract의 과거 저작 상수 실패는 별도 증거로 보존한다. 이번 변경의 무돌 회복·현재 타임라인은
기존 valtan-presentation 검증에 연결하고, 쿠크 난수 피해는 정확한 고정 피해 대신 허용범위와
실제 DamageEvent/HP 일치·발동 횟수를 검사한다. 새 CLI나 별도 제품 우회는 만들지 않는다.

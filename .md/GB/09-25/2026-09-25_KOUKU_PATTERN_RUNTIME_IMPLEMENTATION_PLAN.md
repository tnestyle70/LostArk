# 쿠크 패턴 Server 런타임 구현 계획

## G00. 현재 기준과 소유 경계

현재 codex/kouku-timeline-local-preview의 미커밋 변경을 보존한다. 기존 RAID flow와 Logic 결과 소비자를 확장한다. Client/UI는 실행하지 않는다. 렌더링 설정은 변경하지 않는다.

## G01. Logic 접촉 틱과 소환

KoukuSaydonLogicRuntime의 AREA_OVERLAP duration에 repeatIntervalMs가 있을 때 생존 구간의 접촉을 반복 판정한다. 기존 ENTER_AREA의 반복 피해와 같은 per-player next tick, region transform, result 경로를 쓴다. GameplayCatalog 및 KoukuBootstrapRows가 두 kind의 동일 계약을 검증한다.

CARD_RAIN_SOLDIERS는 CLUB/HEART/DIAMOND 순서의 soldierCounts와 spawnRadiusMinM/MaxM를 새 supplemental bootstrap row로 보존한다. 0..32 개별/총1..64, 반경0..100m/min<=max를 검증하고 전체 nav 위치를 stage한 뒤 기존 Spawn_Monster 경로로 commit한다. 실패는 이전 병정과 다른 패턴을 보존한다.

## G02. 추적 종료와 정면 선택

기존 BOSS_TRACK_TARGET의 양수 followSpeedScale은 플레이어 몸체까지 계속 전진한다. 몸체와 플레이어 반경 및0.5m 여유 거리까지 접근했을 때 현재 추적 패턴을 완료하여 RAID scheduler가 다음 패턴을 시작하게 한다. 회전만 하는 trigger는 가장 가까운 살아 있는 플레이어 방향을 즉시 포착한다.

## G03. 빙고 독립 시계

빙고 판은 encounter epoch와 boss entity를 소유하며 단일 parent pattern sequence에 종속되지 않는다. 첫 표식은 입장10초 유예+20초, 이후20초 간격이다. 표식6초 후 현재 cell을 capture하고2초간 숨긴 뒤 바닥 폭탄을 생성하며4초 후 동일 cell 십자를 뒤집는다. 흰 해골은 지우고 빈 검은 바닥은 빨간 해골로 바꾸며 기존 빨간 해골은 유지한다. 망치와 바닥 광기10%/초 시계는 패턴 교체에도 유지한다.

최신 사용자 순서에 따라 매 세 번째 표식은 이동→블랙홀 첫 clip→메두사→블랙홀13초→폭발의 작은 Parent를 호출한다. 기존 보드 전체 Parent는 사용하지 않는다. 세 번째 폭탄은 정상12초에 터진 뒤 빨간 칸의 완성 행·열 합계를 판정한다. 대각선은 제외한다. BINGO_COMPLETED_LINES Duration의 threshold3을 만족하면 PLAYER_INVULNERABILITY Result의30000ms 보호를 모든 생존자에게 부여한다. 이 버프는 패턴 교체와 현재 tile 위치에 종속되지 않는다. BINGO_DETONATION은24.828초에 판정 성공이면 버프로 플레이어를 보호하며 boss13줄을 차감하고, 실패이면 모든 raid participant에게 보호를 무시하는 encounter wipe를 적용한다. Parent 완료 후 중단된 일반 flow entry부터 다시 재생한다.

## G04. 공 분열과 authored 광기

기존 model-less WORLD group을 각 motion member로 확장하여 기존 collider bake에 전달한다. 별도 Server collider 경로를 만들지 않는다. g0..g3 네 세대만 group에 남겨3회 분열로15개 공을 재생하며 각 impact15회와 첫 bounce1회에 DAMAGE를 연결한다. 원본에 collider 크기와 피해가 없으므로1m half extent와최대HP10%는 PROJECT_TUNED 초기값으로 기록한다. 설치는 source hash와 stable ID를 사용한 후보 병합으로 root가 수행한다.

WORLD에 authoredMadness가 있으면 그 occurrence의 legacy aura를 억제한다. projector는 정확히 일치하는 WORLD anchor, 반복 접촉 결과와 전 생존 구간 coverage를 확인한 경우에만 이 값을 생성한다. 파괴 가능 body와 finite lifetime은 기존 world body ledger를 사용한다. damageable WORLD에 연결된 contact는 ownerWorldOccurrenceId를 보존하며 몸체 파괴 시 동일 epoch/member/pattern sequence의 active/tail ledger에서 해당 occurrence를 종료한다. 플레이어 피해 후 boss Logic 전의 room 경계에서 파괴를 정리하여 같은 틱의 유령 피해도 막는다.

## G05. 검증

기존 H/CPP만 확장하므로 project/filter 신규 등록은 없다. 변경 JSON/publisher parse, 최소 Server Debug/Release 증분 빌드, 빙고·Logic·카드비 관련 실제 Server fixture와 git diff --check를 수행한다. 실행 증거는 RESULT에 별도로 기록한다.

## G06. 방향별 화염과 전투 전환 준비

Cross direction의 real child와 세 clone은 기존 stage root motion으로 움직인 직후 각자의 접촉 Logic ledger를 평가한다. child 계약은 AREA_OVERLAP/ENTER_AREA와 FIXED_DAMAGE/MAX_HP_PERCENT_DAMAGE/MADNESS_GAUGE_ADD_PERCENT 결과에 한정한다. 따라가는 상자의 좌표는 실제 actor를 기준으로 매 틱 계산한다.

G1 book은 준비된 hidden pool clone에 기존 Sample 함수를 사용해 전투 endpoint animation을 미리 평가한다. 이 단계는 Play/Seek, Effect, Sound나 맵 visibility를 commit하지 않는다. pending owner 준비 실패는 해당 owner와 hidden clone만 폐기한다. 기존 F7 capture의 named scope로 준비, activation과 commit을 분리한다. 사용자 화면 측정 전에는 원인 확정이나 멈춤 해결로 기록하지 않는다.

## G07. 게시 문서의 유한 용량

최종 제품은 82,441행이며 WORLD transform key가 73,999행이다. 신규 인형의 전체 생존 구간과 본 회전 샘플 때문에 기존 key 32,567행에서 41,432행 증가했다. 신규 최대는 2,352 keys이며 기존 4,096 per-track 상한 안이다. 1ms 간격의 floor/ceil tick bracket은 보간 경계이므로 단순 중복 제거하지 않는다.

Shared의 제품 bootstrap 상한을 131,072행과 64MiB로 통일한다. publisher는 BOM 없는 UTF-8과 실제 플랫폼 newline을 포함한 직렬화 크기를 검증한다. Server는 읽기 전에 크기를 검사하고 bounded exact read 뒤 Bytes 입력에서도 검사한다. Client와 Server staged admission, Python parser도 같은 상한을 사용한다. Debug draft의 16MiB 상한은 별도 계약으로 유지한다. 필드 검증, per-track 상한, 선언 행수, 후행 행 거부와 실패 시 이전 catalog 보존은 그대로 유지한다.

## G08. 빙고 Flow와 특수 Parent의 명시적 연결

빙고페이즈 진입의 자동 시작, 일반 패턴 반복, 매 세 번째 표식의 특수 Parent 삽입과 완료 후 복귀는 기존 RAID flow가 소유한다. 동시 실행 Bundle의 의미를 바꾸지 않고 BINGO patternFlow에 bingoSpecialPatternId를 저장한다. 게시 데이터에는 정확히 한 RAIDBINGOSPECIAL 행이 있어야 하며 Server는 그 stable ID로 대상을 조회한다.

원본 publisher는 같은 gate와 boss의 실제 Parent 구조와 단일 활성 BINGO_DETONATION을 검증한다. P107은 게시 시 10개 stage로 flatten되므로 Server는 ParentChildren의 존재를 요구하지 않고 finite fixed clock, stage 합계와 단일 폭발 Trigger를 검증한다. 일반 flow 또는 그 Bundle 안에 같은 특수 Parent가 다시 들어가면 거부한다. 누락, 중복, 다른 gate, 없는 ID와 잘못된 대상은 이전 catalog를 보존한 채 실패한다. format 37과 protocol 111의 기존 계약을 유지한다.

최종 검증은 product 및 raid 전용 selector로 실제 게시 P88/P33, plain entry, WORLD owner 수명, draft 선택 closure, 유한 bootstrap 용량과 빙고 특수 Parent의 실제 10단계 실행을 확인한다. 전체 contract의 중단된 구간과 Client 화면 확인은 실행한 검증과 분리한다.

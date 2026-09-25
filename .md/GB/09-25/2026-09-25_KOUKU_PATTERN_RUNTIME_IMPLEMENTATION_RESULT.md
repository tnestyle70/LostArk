# 쿠크 패턴 Server 런타임 구현 결과

## G00. 반영 범위

기존 Server Logic, RAID flow, WORLD body와 publisher 경로를 확장했다. authored 원본 데이터 설치와 통합 Product 빌드는 root 작업이며 이 문서는 Server 분담 결과만 기록한다. Client/UI를 실행하지 않았고 렌더링 설정은 수정하지 않았다.

## G01. 구현 완료

- AREA_OVERLAP의 repeatIntervalMs를 ENTER_AREA와 같은 플레이어별 접촉 틱으로 소비한다. 접촉한 틱부터 damage와 madness 결과를 함께 실행하며 종료 틱에는 추가 피해를 주지 않는다.
- CARD_RAIN_SOLDIERS는 CLUB/HEART/DIAMOND 수량과 최소·최대 반경을 검증하고 전체 생성 위치를 먼저 확보한다. PATTERNCARDRAINSOLDIERS supplemental row가 기존 Trigger와 연결된다.
- 34ms 회전 Trigger는 가장 가까운 생존 플레이어를 즉시 바라본다. 양수 속도의 추적은 몸체 반경과 플레이어 반경에0.5m 여유를 더한 거리에서 현재 패턴을 완료한다.
- 빙고 보드는 encounter 소유로 유지한다.30/50/70초 이후20초 간격 표식,6초 표식과2초 비표시 지연 및4초 바닥 fuse를 분리했다. 표식 종료 때 cell을 고정한다. 매 세 번째 표식이 작은 특수 Parent를 삽입하고 완료 뒤 일반 flow로 복귀한다.
- 최신 확정 계약: 세 번째 폭탄은 정상12초에 폭발한 뒤 뒤집힌 빨간 칸의 완성 행+열 합계를 판정한다. 대각선은 제외한다. BINGO_COMPLETED_LINES Duration의 threshold3 성공은 PLAYER_INVULNERABILITY Result의30000ms를 모든 생존자에게 적용한다. BINGO_DETONATION은24.828초에 성공이면30초 버프로 보호하며 boss13줄을 차감하고, 실패이면 raid roster 전원에게 보호를 우회하는 encounter wipe를 적용한다. 빨간 tile 유지, 흰 tile 제거, 빈 tile 빨간색 전환을 구현했다. 바닥 광기는10%/초다.
- authoredMadness WORLD supplemental row와 소비자를 연결했다. 명시된 WORLD만 기존 aura를 억제한다.
- ownerWorldOccurrenceId는 damageable WORLD에 연결된 contact의 stable occurrence를 보존한다. body HP0/취소/만료 때 동일 epoch/member/sequence의 active 또는 tail ledger를 종료하며 후속 결과와 Timeout을 실행하지 않는다. 플레이어 피해 처리 후 boss Logic 전에도 body retirement를 수행한다.
- model-less WORLD group의 collider를 실제 motion member와 emission별로 기존 bake 경로에 전개한다. P83 후보는 out/KoukuPattern20260925/circus-split.patch.json에 source hash와 stable ID 패치로 작성했다. g0..g3의15개 공 impact 및 첫 bounce를 포함한16개 피해 창이다.1m half extent 및10% 피해는 원본 복원값이 아닌 PROJECT_TUNED 초기값이다.
- Cross direction real child와 clone별 접촉 ledger를 추가했다. 기존 root motion 적용 뒤 실제 위치를 기준으로 평가하며 child 결과는 contact damage/madness만 허용한다.
- Bone contact의 baked yaw는 unit quaternion 검증을 유지하며 허용한다. 기존 identity yaw만 받던 PS/Server 최종 guard를 교정했다. unit scale, visible key, identity baseline과 정확한 접촉 시계 검증은 유지한다. 기존 transform 소비자가 bone yaw, authored offset, 실제 boss basis를 한 번씩 합성한다.
- G1 popup book의 endpoint pose를 숨겨진 prepared clone에 미리 Sample한다. 기존 scene을 바꾸거나 재생 owner를 만들지 않는다. 준비 실패는 pending owner 폐기로 rollback한다. WorldSequence.HiddenPose.Prewarm 및 Kouku.GateObjects.Prepare/Activate/Commit profiler scope를 추가했다. 화면 멈춤의 원인이나 해결을 실측한 것은 아니다.
- 최종 게시의 82,441행은 기존 65,536행 상한을 넘었다. WORLD key의 증가는 인형 39,132, 백스텝 2,232, 분열 공 48, aura 공 20으로 총 41,432행이다. 기존 150 tracks의 key 수는 그대로이며 신규 최대 2,352 keys는 4,096 per-track 상한 안이다. 이를 위해 제품 bootstrap을 131,072행/64MiB로 제한하고 Shared/Server/Client/publisher/Python 소비자를 맞췄다. Server의 기존 전체파일 무제한 읽기는 크기 선검사와 exact bounded read로 교체했다. draft 16MiB는 유지했다. 통계는 out/KoukuAuthoring20260925/bootstrap-key-contributions.json 및 bootstrap-key-summary.json이다.

## G02. 실행한 검증

- python -m unittest Tools.KoukuSaydonPipeline.test_world_object_collider:22tests 통과. group의 독립 위치·시계·ID와 disabled member 제외, missing member/binding 거부를 포함한다.
- 변경 Server 및 publisher 파일의 git diff --check 통과.
- KoukuBootstrapRows.ps1 PowerShell AST parse 통과.
- Debug Server --kouku-object-overlap-contract-test: failures0. 신규 AREA_OVERLAP100ms 반복 damage/madness 및 종료 경계를 포함한다.
- Debug Server --bingo-contract-test 첫 실행:3fail. 기존 red mask equality는 빈 tile도 빨간색으로 바뀌는 새 규칙에 맞춰 bit-mask 비교로 수정했다. 기존 hook fixture2개는 navigation=nullptr였으므로 실제 admitted navigation과 보드 내 endpoint를 사용하도록 수정했다. 후속 최신 Bingo 실행에서 모두 통과했다.
- 두 번째 Bingo 실행은 hook landing projection 기대1개만 실패했고 새detonation/facing/pursuit/per-actor contact는 통과했다. hook은 raw sample position 대신 실제 Project_PointOnSameLevel 결과와 비교하도록 수정했다. 이후 사용자3줄 판정 계약으로 갱신한 최신 fixture도 아래 최종 실행에서 통과했다.
- Debug Server --kouku-raid-contract-test 첫 실행:32fail. actual published G1/standalone intro/sequence-clock fixture에서 실패했고 out/KoukuPattern20260925/server-raid.log에 남겼다. 이는 최종 게시 전 실행이며 최신 게시본 검증 결과와 구분한다.
- Server 통합 첫 빌드에서 INITIAL_DELAY 상수 이름 오타를 발견하여 Shared의 KOUKU_BINGO_BOMB_INITIAL_DELAY_MS로 수정했다. Product 6 Debug Server 링크에 최신 구현과 fixture가 포함됐다.
- 최신 Debug Server --bingo-contract-test: failures 0. 세 번째 폭탄의 정상 12초 폭발 직후 행·열 3줄 판정, 대각선 제외, 모든 생존자의 30초 무적, 후속 폭발의 보스 13줄 피해와 실패 시 보호막·기존 무적을 무시하는 전멸을 통과했다. hook의 실제 착지 projection, 가장 가까운 플레이어를 보는 Trigger, 추적 종료, 실제 actor/clone별 접촉 위치와 중복 틱 방지도 통과했다. 로그: out/KoukuPattern20260925/server-bingo-final.log.
- 최신 Debug Server --kouku-object-overlap-contract-test 및 --kouku-dice-hit-contract-test: 각각 failures 0. AREA 100ms 접촉의 즉시 판정·반복·이탈·종료 경계, damage와 madness 동시 적용, 기존 laser 전방 및 push 수치 계약을 확인했다. 로그는 같은 폴더의 server-overlap-final.log와 server-dice-final.log다.
- 최신 Debug Server --kouku-bundle-contract-test: failures 0. 실제 root motion, bundle participant의 실패 rollback, 지연 participant 사망 정리 및 게시 generation pinning 회귀를 통과했다. 로그: out/KoukuPattern20260925/server-bundle-final.log.
- Product 6 전체 빌드는 root가 Engine/Server/Client 통과를 확인했다. 이후 최종 publish가 구형 Bone yaw guard를 찾아 Server guard와 yaw contact fixture 1개를 추가했다. root의 Product 7 증분 빌드도 통과했다. Product 7 Bingo 재검증은 failures 0이며 로그는 server-bingo-product7.log다. 추가 yaw fixture는 최종 게시 뒤 전체 contract에서 통과했다.
- bootstrap 용량 변경의 Python 기존/추가 4 tests는 logic_tuning이 실행하여 통과했다. 행 상한과 1 초과, 정확한 행수, byte 상한과 1 초과, stat 이후 파일 증가, Shared 상수 일치를 포함한다. Server native fixture에는 실제 catalog의 131,072행과 64MiB 허용, 각각 1 초과 거부, 후행/잘린 행 거부와 이전 catalog 보존을 추가했다. Product 8 빌드가 통과했고 전체 contract에서 추가 native capacity fixture가 모두 통과했다. publisher PowerShell AST parse와 변경 diff --check는 통과했다.

## G03. 남은 검증 경계

1차 owner publish는 revision 2345, 82,441행, UTF-8 23,883,405bytes로 네 domain 모두 통과했다. Product 8 전체 빌드도 통과했다. 이 게시본의 전체 contract는 변경과 무관한 catalog 반복 검증을 줄이기 위해 중단했고, raid 검증은 끝까지 실행했다. 부분 검증과 실패를 아래에 분리한다. 이후 revision 2346의 게시와 focused 검증은 G06에 기록한다. 실제 Client 화면과 빙고 시각 판정은 사용자 확인 경계다. 특수 Parent의 폭발은 최신 순서에 따라 표식 후 24.828초이며 독립 폭탄의 12초 폭발과 다르다.

## G04. 1차 게시본 검증과 실패 원인 분리

- 전체 contract에서 131,072행/64MiB 정확한 경계 허용, 각각 1 초과 거부, 잘린 행/후행 거부 및 이전 catalog 보존을 통과했다. Bone yaw→offset/shape→boss basis의 실제 접촉 fixture도 통과했다.
- 전체 제품 23MiB를 단일 draft로 보내던 positive fixture는 16MiB 제한으로 실패했다. 실제 Client producer는 선택 pattern/bundle dependency closure만 보내며, 최대 P92를 실제 Prepare 경로로 생성했을 때 gameplay.rows 6,957,898bytes/23,544행, encounter 14,716,566bytes로 16MiB 안이다. 제품 제한을 늘리지 않고 fixture를 P1/P4 선택 closure로 교정했으며 whole-product 초과 입력은 negative 검증으로 유지했다. 실제 producer 테스트 로그/산출물은 out/KoukuAuthoring20260925/draft-largest-request다.
- direct Mario Intro fixture 4개는 NORMAL 초기값을 CLOWN 필수 전제에 맞췄다. P88은 실제 5개 Parent child를 가진 저장본과 chain-free runtime을 검증하려던 fixture의 전제가 달랐다. 실제 P88/P33 검증과 synthetic plain-entry 7개 사례를 분리했고, HP/form/위치/시계/status 진단을 추가했다. 제품 P88의 타이밍이나 피해를 줄이는 변경은 하지 않았다.
- full contract 출력이 고정되어 보인 시점의 native stack은 Run_RevisionProtocol의 catalog loader 대기를 확인했다. 마지막 Valtan 오류 줄에서 멈춘 것으로 단정하지 않았으며 미완료 전체 검증은 PASS로 기록하지 않는다.

- Product 8 전체 contract는 PID19672의 경로·실행인자·시작시각을 확인한 뒤 root 요청에 따라 중단했다(exit -1). 관찰 실패32개는 draft 전체제품 입력1, P88 진입14, directMario CLOWN 전제4, all-disabled Gate Debugspawn 전제7, Valtan 자원·push상태·저장flow 전제6이다. 뒤쪽 미실행 구간을 포함한 전체 PASS를 주장하지 않는다. out/KoukuPattern20260925/server-contract-final.log.
- Product 8 게시본 Raid는 끝까지 실행하여 failures 23이었다. 오래된 whole-Bingo-Parent 기대 5개, standalone advance/restart 18개다. 후자는 fixture의 저수준 Spawn 뒤 실제 제품 경로가 호출하는 Note_GatePlacementRaised가 빠져 있었다. 실제 데이터와 소비자에 맞춰 fixture를 교정했고 G06의 최종 실행에서 모두 통과했다. 이전 실패 로그는 server-raid-final.log로 보존한다.
- P1/P4 선택범위 fixture와 같은52행/37,034bytes를 기존 binary의 실제 draft admission override에 넣어 --kouku-draft-contract-test 실행: failures0. nonKouku/balance보존, checksum·다른domain거부,16MiB upload상한, 연속chunk/timeout/동일request/hash고정 모두통과했다. server-draft-selected-prebuild.log.
- Product 9 재검증을 위해 product-only, ValtanLifecycle+PinnedGeneration, SkillStages 전용 selector를 기존 contract worker stack/catalogLoad 경로로 추가했다. stdout 즉시출력으로 실패시점과상태를기록한다. 변경기능확인은이focused경로를사용하며모든광역catalog변형의완료를조건으로삼지않는다.

## G05. 빙고 반복 묶음의 명시적 특수 Parent 연결

- 사용자 최종확인 범위는 3관문 최초입장이 아니라 빙고페이즈 진입 자동시작이다. BINGO patternFlow의 bingoSpecialPatternId와 RAIDBINGOSPECIAL supplemental row를 Server gate정의에연결했다. 기존 동시실행 Bundle의 뜻을 바꾸지 않는다.
- 전체패턴을스캔해 특수Parent를추측하던코드를제거했다. 실제P107는authoring Parent가10개runtime stages로flatten되어 ParentChildren가없었다. 기존scan은이를못찾는문제가있었으며 exactFlowref로해결한다. source publisher는Parent구조를검증하고PS/Server는동일gate/boss, finitefixedclock, 단일detonation을검증한다. timelineDurationMs가0이면기존stage sum계약을사용한다. P107의10stage합은32628ms, detonation은24828ms다.
- 명시 ref 누락·중복·다른 gate·없는 pattern·단일 detonation이 없는 대상과 일반 flow 또는 Bundle 중복을 거부한다. 오류 시 이전 catalog 보존 native fixture를 추가했다. format 37과 protocol 111을 유지했다. 새 publisher와 컴파일 및 실제 native 실행 증거는 G06에 구분한다.
- P88의실제5children(59/125/116/124/118)과8.083~41.109초입장창을확인했다. 처음4.667초만보고입장중단을의심한추정은철회했다. 실제savedP88/P33/피해WORLD를보존한검증과syntheticplainentry7사례를분리했다. 제품P88 타이밍/피해/코드는수정하지않았다.
- 변경PS2개 ASTparse와diff--check통과. 기존Raid publisher arrival/dense-order 단일 Python unittest통과. 초기시험명/모듈경로오류는테스트수행실패로정정한뒤실제정확한test로1testPASS를확인했다.

- 실제2345 Encounter를메모리로만읽어specialref=P107을넣은 Add-KoukuRaidRows dryrun통과:268raid행,정확한RAIDBINGOSPECIAL1행,정렬gate0/special36. source/runtime파일교체는하지않았다. logic_tuning이실행한 test_raid_flow_projection 전체15tests도통과했다. PS flattened stage-sum32628, ref정확행/정렬, 누락·없는ID·다른gate·폭발0/2개·일반flow중복거부를포함한다.

## G06. 명시적 Flow 연결 후 최종 검증

- root의 Product 10 Debug 빌드는 Engine/Server/Client 모두 통과했다. receipt는 out/BuildPipeline/runs/20260924T201837120Z-debug-product.json이다.
- root가 최신 저장본에 stable field만 병합한 revision 2346 owner publish는 exit 0이다. koukusaydon.product, world, gameplay가 통과하고 map은 검증된 동일 산출물을 재사용했다. 최종 Gameplay bootstrap은 82,442행/UTF-8 23,883,456bytes이며, 새 RAIDBINGOSPECIAL 행 하나가 추가됐다. 로그는 out/KoukuAuthoring20260925/publish-flow-link.log다.
- 게시가 모두 끝난 뒤 product 및 raid 전용 selector를 병렬 실행했다. Product는 exit 0, failures 0, PASS 316개다. 실제 P88/P33, plain entry 7개 사례, WORLD owner 파괴 후 contact 정리, 선택 draft admission, 131,072행/64MiB 경계와 초과 거부, 명시적 special ref, Debug gate 및 FIFO lifecycle을 통과했다. 로그는 out/KoukuPattern20260925/server-product-flow-final.log다.
- 같은 Product 10의 Raid는 exit 1, failures 1, PASS 1,709개다. 과거 23개 실패의 standalone gate 전제와 독립 Bingo flow 기대는 교정 후 통과했다. 유일 실패는 아래 특수 Parent 삽입의 실제 busy admission 문제다. 로그는 out/KoukuPattern20260925/server-raid-flow-final.log다. 두 검사 process는 정상 종료했고, 전체 광역 contract의 미실행 구간을 PASS로 표현하지 않는다.
- 이 실행에서 실제 P88의 5개 child, 늦은 portal 진입과 P33 전환은 통과했다. Raid의 특수 Parent 실행은 busy admission으로 중단되는 제품 문제를 발견했다. Start_KoukuBingoSpecialPattern이 실행 중인 live boss를 그대로 preflight에 넘겨 이미 occurrence를 소유한다는 거부를 받았다. 대상 boss의 복사본에 기존 Abort_Pattern을 적용해 실제 취소 후 상태를 먼저 검증하고, 성공한 경우에만 live occurrence를 취소하고 새 Parent를 commit하도록 수정했다. admission 실패 시 live actor와 board 및 이전 owner를 보존한다.
- root의 Product 11 Debug 증분 빌드는 exit 0이다. Server OBJ 2개와 링크 1개만 갱신했으며 Client OBJ/CSO 갱신은 없다. receipt는 out/BuildPipeline/runs/20260924T203557169Z-debug-product.json이다. CPP와 Raid fixture만 수정했으므로 revision 2346 데이터 재게시는 필요하지 않았다.
- Product 11/r2346의 --kouku-raid-contract-test 재실행은 exit 0, failures 0, PASS 1,712개다. 실제 세 번째와 여섯 번째 표식이 실행 중인 일반 패턴을 중단하고 특수 Parent를 시작한다. 단계 상대 tick [0,162,189,249,314,355,745,751,786,939], 폭발 tick 745, 완료 tick 979 및 중단된 stable entry부터의 재시작을 통과했다. 실패 preflight는 모든 live actor/기존 occurrence/receipt/board/다음 epoch를 보존한다. 독립 보드와 일반 prefix/loop, 2~4인 gate 진행·재시작, Encore와 종료 시계도 통과했다. 로그는 out/KoukuPattern20260925/server-raid-flow-interrupt-fixed.log다.
- 이번 변경에 필요한 focused 검증은 완료했다. 제품 selector는 Product 10에서 통과한 뒤 영향받지 않았으므로 반복하지 않았다. 전체 광역 contract는 G04의 부분 실행 상태를 유지하며, 실제 Client 화면과 G1 전환의 프레임 시간 개선은 사용자의 화면 및 profiler 확인 경계로 남긴다. 에이전트는 Client/UI를 실행하지 않았다.

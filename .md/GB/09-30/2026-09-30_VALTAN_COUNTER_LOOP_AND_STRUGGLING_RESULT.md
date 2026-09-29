# 발탄 카운터 반복·발악 돌진·부위 파괴 표시 결과

## G00. 적용 범위

2026-09-30 현재 저장본에 승인된 필드만 병합했다. Valtan gameplay와 두 authored Effect의
교체 직전 hash를 재확인하고 백업·원자 교체했다. 기존 미커밋 변경, 렌더링 옵션, 사용자
pattern timing은 보존했다. Resource 추가와 project/filter 변경은 없다.

`out/ValtanRaidCorrections20260930/data-merge.json`에 세 파일의 전후 hash와 적용 필드가 있다.
각 `.before`는 교체 직전 원본이다. 실행 중 도구의 메모리 draft와 Server 활성 상태는 별도다.

## G01. Trash 반복과 성공 판정

`VALTAN_TRASH`와 `VALTAN_TRASH_CATCH_IF`의 `RETRY_EXHAUSTED`, `CATCH_SLAM`,
`EXECUTE_TAIL`은 단독 TIMEOUT/default로 `RECHARGE_WAIT_02`에 돌아간다. 세 번 miss와
부분 포획 피해·해제로는 종료하지 않는다. 실제 카운터가 `GROGGY`로 분기한 뒤 정상 완료한다.
전원 포획은 기존 즉사와 tail을 유지하며 다음 재시도에 생존 대상이 없으면 NO_VALID_TARGET으로
중단한다. 이를 counter 성공이나 Next 진행으로 기록하지 않는다.

저작 생성기와 Python/Gameplay publisher/Server catalog/Client canonical tree/Encounter reader의
순환 검증은 이 exact 세 edge만 허용한다. 단독 성공·실패 audition 조각과 다른 graph의 유한 검증은
유지한다. Workbench 그래프는 retry 화살표를 보여 주며 그래프와 타임라인 미리보기는 한 회차까지
표시한다. 반복 시 `then repeat`로 안내하고 종료 상태로 바꾸지 않는다.

기존 native `ValtanPinnedGeneration` 테스트는 세 번 miss 뒤 반복·실제 counter, 부분 포획 뒤 반복·
실제 counter, 전원 처형 뒤 targetless 중단, 처음 counter 성공의 네 경로를 검사하도록 바꿨다.
`ValtanResetlessNext`의 전원 포획 후 정상 완료 기대도 실패 중단 계약으로 바꿨다.

## G02. 발악 포탈과 경고

STEP_01의 NONE target/aim을 유지하고 STEP_02는 중앙 이동에서 FORWARD 6m/500ms로 바꿨다.
STEP_03의 기존 중앙 이동은 유지했다. body root -90도와 snapshot source basis -90도를 모두
적용하면 이전 portal local +X는 owner 왼쪽이다. notify-012 root emitter 16개의 위치만
`[0,0.5,-6]`로 바꿔 owner 전방6m에 놓았다. 왼손 emitter 7개와 geometry 회전은 보존했다.

발밑 Effect는 `.growing-warning` 한 element의 visible만 false다. 나머지 폭발·light·post와
Server damage/밀치기는 동일하다. Product 게시 후 rootmotion 재생성으로 STEP_02의 이전 curve를
제거해야 한다. 이 순서와 Gameplay publish는 통합 작업이 수행한다.

## G03. 부위 파괴와 즉사 전달

준비 PNG와 성공 text를 reference pixel 기준24px 위로 옮겼다. Server 조사상 PART_BROKEN은
실제 typed durability 소진에서 발생했다. 일반 타격마다 Server event가 발생하는 현상은 재현하지
못했다. Client는 같은 snapshot의 제거된 armor mask를 확인하고 part별 일회 feedback mask를
두어 일반 타격·중복 event가 성공 표시를 다시 시작하지 못하게 강화했다.

후속 사용자 요청으로72×72 PNG만 직전 위치에서 왼쪽24px·위24px 더 이동했다.
WorldHealthBarView의1280×720 reference 기준 x에-24, y는-16→-40을 적용했다.
성공 text와 파괴 판정은 유지했다. ASCII/CRLF와 기존 project/filter 등록을 확인하고
관련 git diff --check를 통과했다. 이 위치 추가 변경의 컴파일·제품 반영은 통합 담당이 수행하며,
화면상의 최종 위치는 사용자가 확인한다.

Valtan contact의 INSTANT_DEATH와 MAX_HP_PERCENT≥100은 공통 hit의 bInstantDeath로 전달한다.
이 판정은 isCombatReady 보호를 우회하고 공간 충돌·cover 검사는 보존한다. Shield/무적 우회와
Guide 포함 여부의 최종 판정은 통합 공통 hit runtime이 소유한다.

## G04. 교차 검토에서 보완한 무력화 채널

공통 damage/1000은 Valtan/Ghost/Kouku archetype에만 적용하고 다른 boss의 iStaggerDamage는 보존한다.
PlayerSkillSystem의 DAMAGE/COUNTER/STAGGER는 독립 결과 row다. STAGGER의 rawDamage를 호출자가
미리0으로 지우는 경계를 고쳐 caster·projectile timed/contact에서 같은 cast의 독립 subhit ordinal로
피해 기준을 전달한다. buff/spread/critical/armor를 거친 STAGGER 기준값은 /1000에만 사용하고 HP는
차감하지 않는다. DAMAGE/COUNTER는 무력화를 중복 지급하지 않는다. Guide 제외와 회오리1/3을 유지한다.

Numeric STAGGER의 단일 RAID_COMMON source CAS·catalog·publish·room 경로를 확인했다.
NumericSourceBindings는 PATTERN_DAMAGE용으로 common STAGGER가 누락된 것이 아니다.
기존 temp sandbox의 4세션 실제 typed Save+Apply에 40000→41000→40000 roundtrip을 추가했다.
두 room의 활성 generation, durable catalog 재로드, source/bootstrap bytes 복원을 검사한다.

## G05. 검증과 남은 화면 확인

- Python Trash projection와 네 variant 임의 cycle 거절 집중 테스트2개 PASS.
- 효과 위치: root emitter16개 × owner yaw49개 =784개 수치 PASS. 최대 오차8.89e-16m 이하.
- portal 변경 필드는 root16개 position뿐이며 왼손7개·나머지 필드 동일. underfoot은 visible 한 필드만 변경.
- split JSON parse/strict gameplay validation/join PASS. `git diff --check` PASS.
- 개별 TU compile 대신 통합 작업이 Debug/Release 제품 빌드와 native 계약을 수행한다. 이 문서 작성 시 결과 대기.

필요한 native 실행은 Server `--valtan-lifecycle-contract-test`, `--numeric-balance-contract-test`,
`ValtanPatternAuditionServiceHarness --action-composition-graph-contract`다. Lifecycle에는 게시된
FORWARD6m/500ms·빈 rootmotion·4방향 Try_BuildStageMotion 검사를 포함했다.

Client/UI는 실행하지 않았다. 사용자는 실제 포탈 진입 방향·속도, 경고 제거, 부위 파괴 PNG와 성공
문구 위치, 카운터 직전까지 반복되고 카운터 뒤 종료하는 화면을 최종 확인한다. 이 CPU 검증을
화면 판정으로 대신하지 않는다. 최종 publish·빌드·패키지 증거는 통합 RESULT가 소유한다.

## G06. 편집기 그래프 native 실행

Debug/Release에서 ValtanPatternAuditionServiceHarness 대상만 native MSBuild Build로 검증했다.
BuildProjectReferences=false로 Engine/Shared 제품 재빌드를 중복하지 않았다. 최종 빌드는 두 설정 모두
exit0이고 `--action-composition-graph-contract`는 각각11/11 PASS다. Client/UI는 실행하지 않았다.

최초 실행은8/11이었다. 새 Trash 반복·미리보기·임의 cycle 검사는 통과했고 기존 Dash 테스트의
11983ms/timeout→groggy 기대3개가 현재 저장본과 달랐다. 저장한 gameplay는 바꾸지 않고 테스트를
현재 stage duration 합계와 WALL_CONTACT 전용 groggy 분기로 교정한 뒤 재실행했다.

로그는 `out/ValtanRaidCorrections20260930/graph-debug-build-retry.log`,
`graph-release-build-retry.log`, `graph-debug-test-retry.log`, `graph-release-test-retry.log`다.
최초 실패 로그도 같은 디렉터리에 보존했다. 공통 Engine header의 기존 C4828 경고는 빌드 로그에 남아 있다.

추가한 SkillStages 검증은 실제 CPlayerSkillSystem::Try_Start/Update를 사용한다. 메모리상 복제된
기존34040 스킬의 총damage1000003을3회로 나누고 DAMAGE/COUNTER/STAGGER 독립 결과를 caster·
timed projectile·contact projectile로 실행한다. Mario HP감소 OFF/ON을 조합한6경로에서
HP1000003/999와 무력화999, HPevent3회와 무력화event3회를 검사한다. 이 native 실행 결과는
통합 작업의 `--skill-stages-contract-test` 로그를 따른다.
무력화 최대치 숫자 반영에서는 현재 진행량을 단순 min으로 clamp하면 이미 가득 찼는데 성공 outcome이
없는 상태가 될 수 있었다. 기존 ratio 계약으로 current/maximum 비율을 보존하도록 교정했고 Kouku
현재·retained owner의 credit에도 적용했다. Numeric roundtrip에75% 진행률을 두 번 보존하는 검사를
추가했다. 테스트의 boss state와 source/runtime 수정은 모두 복제 sandbox 안에서만 발생한다.

## G07. 기존 연속 성공 fixture의 필수 카운터 입력 보완

Release lifecycle 최종 검증의 실패8개 중7개는 연속4인 성공 fixture가 VALTAN_TRASH에서
카운터를 입력하지 않아 발생한 연쇄 실패다. tick12399에서65줄 Trash에 도달한 뒤60000틱까지
반복했고 기존 fixture는 STAGGER와 TRIPLE_COUNTER만 성공 입력을 공급했다. 나머지1개도
VALTAN_TRASH_CATCH_IF가 카운터 없이600틱 안에 종료한다고 가정한 구형 기대였다. 새
NONE/PARTIAL/ALL/실제 카운터 네 경로 검증은 이미 PASS였다.

제품의 반복 로직과 저장 gameplay를 변경하지 않고 두 기존 fixture만 보완했다.
ServerGameplayContractTests_ValtanLifecycle은 실제 COUNTERABLE 시점에 기존 Triple Counter와
같은 confirmed-hit 경계로 Trash 카운터를 전달하고 성공 횟수1을 검사한다. PinnedGeneration의
전체 CATCH_IF도 같은 카운터 입력 뒤 종료와 attachment 정리를 확인하며 독립 SUCCESS/FAIL
fragment의 기존 finite 검사는 유지한다. 강제 outcome이나 pattern skip은 사용하지 않는다.
소스 diff·git diff --check는 PASS이며 최종 Debug/Release 재빌드 뒤 native 재실행 결과는 통합
담당자의 로그와 후속 기록을 따른다. 이 수정 직후 Server를 따로 실행하지 않았다.


## G08. 최종 Release 실행과 Debug numeric 대기 경계

최종 제품 Release 빌드에서 Lifecycle93 PASS/0 FAIL, CardMaze75/0, SkillStages95/0을 확인했다.
각 프로세스 exit0이며 test-release-{valtan-lifecycle,card-maze,skill-stages}-delivery.log에 있다.
연속4인 lifecycle과 Trash IF 카운터 후 종료의 기존8실패는 모두 해소됐다. SkillStages의 실제
caster/timed/contact×Mario 감소 OFF/ON6경로도 독립 HP와 damage/1000 무력화를 통과했다.

별도로 통합 담당자가 실행한 Release numeric은 실제4세션 typed Save+Apply, BUSY/stale/invalid,
Windows replacement lock rollback, 재시작, shared maximum40000→41000→40000, 두 room과
retained owner의75% 진행률 및 exact source/bootstrap bytes 복원을 모두 통과했다.

동일 Debug numeric은 첫 저장의 fixture45초 제한을 넘겼다. PID45312는 UTC20:16:18 시작,
20:17:02 persist admission lock,20:18:03 실제 source/bootstrap/receipt 저장을 완료했다.
약105초 뒤 저장은 이루어졌지만 테스트는 이미 이전 revision을 캡처하여 후속 검사가 연쇄 실패했다.
Release 성공을 Debug 통과로 대신 기록하지 않는다. 비최적화 Debug의32MB bootstrap 및 전체
source 검증·저장 비용이 제품 사용에서도 길 수 있다는 성능 한계는 남는다. 단계별 CPU profiling을
수행한 것은 아니므로 특정 parser 하나를 병목으로 확정하지 않는다.

제품 worker를 변경하지 않고 RevisionProtocol fixture의 bounded settle을 Debug180초,
Release45초로 분리했다. 모든 settle은 elapsedMs·budgetMs·requester·persisting·workerValid를
출력하고 최초 settle 미완료면 실패1개를 기록한 뒤 종속 검사를 중단한다. UTF-8/CRLF와
관련 diff --check는 PASS다. 이 테스트 변경 뒤의 Debug 재빌드·단독 재실행 결과는 통합 작업의
최종 로그를 따른다. 원본 제품 수치는 변경하지 않았고 모든 numeric 실험은 PID별 TEMP 복제본이다.

최종 root 재검증: Debug Numeric도 18 PASS/실패0으로 완료됐다. 최초 저장95.790초, 잠금 복구40.578초, 무력화 변경51.245초/원복49.449초이며 실제 durable byte 원복과 전 room 진행률 유지가 통과했다. 최종 전달은 RAID_GAMEPLAY_REPAIR_RESULT G07의 ZIP을 따른다.


## G09. G 입장 후 두 번 재생되는 휠윈드

사용자 확인 경로는 F1 Play All이 아니라 G 입장 → 컷씬 → 휠윈드 두 번이었다.
SelectPattern은 자동 cinematic 뒤 VALTAN_ENTRANCE_WHIRLWIND를 강제하고 이후
ORDERED_LOOP 첫 항목 VALTAN_WHIRLWIND를 선택했다. 등장용 공격은 SWEEP1930ms/반경5m,
저장된 일반 공격은 SPIN1200ms/반경10m인 서로 다른 기존 정의다. 09-29 G06과 이전 Lifecycle
fixture가 이 두 공격을 모두 요구하여 기존 검증도 잘못된 입장 기대를 통과시키고 있었다.

Trash 변경 전후 gameplay decisionModel은 같으며 Brain의 이 선택 분기는 이번 수정 전까지
변하지 않았다. 버러지 retry가 휠윈드를 추가한 것으로 단정하지 않는다. 자동 cinematic 선택에서
bEntranceCinematicConsumed와 bIntroPatternConsumed를 함께 true로 하여 legacy 공격을
추가하지 않도록 교정했다. legacy/F1 진입의 range gate와 모든 사용자 저장 데이터는 보존했다.
정상 체력의 첫 세 occurrence는 CINEMATIC → WHIRLWIND → DASH_CHARGE다.

PendingPatternIds의 체력 기믹 우선순위와 FinishPattern cursor 증가는 변경하지 않았다.
자동 health ORDERED_LOOP의 기존 추적 시간0과 F1 전체재생 scripted sequence의 저장1000ms
추적 계산도 그대로다. Client BGM은 M05 상태에서 non-IDLE snapshot을 받으면 M06로 바뀌는
기존 경로가 있으므로 일반 휠윈드 WINDUP에서 전환 가능하며 별도 수정하지 않았다.

Server source와 Lifecycle 기대를 교정했고 UTF-8/CRLF 유지·관련 git diff --check를 확인했다.
이 항목 작성 시 재빌드와 실제 native Lifecycle 재실행은 통합 담당자 대기이며 통과로 기록하지
않는다. Client/UI를 실행하지 않았고 십자돌의 추가 화면 차이는 사용자 확인 중이므로 수정하지 않았다.

## G10. 버러지 화면 정지의 추가 소비자 경계

서버 retry는 같은 patternSequence 안에서 RECHARGE_WAIT_02로 돌아간다. Client의 기존
Apply_NetworkState는 stageIndex가 감소하거나 같은 stage의 actionStartTick이 달라지면
거절하여, 첫 회 이후 서버가 계속 진행해도 해당 객체의 화면 갱신이 중단될 수 있었다.
기존 서버 Lifecycle의 반복·카운터 성공은 이 Client snapshot admission을 검증하지 않았다.
Client 담당자가 Trash/TrashCatchIf의 정확한 retry action12개 사이에서만 최신
forward actionStartTick의 같은 stage 또는 이전 stage 재진입을 허용했다. initial STEP과
GROGGY는 예외 대상에서 제외하고 serverTick/sequence/다른 pattern/과거 actionStartTick
및 일반 패턴 역행 거절은 유지했다. 실제 Apply_NetworkState admission 본문을 이용한
격리 native CPU 검증48/48 PASS이며 이전 소스에서는 두 패턴의 세 retry tail6개가
실패했다. 동일 stage로 합쳐 수신된 다음 회차24개, 정상 카운터 종료, tick wrap도 포함했다.
근거는 out/ValtanTrashSnapshot20260930/validation-receipt.json, run.log, edit-receipt.json이다.
제품 빌드와 실제 Client 화면 검증은 이 CPU 결과에 포함하지 않았다.


## G11. 지정 정상 ZIP과 변경 이력의 독립 대조

legacy 강제 입장과 첫 ORDERED_LOOP 휠윈드는 commit e3f7ff1c5
(2026-09-29 04:19:09 KST, valtan-pattern-composition)에서 함께 도입됐다.
부모 commit의 mode는 ORDERED_ONCE_THEN_IDLE이고 해당 commit에서
HEALTH_BAR_ROTATIONS, entranceCinematicPatternId, mandatoryEntranceAttack,
첫 WHIRLWIND 순환이 추가됐다. 이번 세션의 원래 Brain 변경은 instant-death 접촉3구역뿐이었다.

사용자가 정상 기준으로 지정한 Desktop/LostArk-Release-20260929.zip의 실제 bootstrap과
현재 bootstrap을 메모리에서 읽어 비교했다. intro/rotation/WHIRLWIND/GROUND_ROAR/CROSS
관련196개 row는 동일했다. 09-30 Verification ZIP의 같은196개 row도 직접 읽어
대조했으며 정규화 SHA256는 모두
179ed94298d14774a329248f8dfae015d05ebc6ef6ec6156f6a2bc1444edf893이다. Valtan 전체2082→2081 row 차이는 presentation generation1,
Trash/IF TIMEOUT6개, Struggling STEP02 FORWARD6m 및 기존 rootmotion17sample 제거뿐이다.
GroundRoar는 양쪽 모두 AUDITION_ONLY이고 자동 rotation에는 없으며 기존 Play All의
0-based22 위치에 있다. 데이터가 legacy 순서로 되돌아갔다고 볼 근거는 발견하지 못했다.
이 비교만으로 두 ZIP의 실행 바이너리·접속한 Server·활성 런타임이 같다고 단정하지 않는다.

기존 test_valtan_trash_finite_retry_contract.py는 이전 finite기대 때문에6개 중3개가 실패했다.
두 개는 retry를 종료로 보던 기대이고 하나는 recipe가 생성한 recharge5707ms와 사용자저장4100ms,
추가된 catch effectCues를 문서 전체 동일성으로 비교한 문제다. 저장 데이터는 변경하지 않았다.
테스트를 retry/counter 소유 필드에 맞추고 정확한3개 backedge, 임의 cycle 거절, standalone
SUCCESS/FAIL finite, NONE/PARTIAL/ALL 두 회차 이상 재시도 후 COUNTER_HIT→GROGGY 종료를
검증하여8/8 PASS다. 실제 confirmed hit의 outcome 생성은 기존 native Lifecycle 범위이며
Python graph 추적을 제품 runtime 실행으로 대신 기록하지 않는다. 최초 실패/최종 로그는
out/ValtanRaidCorrections20260930/trash-retry-contract-before.log와 after.log다.


## G11. 사용자 지정 백업 ZIP 데이터 비교와 후속 Release 컴파일

비교 정본은 `C:\Users\user\Desktop\LostArk-Release-20260929.backup-20260930-024730-971826.zip`이다.
SHA256 `115a26384dfd2bb08972dc1b7a0e790ac01d2c555a7503b5fbca4f5de5ca84c0`.
Data 및 Client/Server DataFiles 2493개를 비교해 2474개 byte 동일, 19개 변경, 누락0이었다.
Valtan decisionModel 전체와 실제 bootstrap intro/sequence/rotation144행은 동일하다.
44개 pattern presentation과 binding/cue는 동일하고 gameplay/Encounter의 변경 pattern은
TRASH/TRASH_CATCH_IF/STRUGGLING 세 개다. 발탄 Effect 변경은 요청한 포탈 위치와
underfoot 경고 비표시 두 문서뿐이다. 이 비교는 ZIP에 없는 외부 Resources를 검증하지 않는다.

입장 legacy 강제 코드는 e3f7ff1c5f(09-29 04:19:09 KST)에 도입됐고 G06 문서에도 기록됐다.
이는 최근 Trash retry 데이터 변경이 입장 휠윈드를 추가했다는 증거가 아니다.
사용자는 이후 입장 순서만이 아니라 반복 전투에 잘못된 패턴이 섞인다고 정정했다.
저장 전체재생70개에는 GROUND_ROAR/SEQUENCE_WHIRLWIND/ATTACK_WHIRLWIND가 포함되지만
현재 HEALTH_BAR_ROTATIONS selectionSets에는 없다. 독립 패턴 부재를 같은 동작의 부재로
확대하지 않는다. BIND_SLOT/지형파괴/발악에는 각자 땅구르기·사자후 복합 단계가 존재한다.
실제 관찰 occurrence가 어느 경로인지는 화면 이름이나 asset 이름만으로 확정하지 않았다.
목록 정본 덤프는 `out/ValtanLegacyAudit20260930/pattern-flow.md`, byte 비교와 bootstrap 행 비교는
같은 폴더의 `backup-data-comparison.json`, `server-comparison.json`이다. 데이터 재게시·되돌림은 없다.

기존 세션의 G09/G10 소스와 이번 PNG 왼쪽24/위24 reference px 변경을 정상 Product Release
빌드로 컴파일·링크했다. receipt `out/BuildPipeline/runs/20260929T213022159Z-release-product.json`
PASS(54.232초), Server OBJ2/EXE1, Client OBJ2/EXE1, shader 컴파일0, missing/invalid runtime0.
새 Release `--valtan-lifecycle-contract-test`는 93 PASS/failures0이며 4인 입장의
CINEMATIC→WHIRLWIND→DASH_CHARGE와 Trash 반복·실제 카운터 종료를 검사했다.
이는 실제4Client 화면, Client retry snapshot 전체 통합, PNG 최종 위치 판정을 대신하지 않는다.
Client 실행·UI 조작 및 ZIP 갱신은 하지 않았다. 실행 출력은 `Client/Bin/Release/Client.exe`,
`Server/Bin/Release/Server.exe`이고 기존 ZIP은 변경하지 않았다.

## G12. 최종 발탄 동작과 저장값 보존

자동 G 입장의 HEALTH_BAR_ROTATIONS cinematic 경로는 entrance cinematic과 legacy intro를
함께 소비한다. 컷씬 뒤 legacy `VALTAN_ENTRANCE_WHIRLWIND`를 추가하지 않고 현재 저장된
첫 일반 순환으로 진행한다. Lifecycle 검사는 순환 목록으로 필터하기 전의 첫8개 occurrence를
`CINEMATIC → WHIRLWIND → DASH_CHARGE → HIGH_JUMP → FOUR_SLASH → CROSS →
DASH_CHARGE → WHIRLWIND`로 대조하고, 연속 성공 전투에서 legacy entrance0회를 확인했다.
기존 명시적 audition과 저장된 timing은 유지한다.

TRASH/TRASH_CATCH_IF는 놓침·포획 뒤 재시도하고 실제 COUNTER_HIT 이후 GROGGY를 거쳐
완료한다. 전원 처형 후 살아 있는 대상이 없는 중단은 카운터 성공으로 기록하지 않는다.
Client는 같은 patternSequence의 정확한 Trash retry action 사이에서 새 forward actionStartTick이
있을 때 stageIndex 역행과 같은 stage의 다음 회차를 수용한다. 과거 tick·다른 패턴·일반 패턴
역행 거절은 유지한다. 기존 실제 함수 native CPU48/48 결과와 이전 소스 실패6건 재현은
`out/ValtanTrashSnapshot20260930/validation-receipt.json`을 따른다. 실제 Client 화면 검증은 아니다.

### 갑옷 파괴와 PNG·실제 파괴 피드백

`VALTAN_PART_BREAK/PART_BREAK_RECOVERY`의 cardinal-rocks spawn과 전용 orphan
combatobject·visual·sound 연결을 제거했다. 원본 GROUND_ROAR의 돌과 공유 Effect는 유지한다.
PART_BREAK1800ms와 recovery5183ms는 그대로다. 현재 native 검사는155tick 유지·156tick
완료와 recovery에서 돌 definition·live object·damage·lifecycle event가 없음을 확인했다.

primary BOSS_VALTAN의 일반 몸체 갑옷은 서버가 archetype·owner pattern/action·impact hit를
확인한 destruction_bomb projectile 결과에만 반응한다. 일반 공격·스킬·폭탄 번호만 가진 hit와
벽 충돌은 갑옷을 차감하지 않는다. HP·독립 무력화 및 다른 보스의 일반 part-damage는 유지한다.
실제 두 groggy 구간에서 양쪽 갑옷을 제거하고, 이미 제거된 뒤 네 번째 실제 폭탄과 반복 벽
impact에서도 추가 파괴·복원이 없음을 검사했다. 네 관찰자가 같은 갑옷 상태를 decode했다.

PNG는 성공 순간에만 뜨는 표시로 바꾸지 않았다. 파괴 가능한 DASH recovery/GROGGY에서
남은 갑옷이 있을 때의 ready 표시다. 갑옷 mask가0이면 반복 돌진이나 추가 폭탄으로 다시
표시하지 않는다. 반면 파편·성공 text는 실제 `PART_BROKEN` 사건과 snapshot에서 새로
제거된 mask, 아직 feedback하지 않은 mask가 일치할 때만 발생한다. 중복 event와 이미 제거된
부위의 새 event sequence는 재생하지 않는다. 기존420627/420628 파편을 사용한다.
이 Client 조건은 소스 검토 범위이며, PNG 위치·파편 화면의 최종 판정은 사용자 확인 범위다.

### 마력구와 속박의 후속 연결

마력구 CHANNEL의 누적 HP 피해 response는 기존 독립 무력화 ENTER/EXIT gauge와
`STAGGER_BROKEN → VALTAN_GROGGY_FOLLOWUP`으로 연결했다. 현재 공통 최대치50000과
이후 F1 저장값을 catalog에서 읽으며 일반 피해/1000과 회오리1/3을 같은 기존 경로로 소비한다.
12000ms channel·+0.5m 높이·실패 stage와 타격 시점은 유지한다. Python4/4·V2 projector와
적용 receipt에 이어 최종 Release presentation suite27개가 모두 통과했다.

속박 제품은 살아 있는 대상의 피격 가능 상태를 유지하고 `bPatternBound`로 이동·스킬 입력을
차단한다. 저장된4107ms hold·3533ms recovery를5000ms로 되돌리지 않았다. 최초 fixture는
준비 상태와 회복 진입 위치를 과거 계약으로 검사했다. 실제 진단에서는 EXIT가 저장XYZ를
복원한 뒤 같은 tick의 플레이어 갱신이 남은 넉백을 진행하는 것이 확인됐다.

최종 검사는 기존 단계 거래 직후·플레이어 갱신 전의 읽기 전용 관찰 지점에서 정확한 위치,
owner/endTick 해제와 HP·남은 넉백 불변을 확인한다. 이후 실제 갱신의 넉백 진행과 추가 HP
감소 부재, 취소 복원, 단독 완료·다음 선택도 검사한다. 제품 피해·무적·넉백 정책은 바꾸지 않았다.
tick2000 입장은2123 RECOVERY,2229 완료이며, 구체적 진단은 같은 out의
`bind-result-candidate.md`와 `bind-exit-boundary-candidate/final-source-verification.json`에 있다.

### 발악과 기존 리소스

기존 승인된 전방6m 포탈·6m/500ms 돌진·중앙 복귀와 source-underfoot growing-warning만
숨기는 범위는 유지한다. 앞선 위치 검증784건과 파편 리소스 중복 제외50개 closure 확인은
기존 `valtan-result-candidate.md`의 근거를 따른다. 새 발탄 리소스 제작·추가 복사는 없으며
별도 PR487/488의 GBResources 복사와 구분한다. 수치·파일 검사는 GPU 화면이나 실제 본 부착
판정을 대신하지 않는다.

## G13. 최신 확정 검증과 남은 전달 경계

최종 Release의 기존7개 CLI suite는 합558 PASS, failures0, 전부 exit0이다.
`out/ValtanFinalRepair20260930/release-final-native-results.json`이 집계 정본이다.

| 범위 | PASS | 이번 발탄 관련 확인 |
|---|---:|---|
| BattleItems | 174 | 실제 폭탄 전용 갑옷·추가 폭탄·벽 impact·4명 복제, 아바타 포함 suite 전체 |
| ValtanLifecycle | 94 | 첫 순환·legacy entrance 제외·Trash 재시도/카운터·연속4인 전투 |
| ValtanPresentation | 27 | 최종 Magic/Bind·단독 속박·침묵·돌진·발악·PartBreak recovery |
| KoukuProduct | 96 | 별도 쿠크 후속 검증 |
| WorldPlayback / VehicleRiding / NPCRaidReturn | 34 / 99 / 34 | 통합 PR 및 복귀 경로 |

최종 전체 DataOnly는 `full-publish-final2.log`에서 완료됐으며 source revision은
`128d68d30634ff0c51eaab5f6e4669434842014cc795997abfe803e2c55eedc4`다. 최초 추가 게시의
world.destruction 실패를 보완한 뒤 Valtan PublishV2·world.destruction·gameplay.balance와
나머지 domain의 PASS/검증된 REUSED를 확인했다. 실행 중 Server 메모리 갱신이나 화면 확인을
게시 완료에 포함하지 않는다.

Debug Product는 `out/BuildPipeline/runs/20260929T233149354Z-debug-product.json`에 PASS다.
이후 Bind fixture만 추가 교정되어 최신 테스트 소스를 포함하는 Debug 재빌드·7개 native
검증의 최종 집계는 아직 대기다. 현재 Debug BattleItems174 PASS만으로 전체를 완료 처리하지
않는다. 최종 Release Product 재빌드는 검증용 Debug Server PID45512 때문에 output guard에서
컴파일 시작 전에 차단됐으므로 아직 대기다(`product-release-final.log`).

최초 광역 Release 전체 계약은1459 PASS/91 FAIL, exit1이었다. 은퇴한 HEALTH_BAR159 행,
과거 managed rotation, AIRBORNE8000ms 대신 현재7984ms, protocol101 대신126 등 구형
fixture 불일치는 확인했다. 그러나 queued generation 등 복합 실패와 범위 밖 검사 전부를
fixture 문제로 확정하지 않았다. 상세 목록과 미확정 분류는
`release-native-failure-audit.json`, `native-fixture-drift-audit.json`에 보존한다. 이 전체 suite를
PASS로 바꾸거나 사용자 저장 패턴을 과거 기대값으로 복원하지 않았다.

최신 Debug/Release Product 전체 완료, Debug native 최종 결과, GitHub merge와 새 ZIP의
hash·CRC·preflight는 담당자 확인 후 기록한다. 실제 G 입장·반복 Trash·카운터 종료·파괴 ready
PNG·실제 갑옷 파편·전방 포탈의 화면과 음향은 사용자 확인 범위로 남긴다.

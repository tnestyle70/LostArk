# 쿠크 Logic Box Detail 수치 튜닝 결과

## G01. 구현

Logic Box Detail과 연결된 Success/Fail/Timeout Result에서 기존 typed 수치 editor를 열고
적용한다. stable Logic ID별 미적용 draft를 유지하고 Apply는 기존 검증·commit 경로를 사용한다.
고정 HP, 최대 HP 비율, 광기, 넉백 거리/높이/시간과 방향, 공포 시간 등 해당 Result의 값을
편집할 수 있다. 같은 정의를 사용하는 모든 Box가 공유 수치 변경을 소비한다.

AREA_OVERLAP에 repeatIntervalMs 0 또는 34..600000을 연결했다. 0은 기존 종료 판정,
양수는 반복 접촉이다. 카드병정 counts(CLUB/HEART/DIAMOND 각 0..32, 합계 1..64)와
nav 생성 반경 min/max double을 JSON/Client/publisher에 연결했다. 기본은 1/1/1과 3..6m다.
카드낙하 반경/scale min/max는 기존 double을 유지한다. TRIGGER BOSS_TRACK_TARGET은
34ms cue로 최근접 방향 전환을 투영하고 BINGO_DETONATION도 typed Trigger로 저장한다.

BOSS/WORLD 본 Collider가 actual model pose에 local XYZ 회전/offset을 합성한 뒤 최종 XZ
중심·yaw를 사용한다. WORLD preview는 기존 CWorldSequencePlayer의 실제 CModel을 읽는다.
반복 AREA_OVERLAP의 WORLD/BONE track을 매 tick 샘플링하며 combatBody 소유 WORLD의
ownerWorldOccurrenceId, 명시적 전체 수명 광기의 authoredMadness를 투영했다.

cross direction child는 AREA_OVERLAP/ENTER_AREA와 FIXED_DAMAGE/MAX_HP_PERCENT_DAMAGE/
MADNESS_GAUGE_ADD_PERCENT만 허용한다. Server의 actual actor별 ledger 소비는 Server 담당이 연결했다.
빙고에는 BINGO_COMPLETED_LINES threshold 1..10과 PLAYER_INVULNERABILITY durationMs 1..600000을
Client codec/Box Detail/publisher에 연결했다. 실제 폭탄 사건·보호·폭발은 Server 담당 소비다.

## G02. 저장 후보

이 담당은 정본 Data를 직접 교체하지 않고 root의 통합 CAS 설치에 후보를 제공했다.
`out/prepare_kouku_result_tuning.py`의 `mutate(document)`는
최신 문서를 인수로 받아 narrow field 병합 후보를 만든다. 513/514 100ms, 515 고정100HP,
516 고정10HP, 인형·공 광기3% (기존2%/100ms보다50% 빠른30%/초), BOX 공통 형상, 기존 불 접촉·신규 십자/3방향 접촉,
10개 공과 10개 인형의 20개 mouth 본 접촉, orphan contact 3개 disable을 포함한다.
기존 일반 불 Collider 61개에 공통 피해 정의와 효과 수명을 연결한다.

P43/P102/P114의 9개 backstep Collider는 effect 담당의 실측 receipt local TRS를 사용한다.
사용자가 맞춘 P102 2858/2886/2913ms의 geometry는 재합성 오차 2.44e-7 이하로 보존한다.
효과 시작 1555ms까지 수명을 확장한 새 시작 pose는 당시 실제 본 pose를 따른다.
서로 다른 본이므로 기존 단일 anchor selectionGroup만 해제한다.

P81/P118 공굴리기 카운터는 WORLD resource의 `anchorBone`을 비워 Saydon BODY pivot에
맞춘다. 실측 b_root는 +Z가 수직 아래이며 몸체 기준과 다른 basis였으므로 기존 placement를
BODY 기준 yaw 90도로 바꾸고 XZ offset [-0.15,0.8]을 [0.8,0.15]로 함께 변환했다.
Y=0.7과 사용자의 scale은 보존한다. P118은 G3 Saydon을 명시하는 별도 objectResource/instance와
Composition WORLD 정의를 사용하고 기존 motion template/model/수치를 공유한다.
`mutate_world_sequences(document)`가 이 WORLD 후보를 만들며 두 mutate의 재적용은 동일하다.
이 수정은 실제 설치 모델의 b_root 측정과 저장 transform에 근거하며 최종 화면 판정은 별도다.
P81 화염 Collider는 저장된 BODY-local [5.10,0,-0.15], yaw -87.60도와 BOX halfExtents
[0.9,2,4.5]를 보존한다. 이미 몸체 +X 앞쪽을 덮는 이 geometry의 수명만 3002..12661ms로
맞추고 공통 100HP/100ms Duration에 연결했다.

Result 119의 Big Saydon wind는 기존 10% 피해, 16m 이동, 1500ms, 접촉점 반대 방향,
ballistic/arena-edge 허용 값을 같은 Box Detail에서 편집한다. 별도 이름 기반 강도 분기는
추가하지 않았으며 수평 거리/높이/시간을 typed 저장 값으로 전달한다.

## G03. 검증 증거와 남은 경계

- `python -B -m unittest Tools.KoukuSaydonPipeline.test_result_tuning_contract -v`: 7 tests PASS, 70.009s.
  숫자 보존/잘못된 interval 거부, 카드병정 기본·범위, 카드 scale 소수, cross contact 제한,
  광기 aura 대체 범위, 실제 설치 인형 WModel 본 회전과 local TRS를 검사했다.
- 신규 빙고 정규화/범위 거부 단일 test PASS, 0.046s. 전체 파일은 현재 8개 test다.
- 저장 후보 validate_document PASS(최종 G3 WORLD 추가 이전). 최종 Composition/WORLD 두 mutate 재적용의
  idempotence PASS. 마지막 G3/빙고 통합 후보의 overlay 검증·설치는 root가 실행한다.
- 변경 C++/Python/harness의 `git diff --check` PASS. 빙고 threshold 입력을 kind 초기화 블록에서
  지속 표시되는 DURATION 값 편집 분기로 옮겨 1..10 수치 편집을 연결했다.
- native --kouku-fixed-damage-contract에 duration 100ms, invalid rollback, fractional soldier
  Save/Reopen, instant facing/Bingo, threshold3/11, 30s 보호 Save/Reopen을 추가했다.

## G04. 최종 설치 확인

root가 revision 2345를 포함한 13개 authored 문서를 CAS transaction
`6e5464e620764245a74efce88e335dce`로 설치했다. `applied.receipt.json`의 status는 installed,
conflicts/rollback은 빈 목록이며 115개 Pattern과 GATE1/GATE2/GATE3/BINGO Flow가 검증되었다.

실제 설치 Data를 다시 읽은 `out/KoukuAuthoring20260925/logic-installed-audit.json`은 issues가
빈 목록이다. 일반 100HP/100ms, 인형·공 10HP/100ms와 광기 3%/100ms, Collider 61/10/20,
WORLD와 접촉창의 수명 일치, P81 WORLD28/G1 BODY 및 P118 WORLD47/G3 BODY,
yaw90도·Y/scale 보존, 9개 backstep의 실측 receipt 필드와 1555+4894ms 수명이 모두 일치한다.
Workbench 소스의 빙고 기본 threshold3 및 1..10 지속 편집 분기도 확인했다.

통합 Product 빌드·최신 native harness·domain publish 결과 정본은
[상위 구현 RESULT](2026-09-25_KOUKU_AUTHORING_REFINEMENT_IMPLEMENTATION_RESULT.md)다.
설치 파일과 실행 중 도구의 메모리 draft/Server 상태는 구분한다. Client/UI를 실행하지 않았으며
화면에서의 hitbox/이펙트 최종 판정은 사용자가 수행한다.

## G05. 게시 단계 BONE yaw 검증 교정

첫 domain publish의 gameplay.balance는 KoukuBootstrapRows.ps1이 BOSS_CURRENT 본 궤적에
identity yaw만 허용하던 잔여 조건 때문에 실패했다. `Publish-GameplayBalance.ps1`의 include를
따라 실제 validator를 수정하여 sampled yaw를 허용했다. visible=true, 각 key의 unit scale,
identity baseline, 접촉창과 같은 clock, finite/unit quaternion 조건은 그대로 유지한다.
Server의 동일 잔여 조건은 Server 담당이 같은 계약으로 수정했다.

- 기존 `test_result_tuning_contract`의 단일 추가 회귀검증 PASS, 2.368s. sampled yaw 허용과
  quaternion/scale/hidden/baseline/clock/endpoint 변조 여섯 종류의 거부를 확인했다.
- sourceRevision 2345의 실제 Encounter 백업에서 BOSS_CURRENT 궤적 27개, 2,850개 key를
  원본 PowerShell validator 블록으로 전수 검증했다. 2,877개 bootstrap row 생성 PASS.
  근거는 `out/KoukuAuthoring20260925/bone-yaw-guard/check.ps1`과 `regions.json`이다.
- 수정 PS/Python/문서의 diff check PASS. 정상 domain owner 재게시 결과는 상위 RESULT에 기록한다.

## G06. bootstrap 행 용량 실측과 Python 경계

두 번째 게시의 전체 82,441행 중 logic WORLD key는 73,999행이다. 기존 revision2304의
32,567개 key에서 추가된 41,432개는 인형 20개 입 track의 39,132개, 백스텝 9개 track의
2,232개, P83 개별 공 폭발 16개 track의 48개, 원형 공 10개 track의 20개다.
기존 150개 track의 key 수는 변하지 않았다. 신규 track의 중복 시각·역순·같은 pattern 안의
완전 중복 track은 없다. 인형 동일 인접 pose 15,634쌍은 모두 1ms 간격의 floor/ceil tick
bracket이다. 최대 track은 2,352개 key로 개별 4,096 상한 안에 있다.
`bootstrap-key-contributions.json`과 `bootstrap-key-summary.json`에 실측을 보존했다.

Server 담당의 131072행/64MiB bounded capacity 변경 뒤 기존
ValtanGameplayBootstrapAdmissionContractTests 4개가 0.209s에 PASS했다. 실제 131072행 허용,
131073행 거부, 잘린 declared count 거부, byte 상한/상한+1 및 stat 이후 파일 증가 거부,
빈 파일 거부와 Shared의 실제 131072행/67108864바이트/version37 상수 일치를 확인했다.
byte I/O 경계는 1024바이트 mock cap으로 검사했고 실제 배포 상수는 별도로 비교했다.
현재 게시된 이전 bootstrap의 파싱도 통과했다. 새 82,441행 전체의 최종 제품 admission은
상위 RESULT의 정상 domain 재게시와 Server 검증 결과를 따른다.

## G07. Debug 관문 fixture 전제 교정

최종 광역 Server 계약 검사에서 gate spawn/idle 7개 검증이 실패한 첫 원인은 fixture가
G1 Kouku를 아직 enabled 자동 생성 보스로 가정한 것이다. 실제 HEAD·저장본·게시본은 이미
G1 Kouku를 disabled로 둔다. placement admission 첫 조건이 false가 되며 build/idle/후속
body 거부 검사도 실행되지 않았다. 현재 all-despawn은 disabled G1/G3 모두를 제거한다.

해당 KoukuProduct fixture 구간만 수정하여 G1을 명시적으로 생성하고, enabled 입력 거부는
복사한 입력으로 유지한다. G1 idle·Saydon 몸체 불일치 거부·G3 완료 검증은 유지했다.
all-despawn의 G1/G3 제거와 재생 owner ABORT를 검사하고 unrelated Esther 보존을 추가했다.
좌표 검사는 실제 Client/Server F1 Gate1 목적지 (-2.45,1.32,740.37)를 사용한다. 제품/저작 데이터는 변경하지
않았으며 diff check PASS다. 수정 fixture의 컴파일·native 재실행은 root의 Product9 결과를 따른다.

## G08. standalone restart fixture의 관문 상태 교정

최종 Raid 검사 18개 실패는 G2/G3 restart가 각각 2~4인과 준비 성공/실패 조합에서
초기 관문 0을 유지한 fixture 때문이다. Spawn_GatePlacement는 actor만 생성하며 실제
Debug spawn과 Raid combat은 별도로 Note_GatePlacementRaised를 호출한다. fixture는 이를
생략하여 RESTART가 정상적으로 max(currentGate,1), 즉 G1을 준비했다. READY와 이전 상태
보존은 통과했고 예상 G2/G3의 이름·intro 비교만 실패했다. Advance는 boss death가 관문을
기록하므로 영향이 없었다.

KoukuRaid fixture의 source spawn 직후 같은 Note_GatePlacementRaised 호출과 source gate·
clear bit 검증을 추가했다. 기존 준비·부분 READY·실패 보존·intro 조건은 그대로 유지했다.
제품/데이터 변경 없이 diff check PASS이며 컴파일·native 재실행은 root Product9에서 확인한다.

독립 재검토에서 초기 fixture 교정이 stage.kakul.sl04의 먼 playerSpawn 좌표를 사용해
검증을 약화한 것을 확인하고 실제 F1 좌표로 다시 교정했다. G1 Saydon 중심과 XZ 거리는
실제 F1 목적지 약 3.705m, 잘못 선택한 playerSpawn 약 7.773m다. 두 product 소비자의
좌표와 일치하는 좁은 목적지를 검사하며 boss authored XZ 0.001m 조건도 유지했다.


## G09. 빙고 Flow 특수 Parent의 명시적 저작 연결

Pattern Flow의 optional `bingoSpecialPatternId`를 Client Composition load/save, 삭제 참조,
Boss Tool의 기존 Flow 편집/저장 목록에 연결했다. 기존 미지정 초안은 읽고 저장할 수 있다.
BINGO Complete Play/게시에는 같은 boss·actor의 유한 Parent와 자식 전체의 활성
BINGO_DETONATION 정확히 하나가 필요하다. 일반 Flow row 또는 Bundle 멤버와 특수 Parent의
중복도 거부한다. 특수 Parent 삭제 시 해당 참조만 정리하고 무관한 Flow 항목은 보존한다.

Flow 목록에서 일반 항목과 `[Every third bomb]` Parent를 함께 선택할 수 있다. 특수 재생 후에는
중단된 항목의 처음부터 재생된다는 안내를 명시했다. BINGO의 Play Saved Pattern Flow도
Server Complete Play admission을 사용한다. 기존 Complete Play의 전체 published pattern
prewarm에 특수 Parent가 포함되므로, 독립 selected-pattern memory draft를 전체 문서로
확장하지 않는다. P107 원본 children 127/94/128은 기존 dependency closure로 준비한다.

Python은 source의 raw Parent를 검사한 뒤 RAID gate에 stable ref를 투영한다. 실제 P107은
flatten Parent이며 runtime에서 ParentChildren나 PATTERNTIMELINE의 존재를 새로 강제하지
않는다. Server/PS 담당의 `RAIDBINGOSPECIAL` supplemental 소비와 연결하며 version37은 유지한다.

- `test_raid_flow_projection.py` 15개 PASS, 0.930s. 기존 Flow/HP group/arrival 검증에 더해
  draft 호환, stable ref projection, 실제 P107 closure, 누락/다른 scope/nested/repeat/0·2개
  detonation/일반 항목 중복 거부, PS flattened Parent의 exact row와 정렬을 확인했다.
- 기존 native fixed-damage fixture에 실제 P107 저장·재열기, 잘못된 참조의 last-good 보존,
  미지정 초안 허용 검사를 추가했다. C++ 통합 빌드와 native 재실행은 상위 RESULT를 따른다.
- diff check PASS. root의 revision2346 후보 설치·최종 게시 결과는 상위 RESULT에서 관리한다.
  Client/UI를 실행하지 않았다.


G09 최종 native 확인: Product10 전체 PASS 뒤 최신 authoring harness를 정상 x64 Debug로
증분 빌드했고 exit0이었다. `authoring-flow-build.log`에 기록했다. focused main 분기가
published generation/receipt를 읽지 않고 확정된 revision2346 source와 복사한 Action reference,
격리된 temp Data만 소비하는 것을 확인한 뒤 `--kouku-fixed-damage-contract`를 실행했다.
결과는 exit0 PASS이며 `authoring-flow-final.log`가 근거다. actual P107 ref save/reopen,
잘못된 ref의 last-good 보존, 미지정 draft 허용과 기존 3줄/30초 invulnerability typed
Save/Reopen을 포함한다. live source 바이트 보존도 PASS했다. 전체 runtime admission과
최종 owner publish는 이 focused 결과로 대신하지 않으며 상위 RESULT를 따른다.


최종 revision2346 domain 게시 완료 뒤 최신 native
`--presentation-generation-admission-contract`를 실행하여 exit0 PASS를 확인했다.
`out/KoukuAuthoring20260925/admission-flow-final.log`에 Pattern Sound의 독립 typed receipt
계약도 유지됐다는 결과가 기록됐다. 상위 owner의 최종 게시 수치는 82,442행/23,883,456바이트다.
이 결과는 게시 파일의 세대 admission 검증이며 실행 중 Client Reload·Server 재시작이나
사용자의 화면 확인을 수행했다는 뜻은 아니다.


## G10. 3관문 HP Flow 독립 후보와 범위 감사

revision2346 저장본으로 `out/KoukuHealthFlow20260925/prepare_g3_health_flow.py`의
`mutate(document)` 후보를 작성했다. 실제 Data는 변경하지 않았다. 10개 공통 순서는
P52 분신 기분나빠, P119 공먹고 노란장판, P59 화염파동, P116 정면바람방구,
P118 공굴리기 카운터, P39 알비온 감전, P38 무지개댄스, P117 돌진 카운터,
P43 백스텝 화염링, P40 십자화염폭발이다.

기존 G1 HP group 계약으로 180→155→125→90→80→55→0 여섯 일반 구간과
P88 / P91 / P76→P35 / P92 / P93 기믹 구간을 연결한 66 entries·11 groups다.
기존 16개 entry ID 및 해당 wait 값을 재사용한다. 사용자가 마지막에 지정한 공통10개를
우선한 후보이므로 기존 독립 P124/P125 추적·P62 레이저 항목은 Flow에서 빠지지만
Pattern 자체와 마리오 Parent 내부의 추적·공격 children은 전부 보존한다. 마리오 내부
패턴은 입장창 진행 내용이며 HP 일반 구간과 역할이 다르다. 각 입장의 MARIO_ENTER
result93/94/95/96이 공통 P33을 삽입하고 그 완료를 scheduler가 기다리므로 P33을
Flow에 중복 추가하지 않는다. P125는1000ms, P124는2134ms의 기존 추적이다.

P116.logic.2는 공유 DURATION65를 변경하지 않고 start0의 typed BOSS_TRACK_TARGET
Trigger로 연결했다. 후보의 새 Logic527은 실제 적용 시 최신 ordinal에서 할당되며,
같은 typed 정의가 생기면 재사용한다. 기존 occurrence ID, enabled, start와 결과 배열은
보존하고 duration만34ms로 맞춘다. Logic65의 나머지 P35/P113 소비자는 그대로다.

노란 원/도넛 P119는 이미 G3 MAP 좌표로 정렬돼 있다. FX4개(.4~.7)와 Collider3개
(.23~.25)가 모두 (-2.2561512,1.29900544,936.31436026)이다. G1 P79 FX 중심 대비
Z +204.800017m이며 selectedEffectGroup28의 G3 occurrence들이 같은 중심을 소유한다.
중복 변환을 피하도록 이 일곱 좌표와 boss-relative 십자 장판은 변경하지 않았다.
`g3-coordinate-and-child-audit.json`에 좌표와 child 원문을 보존했다.

시작의 ‘노란 장판·공3개 발차기·오허’는 기존 saved Pattern에서 아직 확정하지 못했다.
원본 reference의 4219902(서커스공 스매쉬_A)는 clip23_01/02/03/04와24_03을 사용하며
현재 saved Pattern에는 없다. Git의 09-11 notify inventory에서 stage0 첫1ms의
Attack02_ShotVox1·Cast1, stage2/3의0.3s projectile decal과1.3s Shot1은 확인했다.
원본 전체 ActionNameSources 경로는 이 PC에 없고 CSV는 projectile 숫자/스폰 좌표를
담지 않으므로 실제3공 및 ‘오허’ 청취를 확정한 것으로 기록하지 않는다.
`opening-action4219902-notifies.json`은 조사 근거이며 시작 Pattern을 임의 추가하지 않았다.

검증은 Flow validator, typed Logic validator, 동일 후보 재적용 불변, 타Pattern·타Gate
보존, live source 불변 모두 PASS다. P116만 격리하여 source shape/publishable 검증 및
read-only encounter projection도 PASS했으며 실제 BOSS_TRACK_TARGET start0/duration34ms
한 개를 확인했다(`g3-candidate/wind-projection.json`). 후보 JSON과 receipt는 같은
`g3-candidate` 폴더에 있다. 전체 통합 CAS 설치·제품 빌드·게시·UI는 실행하지 않았다.


## G11. 시작 연출 원본 조사와 마리오 내부 순서 확인

**시작 연출 미적용 / 사용자 모션 답변 대기.** revision2347의 공통 HP 그룹과 P116
Trigger 설치·owner 게시 PASS는 상위 RESULT에서 관리한다. 이 추가 조사는 live Data·제품
소스·빌드·게시·Client/UI를 변경하거나 실행하지 않았다.

retail data3.lpk의 MN_RPCT_07.loa를 out으로 추출하여 원본 102 actions·702 stages·6624
notifies를 읽었다. source SHA256은
`815959aa8ef30d8500eeec74b87adf44682380f115526ddc48ca04ad323f91f7`이다.
4219902는 clip23_01/02/03/04·24_03 두 세트의 선언 길이 합19초이지만 stage 분기/타이머를
그대로 선형19초로 사용할 근거가 없다. 원본 Effect는 포탈 예고와 grenade projectile
421990201/202/203에 연결되고 콜백은 피해이며 NPC 공 소환은 없다. 따라서 시작의 공3개
연출로 채택하지 않았다. ‘오허’ 청취 확인도 수행하지 않았다.

별도 후보4219944 ‘랜덤박스 요청’은 clip8_01의 Dance_Aura_02와 Attack27_ShotVox1,
clip8_02의1.0초 Effect421994401→NPC480602 ‘세이튼 서커스 공’ 연결이 있다. 같은 단계에
상자 소환0.6/0.8초도 있어 요청한 발차기와 동일하다고 확정하지 않는다. 이 action과
MN_RPCT_07의 runtime clip8_01/02는 현재 Composition 사용처가 없다. 큰 세이튼의
동명 번호 clip은 다른 profile/action이므로 포함하지 않았다.

현재 P88 시작은4219911 ‘제물 의식_A’ stage000, `rpct00_att_battle_27_01`4667ms다.
Attack08_ShotVox1(1ms), Cast1(100ms), Shot1(1800ms) 및 pentagram/portal 계열을
사용한다. 3365ms에 WORLD35 공2개와 WORLD22 인형2개가 나온다. 사용자가 이 시작
모션과 같다고 확인할 때에만 첫 animation·해당 sound/effect를 별도 연출로 재사용할 수
있다. Parent 전체·MARIO_ENTER·입장 판정·인형을 복제하는 방식은 해당 요청과 다르다.

WORLD35는 기존 Mario StripedBall 정적 모델과 MN_PPCC_00의4194527 stage002 notify003
`Par_L_PPCC_SK_02_Single` aura를 조립한 리소스다. 2000HP와7500ms 반복은 현재 저작값이다.
NPC480602는 같은 MN_PPCC_00 계열이며 spawnAction4194518 ‘스폰시 낙하피해’를 가리킨다.
이는 동일 공 계열의 근거이지 원본 NPC 전체 동작을 재현했다는 근거는 아니다. WORLD35
3개 배치는 PROJECT_AUTHORED로 가능하지만 원본3개·좌표·타이밍이라고 표시하지 않는다.
짧은 opener 이후에도 authored contact를 유지하는 수명/Flow tail은 별도 확인 대상이다.

revision2347 P88/P91/P92/P93 내부의 다섯 children은 사용자 |사이| 목록과 모두 일치했다.
P88은 화염파동→1초추적→정면바람방구→추적→공굴리기 카운터, P91은 무지개댄스→추적→
화염파동→추적→돌진 카운터, P92는 백스텝 화염링→추적→알비온 감전→추적→분신소환,
P93은 공먹고 노란장판→추적→십자화염폭발→추적→알비온 감전이다. P119 좌표는 이미
G3 보정되어 있다. P33 2페이즈는 기존 Server follow-up이 담당한다.

근거는 `out/KoukuHealthFlow20260925/opener-investigation.json`과 그 evidenceFiles,
`mario-children-r2347.json`이다. 새로운 opener Pattern이나 Flow 항목은 생성·설치하지 않았다.

초기 추출의 선행 경로 처리 실수로 C:\EFGame_Extra 아래 Action/MN_RPCT_07.loa 및
LookInfo/Monster/EFDLChar_MN_RPCT_07.MN_RPCT_07.loa 복사본이 생겼다. 이후 추출은
out 내부 resolved 경로를 검증한다. 앞선 C:\ 디렉터리 목록에 해당 root가 없었으나 생성
직전 전용 부재 영수증은 없었다. 두 파일의 정확한 경로·현재 hash/크기는 out 보고서에
기록했다. 자동 승인 검토가 정확한 두 파일 정리 명령을 `blocked by policy`로 거부하여
삭제를 재시도하지 않았고 복사본은 그대로 남아 있다. retail archive는 변경하지 않았다.

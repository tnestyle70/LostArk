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
## G12. 2026-09-25 화염파동 Apply 거절과 28개 Collider 일괄 저장

사용자와 실제 제품 OBJ 재현에서 같은 오류를 확인했다. revision2354의 P59.presentation.12에
수평0·상승3·500ms·ballistic/force를 넣으면 Apply=false, Dirty=false, generation1 유지,
`Selected Effect group is missing or has no members`가 발생했다. 수치 validator는 허용하지만
모든 Commit의 singleton 정리가 P47의 target2/target3 단일 Effect 그룹을 지워 다른 패턴의
의미 있는 참조가 깨졌다. Publish는 이미 정상 종료된 상태였으며 Apply 실패 원인이 아니다.

`Remove_SingletonPresentationGroups`는 해당 Pattern의 실제 Logic occurrence가 참조하는
selectedEffectGroupId/fixedSelectionGroupId를 보존한다. disabled occurrence도 보존하고
미배치 catalog 정의는 owner로 취급하지 않는다. 비참조 UI singleton 정리와 실제 누락·빈 그룹의
검증·실패 시 last-good 보존은 유지한다.

수정 전 제품 OBJ에서 신규 회귀는 같은 오류로 실패했고, 수정 Workbench 격리 TU에서
기존 fixed-damage 확장 회귀는 exit0이다. 실제 P59도 Apply=true, Dirty=true, generation1→2다.
회귀는 selected/disabled fixed singleton 보존, 비참조 UI singleton 정리, 수직 넉백의
Save/Reopen, 실제 누락 거부·디스크/last-good 보존을 확인했다. 제품 source/테스트 TU의 격리
컴파일과 diff check를 통과했다. 증거는 `out/KoukuVerticalApply20260925/`의
`fixed_probe_run.log`, `regression_run.log`, `regression_before_run.log`, `fixed_receipt.json`이다.

사용자가 3관문 화염파동 동일 Collider 전체에 일괄 적용을 명시하여 최신 디스크 저장본을
다시 읽고 P59 resource356의28개 Collider/28개 접촉창에 Trigger507→Result545를 연결했다.
Result545는 기존 최대HP10% 피해를 유지하며 수평0m·상승3m·ballistic=true·force=true다.
기존 이동시간이 없어 UI 기본500ms를 사용했다. revision2354→2355, nextLogicOrdinal545→546.
기존 공용 Result98 및 다른 모든 Pattern/Logic/presentation resource, Collider 배치는 그대로다.
새 Result1개와28개 창의 두 참조·revision/ordinal 외 의미 변경은 없다.

`apply_flame_wave.py`는 stable ID 필드만 교체하고 직전 bytes 일치·백업·원자 교체를 수행했다.
현재 저장 SHA256은 `f73c1e4b379497d7b898d40b0d26e1affe58fe4d547455f143aeb812e77ad03c`다.
`flame-wave.applied.json`에 백업·입출력 SHA가 있고 실제 Python 전체 문서 validation을 통과했다.

갈고리는 P33.world.2 실제 우선 placement의 X=0과 WORLD 기본 X=0 모두 저장되어 있다.
WORLD 원본/게시본 revision2217은 의미가 같고, Composition projected revision2354의
갈고리 placement도 일치했다. Rendering 원본/게시본 revision84가 의미상 같으며 쿠크 base
fxaaEnabled=false를 Mario1~4가 별도 quality override 없이 상속한다. 이 작업은 해당 값들을
수정하지 않았다. 실행 중 Client 메모리와 최종 화면 판정은 파일 저장 확인과 별개다.

## G13. 마리오2 사망·부활의 보스 유지와 앵콜 종료 tick

마리오 단독 입장자 사망을 연결 해제와 같은 runtime ABORT로 처리하던 분기를 분리했다.
사망은 실패한 기믹의 정상 완료로 기록하고 기존 corpse 귀환·typed 부활·다음 Flow를 사용한다.
살아 있는 raid boss를 제거하지 않는다. 실제 연결 해제와 잘못된 참가자 identity는 기존 실패
처리를 유지한다. 이전 입장자 부재 수정이 단독 입장자의 사망 분기까지 다루지 못했던 것이다.

`GameRoom_KoukuAudition.cpp`와 `ServerGameplayContractTests_KoukuProduct.cpp`의 변경 TU를
격리 컴파일했다. 실제 Mario2 사망→기믹 완료→시체 귀환→Raid receipt 소비→typed
C2S_REVIVE_PLAYER 이후 같은 boss ID 생존을 확인했다. 정상 terminal 복귀와 마지막 session
연결 해제도 확인했다. 동일 회귀는 수정 전4개 실패, 수정 후15 PASS/0 FAIL이다.
근거는 `out/KoukuMarioRevive20260925/{red-focused,green-focused}-product.log`다.
초기 전체 Product suite는 무관한 capacity 검사까지 실행하여 종료했고, 결과를 전체 통과로
기록하지 않는다. 같은 정식 회귀 본문의 Mario 세 경로만 out에서 선택해 검증했다.

앵콜 종료에서는 `Tick`이 현재 updateTick으로 컷씬 종료를 확인한 뒤 전투 진입 함수가 아직
commit되지 않은 이전 m_iServerTick으로 같은 deadline을 재검사했다. 정확히 종료하는 프레임에
한 tick 일러 보이는 불일치가 발생했고 실제 Server log에도 Encore combat entry failed가 남았다.
`Enter_KoukuRaidCombat(gate, tick)`으로 검사·preflight·commit이 같은 호출 tick을 쓰도록 했다.
fixed update는 현재 tick, 명령과 투표 경로는 committed tick을 명시적으로 전달한다.

`GameRoom.h`, `GameRoom_GateProgress.cpp`, `GameRoom_KoukuRaidFlow.cpp`,
`ServerGameplayContractTests_KoukuRaid.cpp`를 연결했다. 실제 room.Tick으로 앵콜 종료 직전
대기와 종료 tick의 Bingo COMBAT 진입·참가자 부활·startTick을 1~4인 각각 확인했다.
수정 전91 PASS/4 FAIL, 수정 후95 PASS/0 FAIL이며 관련 TU 컴파일·링크를 통과했다.
근거는 `out/KoukuEncoreTick20260925/{before,after}-receipt.json`과 test.log다.

이 단계의 검증은 제품 소스의 격리 실행이다. 실행 중 Client/Server의 두 Debug EXE는
실제 공유 위반(0x80070020)으로 쓰기 잠금이 확인되었고, 사용자는 계속 편집 중이라고 답했다.
프로세스를 종료하거나 UI를 실행하지 않았다. 제품 실행 파일 링크·사용자 재시작·최종 화면은
아직 완료하지 않았다. 데이터 게시 결과는 아래 별도 항목에 기록한다.

후속 사용자가 Client/Server 종료를 확인했다. 추가 요청의 소스까지 준비하여 한 번에 제품
빌드한다. 교차 검토에서 command drain이 Prepare보다 먼저여서 즉시 부활 시 죽음을 놓칠 수
있음을 확인했다. `Handle_RevivePlayer`는 목적지 admission 후 HP 복원 전에 같은 identity의
solo entrant 기믹을 실패 완료한다. 기존 회귀에 corpse 귀환 직후 Prepare 전 typed 부활 경로를
추가했으며, 아래 최종 빌드·실행 결과와 함께 기록한다.

## G14. 마지막 저장본의 피자 방향·조커 시선·세 번 표적 지정

사용자 마지막 저장본 revision2355에 요청 필드만 병합하여2356을 원자 교체했다.
P25 resetBossYawDegrees126.5→216.5로 시계 방향90도를 더했다. 중앙 grounded teleport는
yaw를 바꾸지 않으므로 시작 시 확정된 이 방향을 유지한다. 위치·소환·Collider는 보존했다.

P13.presentation.55를 기존 양눈을 소유하는
`effect.kouku.gate2.bigsaydon.yellow-eye-warning.full.restore`로 교체했다. occurrence의
중복 bone/roll 보정을 제거하고0–32118ms에 loopEffectToDuration=true로 반복한다.
중복 우안 occurrence.56은 참조가 없음을 확인한 뒤 제거했다. 공유 Effect 문서에 저장된
왼눈[0,.1,.05]/오른눈[0,.18,0]과 각 yaw90을 그대로 사용한다. 해당 DIRECT_AUTHORED_DOCUMENT의
SHA256은85676f8244438ebb28e85bda7d71af73c59b06583da713d3cabe3b52c617b96d이며 변경하지 않았다.
별도 runtime Effect 복사본을 만들지 않고 제품 정본의 다음 spawn 소비를 사용한다.

P13.logic.30/.31/.32는 전용 Logic546 BOSS_RANDOM_TARGET을0/6266/12560ms에34ms 동안
실행한다. 기존 STAGE_3/8/22/16의 retargetOnEnter4개는 false다. 첫 선택은 시작 즉시,
이후 두 선택은 첫·둘째 망치의 복구 종료 후이며 마지막 전멸29085ms 전이다.
공유 placeholder Logic120과 P21/P58/P85 소비자는 보존했다. 기존 P59의28개 수직 넉백
설정·갈고리·Mario FXAA OFF도 이 병합에서 변경하지 않았다.

`out/KoukuVerticalApply20260925/apply_joker_pizza.py`가 최신 bytes·Effect hash를 재확인하고
필드 단위 text patch와 semantic candidate 일치 및 full validate_document를 검증했다.
저장 SHA256은500715538dad4d8cb688dd26b5d76c1a57a1c078659b25ebf5159ffd62e475d2다.
`joker-pizza-applied.json`에 백업과 변경 영수증이 있다. 게시·제품 실행 검증은 아래에 기록한다.

Ctrl+좌클릭은 새 C2S_ROOM_PING/S2C_ROOM_PING을 추가한 protocol113 경로를 사용한다.
기존 packet ID는 유지한다. Controller→typed sink→NetworkManager→실제 Server room의
session/player/world·sequence·HP·navigation 검증→같은 room 전원 reliable 송신→typed
수신 queue→하늘색 Effect3초 표시를 연결했다. Ctrl pending의 과녁 표시는 제거했다.
다른 방은 격리하고 받은 핑은 로컬 사망·속박·focus 변경에 지워지지 않는다.
관련7 CPP 직접 컴파일 후 정식 Product가 만든 Server.exe의 --room-ping-contract-test는
32 PASS/0 FAIL, exit0이다. 동일 위치/frame·죽은 수신자 포함·동일 world의 다른 room 격리,
중복/무효 session/world/좌표·죽은 송신자·8tick 제한 및 후속 정상 요청을 확인했다.
근거는 `out/RoomPing20260925/product-room-ping.log`다.

조커 과녁은 기존 Server target snapshot→Collect_KoukuPresentationViews의 모든 room player
weak character→arena head anchor+0.55m 경로를 사용한다. Server target과 pattern sequence
교체 때 이전 Effect root를 끝내고 새 대상에 부착한다. 로컬 플레이어만 거르는 조건은 없다.
실제 추가한 두 함수를 그대로 추출한 native headless mock11개가 통과했다. 두 client의 같은
remote target·head follow·대상/sequence 교체·대상 사망/이탈·보스 사망·패턴 종료·disconnect·
asset 실패·teardown을 확인했다. 근거는 `out/KoukuJokerTarget20260925/marker-test.log`다.
이 검사는 Client 실행·GPU 표시를 대신하지 않는다.

정식 `Invoke-BuildAndRegression.ps1 -Configuration Debug` Product 빌드는 exit0/PASS다.
Engine→Shared→Server→Client의 컴파일·링크·배포와 기본 runtime data 검사를 완료했다.
실행 파일은 새 코드로 교체됐으며 compile receipt는
`out/BuildPipeline/runs/20260925T123921250Z-debug-product.json`이다(131885ms).
해당 빌드는 데이터를 publish하지 않는다. revision2356의 게시와 그 후 native 검증은 별도다.

revision2356 Composition publish와 Gameplay balance Publish가 모두 exit0이다.
`final-flame-audit.json`은 원본2356→Encounter2356→bootstrap2356의28개 Collider/접촉창/
MAX_HP_PERCENT_DAMAGE10%/PATTERNLOGICPUSH0m·3m·500ms·force/ballistic 일치를 확인했다.
bootstrap SHA256은8def05533e39fe7e02e4f6358d3e352a35eafb195a38e1eaddc544005da5d777이다.
P13 BOSS_RANDOM_TARGET3개0/6266/12560ms와 stage retarget0개, P25 yaw216.5,
P13.presentation.55의저장된노란시선·32118ms loop도 실제 게시본에서 확인했다.
WORLD 원본/게시본2217과 Rendering 원본/게시본84는 여전히 의미상 일치하고 갈고리X0 및
저장된 Effect SHA를 다시 확인했다.

마리오 즉시 부활 보완의 이전 코드에서 추가 회귀3개가 실패했고 최종 게시본과 수정 코드의
4scenario는22 PASS/0 FAIL이다(`green-immediate-product.log`). 정식 Server의 최신
--kouku-bundle-contract-test는99 PASS/0 FAIL이며 신규 랜덤 Trigger의 세 번 선택·직전/사망
대상 제외·한 명 유지·무후보 해제·부적절 수치 거부를 포함한다(`product-bundle-receipt.json`).

앵콜→빙고 실제 재생 확장 회귀는127 PASS/0 FAIL이다. 1~4인 모두 실제room.Tick으로
앵콜 종료→89tick idle→90번째 flow 예약→P129 실제boss/action34ms→다음P108 stage1을
진행했고 board owner/epoch와폭탄·망치timer를 유지했다. 최초 추가 assertion의4실패는
소비 프레임과 공통 예약 시작 tick을 혼동한 테스트 기대값 오류였다. 실제 patternStart,
stageFirstEvaluation, commonStart, LogicLedger 시작을 정확한 firstEntryTick에 고정해 교정했고
제품 기능 코드는 추가 변경하지 않았다. 근거는 `out/KoukuEncoreTick20260925/product-receipt.json`.
수정 테스트 포함 정상 증분 Product 빌드도 exit0이다(20260925T125051830Z-debug-product.json).

이후 사용자 빙고 규칙 확인에서 바닥/보호의 별도 미연결을 발견했으며 G15에서 교정한다.
위 앵콜 진입·연속 재생 PASS를 바닥 규칙과 이난나 보호까지 기존에 완료됐다는 의미로 쓰지 않는다.

## G15. 최종 한 줄 무적·폭탄 전체 반전·이난나 블랙홀 보호

사용자 최종 정정은 새 빨간 가로·세로 한 줄이 완성될 때마다30초 무적이다. 앞선 세 줄
확인은 이 정정으로 대체됐다. Logic525의 단독 소비자가 P107.logic.2임을 확인하고 최신
저장본2356에서 threshold3→1과 표시 이름만 변경하여2357로 원자 저장했다. 기존30초 Result,
타이밍·28개 화염파동 설정·피자·조커·갈고리·Mario FXAA OFF는 보존했다.
`apply_bingo_one_line.py`는 full document 검증, semantic diff 범위 검증, 최신 bytes 비교,
백업·원자 교체와 실패 시 자기 변경만 rollback을 수행한다. 변경 영수증은
`out/KoukuVerticalApply20260925/bingo-one-line-applied.json`, 최종 source SHA256은
db4ebac8816a46b9ec2786b6d30a92403c45d3f0e65c5a594ccc48a9d3257cc8다.

기존 Detonate는 빈 칸을 즉시 빨강으로 만들고 Promote_Lines는 대각선까지 승격했다.
수정은 빨강을 제외한 십자 최대5칸 전체의 검정↔빈칸 반전을 먼저 완료한 뒤 가로·세로10줄만
승격한다. 판 밖은 제외한다. a2~a5 검정→a1 폭탄은 a1 검정/a2 빈칸/빨간 줄 없음이며,
다음 b2 폭탄에서 a2가 채워져 a행이 빨강이 된다. 내부 consumedLineMask는 보상받은 줄의
identity를 보존하고 일반 패턴 교체 때 초기화하지 않는다. threshold1의 새 줄은 해당 폭발에서
모두 소비하며 같은 tick에 한 번30초 보호를 준다. 다른 새 줄은 그 tick부터30초를 갱신하고
같은 빨간 줄이나 지연된 Parent 판정은 보호 시간을 연장하지 않는다. 대상은 같은 레이드의
생존 참가자이며 비참가자만 살아 있을 때는 보상 줄을 소비하지 않는다.

이난나의 기존 승인·소환 경로에30초 iInvulnerableEndTick 적용을 연결했다. 기존4.1초 연출은
유지한다. BINGO_DETONATION은 성공·실패 모두 같은 생존 참가자에 대해 유효 무적만 인정하고
나머지는 encounter wipe로 처리한다. 성공 여부는 보스13줄 피해만 결정한다. 교차 검토에서
기존 성공 분기의 일반 INSTANT_DEATH가 무적 만료 후 실드로도 버틸 수 있음을 발견해 함께
수정했다. 다른 기믹의 encounter wipe와 전역 damage arbiter는 변경하지 않았다.

첫 표식30초, 후속20초, 표식6초+대기2초+fuse4초와 세 번째 표식의 특수 Parent 시작을
유지한다. 특수 Parent는 이동→첫 clip→메두사1회→13초 블랙홀 loop→24.828초 폭발→
32.628초 완료이며 그동안 보드·폭탄·망치 시계는 유지한다. 일반 보스 패턴은 특수 Parent 뒤
중단했던 같은 entry를 처음부터 재생한다. 게시와 최종 제품 실행 검증은 아래에 기록한다.

최종 Composition2357 Publish와 Gameplay Publish는 모두 exit0이다. 원본→Encounter→실제
Server bootstrap의 threshold1/PLAYER_INVULNERABILITY30000 일치는
`final-bingo-audit-2357.json`으로 확인했다. bootstrap SHA256은
fc54b4d235a31c96025c103dbc32a0b9895826a5b89f5f81ff592f0d95a40a97이다.
`final-flame-audit-2357.json`도 PASS로 기존28개 화염파동의0m/3m/force/ballistic 및 피해
10%가 마지막 게시본까지 유지된다. Rendering84와 WORLD2217의 원본·게시본 의미상 일치,
저장된 양눈 Effect SHA85676f82…17b96d도 다시 확인했다.

정상 증분 Product 빌드가 최종 테스트 소스까지 컴파일·링크·배포를 완료했다. 최종 receipt는
`out/BuildPipeline/runs/20260925T131552144Z-debug-product.json`이다. 정식 Server.exe의
`--bingo-contract-test`는137 PASS/0 FAIL, exit0(8.56초)이다. 실제 a1→b2 소비자, 첫 새 줄의
즉시 보상·다른 새 줄 갱신·동시 여러 줄 전부 소비·동일 줄 재지급 방지·Parent consumer의
중복 만료 연장 방지·죽은 참가자/비참가자 제외와 이난나14개 회귀를 포함한다. 이난나는
승인 거절·다른 방 제외·정확900tick 만료·일반 피해·RED0줄 블랙홀·성공 상태에서 실드가
전멸을 막지 못함·다른 encounter wipe 유지까지 검사했다. 근거는
`out/KoukuBingoRule20260925/product-{receipt.json,test.log}`다.

기존 KoukuRaid의 두 주기 회귀를 같은 최종 제품 OBJ와2357 데이터로 격리 실행한 결과는
57 PASS/0 FAIL, exit0(33.64초)이다. 첫 가로1줄과 이후 새 세로1줄이 각각 실제 폭발에 소비되고
정확900tick 무적을 주며, 사용한 빨간 줄이 남아도 재보상하지 않는다. 세 번째·여섯 번째
표식의 특수 시작,10stage·24.828초 폭발·32.628초 종료, 일반 entry의 두 번 재시작,
첫30초·후속20초의 모든 폭탄 deadline을 확인했다. 근거는
`out/KoukuBingoRepeat20260925/product-{receipt.json,test.log}`다. 최종 diff check도 통과했다.

Client/UI를 자율 실행하거나 미저장 draft를 Reload하지 않았다. 새 실행 파일과 게시 데이터는
준비됐으며 사용자 다음 Server·Client 실행에서 소비한다. Client GPU 표시와 최종 플레이 화면의
판정은 사용자 확인으로 남긴다.

## G16. 앵콜 플레이어 카메라와 같은 현재 좌표의 배우·파편

실제 제품 앵콜은 Sequence composition P10/revision171이다. 각 Client의 진입 eye/look/up/FOV를
캡처한 뒤 Server CINEMATIC 종료와 BINGO gate commit까지 고정한다. 원본 카메라 행18.333초가
먼저 끝나도21.322초 시퀀스의 audio tail에서 해제하지 않는다. F6는 override를 해제하며 follow
복귀 때 같은 캡처를 다시 소유한다. 다른 Sequence/최종 엔딩/일반 authoring preview는 유지한다.

WorldSequence TARGET_SET의 optional post-transform을 통해 원본 배우 world를 authored view에서
held view로 옮기고 FOV 비율을 합성한다. 원본 absolute camera/WORLD 데이터는 수정하지 않았다.
14개 부착 V1 track 중 WORLD 입자는 birth basis만 바꾸면2~3초 남은 파편이 과거 카메라에 남는
문제가 있어, 원본 fixed-step simulation/history를 보존하고 완성된 evaluated frame 사본에만
현재 보정을 적용했다. 모델과 파편이 같은 현재 카메라 기준을 쓴다. 옵션 없는 기존 경로에는
frame 복사를 추가하지 않는다. callback은 Level 포인터 대신 immutable pose/cue 값을 캡처한다.

기존 native harness의 --cinematic-view-rebase-contract는 PASS다. 서로 다른4개 viewer pose와
FOV/roll, 실제 앵콜을 포함한3개 authored pose의 point projection 및 부적절 pose의 last-good
보존과 bounded transition을 검사했다. 증거는out/KoukuEncorePlayerView20260925/camera-rebase-test.log.
기존 전체 --valtan-presentation-contract는 앞선 SHAKE payload 검증에서 실패하여 전체 PASS로
기록하지 않는다. 이번 rebase 전에 실패하며 관련 없는 SHAKE 계약은 변경하지 않았다.
Effect의 실제 production pure helper 원문을 사용한9개 focused check도 PASS다. 원본 history/
frame 불변, 기존 WORLD 입자의 현재 변환, 비누적 후합성, carrier/velocity/decal/light/screen좌표,
NaN·singular·overflow의 원자 거절을 포함한다(out/EncoreFramePost20260925/focused.log).

같은 변경의1~4인 실제 room.Tick Encore→BINGO→P129→P108 연속 재생은127 PASS/0 FAIL이다
(out/KoukuEncorePlayerView20260925/product-receipt.json). 이 검증은 source2357 상태이며 아래
G19 데이터 게시 후 다시 실제 진입을 확인한다. 최종 Client 화면·GPU 연출은 사용자가 확인한다.

## G17. 카드미로 W 1초 표시와 평타 쿨타임 오인

카드미로 LMB는 typed W wire slot을 재사용하지만 키보드 W 입력은 차단된다. Server가 평타
전용 reserved ID에400ms를 저장하고, 일반 스킬 map은 absolute end tick을 보존한다. snapshot은
end tick을 그대로 전송하며 Client가 serverTick과 차이를 표시한다. 평타로 일반 W를 감산하는
제품 로직은 없었다. MainApp의 아이콘·원형 표시는Q만 걸렀지만 숫자 루프가 필터를 누락해
평타400ms를 W칸에 ceil=1초로 노출했다.

CombatHUDViewModel::Has_VisibleSkillSlot 정책을 기존 아이콘/원형/숫자 두 경로가 공유하도록
연결했다. MAZE는Q만, 일반 class의W는 기존대로 표시한다. ClientPresentationPrimitive 전체
격리 native 실행 PASS, 실제 CardMaze handler는98 PASS/0 FAIL(47.29초)이다. 연속 두 번
평타는 자신의400ms deadline만 갱신하고 Q 및 Lance Master W34090의 미래 absolute deadline을
변경하지 않는 회귀를 포함한다. 근거는out/KoukuMazeCooldown20260925다.

## G18. 파1빨2 각 Collider의 정확 생존 인원

기존 threshold필드를 Client Apply/serialize/validation/UI, Python projector, PS bootstrap,
Server parser/Brain/LogicRuntime에 연결했다. INVULNERABILITY_ZONE의0은 기존 제한없음,
1~4는 각 Collider별 정확한 판정 가능 생존 인원이다. 집계 후 현재 pattern의 보호 집합만
만들고 인원 부족·초과·영역이탈·사망 때 즉시 보호와 새 pulse를 중단한다. 다른 원과 인원을
합산하지 않으며 기존 이난나·빙고의 iInvulnerableEndTick를 삭제하지 않는다.

기존 실제 LogicRuntime helper35 PASS/0 FAIL, Python focused4 PASS, Client typed Apply→
Save→Reload의0/1/2/4 보존 및5 거절 시 memory/disk rollback PASS다. 관련 CPP7개 개별
컴파일과 PS parse/diff check도 PASS(out/ExactZone20260925/change-receipt.json).
source에는 Logic101 threshold1을 파랑7/9/10에 유지하고 새Logic547 threshold2를 빨강8에
연결했다. V2 asset내부Z+2.5m를 고려한 기존 Collider 위치/반경2m와gaze timing은 보존했다.

## G19. 무지개20줄과 분신·십자 화염의 누락 연결

fresh source2357에서 G18과 G19를 함께 stable필드73곳의 교체/추가로 원자 병합하여2358을
저장했다. SHA256은2b0ef536564e9dbb9edeacec81e8c6878ceed68f2820dde527cf81e5e4ccc5d8이다.
적용script는before bytes·영향필드·변경Pattern집합·증거hash를확인하고backup/CAS/원자교체와
실패시자기변경rollback을수행한다(out/KoukuFinalPattern20260925/applied.json).
P11과9개Pattern38/40/50/53/54/55/60/108/112만 변경했으며 이전 P59화염28/피자/조커 및
WORLD·Rendering·FXAA 설정은 변경하지 않았다.

P38은2개 Effect 각각45도5줄+135도5줄,총20줄인데Collider10개뿐이었다. 기존10개TRS/scale/
resource와Result538(최대HP10%,수평4m,1000ms,높이2m,force/ballistic)을보존하고source Wave의
시작시각에창1초를맞췄다. 두번째 Effect용10개를복제하여각줄이같은판정을받는다. 원본정확
sourceActionId/sourceStageId와시간변환으로기존 SOUND10개의event/start를대조해전부일치하고
실제media/volume도정상이었다. 같은clip을쓰는silent stage004에다른stage의장시간보이스를
추가하지않으며사운드변경은없다.

기분나빠P50/53/54/55/60은원래불Effect3203~7482ms중Collider가첫1초만존재했다. 기존불뿜기
지속불 Logic513(100ms)→Result515(100HP)+544(명시광기0)로전체Effect수명을연결했다. 별도
불뿜기 knockback/pulse540~543은그대로다. BingoP108에는동일front Collider를추가했다.
십자P40은기존3visual과4box를유지하며불1154ms에반복판정을맞췄고,SOUND만있던BingoP112에
그3visual/4box를복원했다. clone추가나animation변경은없으며P63/64/71/72는보존했다.

전체문서validation및실제projector의28windows/34regions검사PASS다. rainbow20개각1region/
동일538,fire8windows의100ms/100HP/광기0와P40/P112각4region공유verdict를확인했다.
clone도기존BossSimulation의동일LogicRuntime를소비하므로새runtime경로는없다.
근거는out/KoukuFinalPattern20260925/consumer-projection.json과REVIEW.md다.

G16~G19 최종제품코드는정상증분Debug빌드/링크/배포PASS다. receipt는
out/BuildPipeline/runs/20260925T134828976Z-debug-product.json(115OBJ,1binary)이다.
현재2358게시를수행중이며설치된Encounter/bootstrap와최종Server회귀결과는아래에추가한다.

2358의공식Composition게시와Gameplay게시모두exit0이다. 그후새로추가한빈줄2곳의공백만
제거하여JSON값/revision은그대로이며최종source SHA는
99b1b1bed01113916add71495b57c6db88e0fe7f1b4100dc34ba20e438091fa7이다.
source-whitespace.json에before/after와semantic equality를기록했다. 최종diff check PASS다.

설치소비자audit는원본/Encounter/Client patternbindings/Server bootstrap모두2358이고,
4개무적구역threshold[1,2,1,1],G19 28windows/34regions,무지개20개동일538,지속불8windows의
100ms/100HP/명시광기0,십자G3/Bingo각3visual/4region과기존사운드10개를확인했다.
`installed-audit.json` PASS/exit0이며P59화염28개의0m/3m/force/ballistic/10%도
`final-flame-audit.json` PASS다. 게시Encounter SHA는e280b88f6658c5afba9c9d693c1f6feac0f2c2ca9aa1a0713385e4b1a2dd5567,
patternbindings는ceab5a80cae6a70ec626b48c38bfd17aa762171f023dfe73b7e6fd9b0e72cc02,
bootstrap은1e35464ce2c2c2d803a0ad5bb09f8c509fff86c987b064a42d18a55aeb9c91e1이다.

최신Product OBJ와2358게시데이터로actual Encore fixture를다시실행하여150 PASS/0 FAIL,
exit0(34.99초)이다(out/KoukuFinalEncore20260925/product-receipt.json). 정식Server의
--bingo-contract-test도137 PASS/0 FAIL,exit0(8.97초)이다
(out/KoukuFinalBingo20260925/product-receipt.json). 이전결과로그는덮어쓰지않았다.
Server/Client다시실행가능상태를사용자에게안내했으며,Client/UI는자율실행하지않았다.
사용자의플레이어시점연출·이펙트표시·최종육안확인은별도다.

## G20. WORLD 본 Collider 중복 계산 제거와 실측

기존 publication session memo에 WORLD 본 Collider track을 연결했다. resolved root, 전체
sequence 문서 내용, world/box/collider 전체 값을 key로 사용한다. pinned JSON은 세션 안에서만
digest를 재사용하고 다른 mutable 입력은 매번 값을 계산한다. 계산 실패는 저장하지 않으며
반환값 deepcopy, session 종료 시 초기화, native 입력의 최종 exact-byte freshness 검사는 유지한다.
원래 sampler body를 그대로 분리한 제품 변경은14줄이다. 별도 worker나 persistent cache는 없다.

동일 source2358과 측정 wrapper로 공식 publish를 실행한 결과 exit0,464.356초에서170.119초로
줄었다. 약63.4% 단축,2.73배이며 Composition 게시 단계만 측정한 수치다. UI Publish 전체의
Map/World/Gameplay domain 시간까지 포함한 수치로 설명하지 않는다. 기존 domain owner도
receipt가 유효하면 재사용하므로 Map publish가 매번 강제로 실행된다고 해석하지 않는다.

WORLD 본 track 요청240회 중 실제 계산은20회, memo hit220회였다. 해당 경계는369.448초에서
42.017초로 줄었고 실제 계산 body는40.106초다. 남은 큰 경계는 object collider45.788초다.
전체115개 product pattern,9개 bundle,577개 stage의 기존 closure/admission/validation과
원자 게시를 모두 유지했다. 관문 물리 분할이나 검사 범위 축소는 이번 수정에 포함하지 않았다.

실제 native fixture의 cache 동작5개와 기존 exact-input/rollback4개, 총9개 테스트가 PASS다.
입력 변경 시 miss, 반환값 오염 격리, 실패 후 재시도, 실행 간 격리, 동일 metadata의 JSON/native
내용 변경 시 최종 거절을 확인했다. 별도 읽기 검토에서도 key 누락이나 freshness 우회는 발견하지
못했다. 근거는out/KoukuPublishProfile20260925/world-bone-cache-tests.receipt.json이다.

최적화 게시 후 Encounter, patternbindings, 기존 Gameplay.bootstrap의 SHA256은 위 G19 최종
hash와 모두 byte 단위로 동일하다. source 역시 위 G19 최종 hash를 유지했다. 결과 데이터 변경이
없으므로 이 Python 수정 때문에 Product EXE를 재빌드하거나 Gameplay를 다시 게시하지 않았다.
측정 근거는profile-publish-20260925T140337581292Z.{json,txt}, 최종 동일성/parse 검사는
out/KoukuFinalPattern20260925/optimized-parity.json이다. 실제 Client 화면 검증은 여전히 별도다.

## G21. 사용자 Complete Play 준비 대기 진단 — 제품 변경 없음

23:13의 Gate1 Complete Play는 Client5948/42860 두 명의 공유 준비 단계였다. owner는23:13:25
START 후23:13:50 READY를 보냈고 Server의 ReadyMask는1/참가자2로 기록됐다. peer는 아직
READY를 보내지 않은 상태에서23:14:52 owner STOP으로 종료됐다. 저장 Action2358, Sequence171,
게시본과 bootstrap hash는 양쪽 모두 일치한다. 이 기록만으로 Gate1 영구 교착을 확정하지 않는다.

같은 실행의 다음 epoch2 Gate3는 owner23:16:41, peer23:17:06 준비를 완료했고 양쪽23:17:07
CINEMATIC,23:17:56 COMBAT에 진입했다. 사용자는3관문 진입 및 현재 테스트 중임을 알렸다.
Gate1의 준비 완료 후 재시도는 아직 사용자 확인 전이다.

현재 Debug 진입은 이펙트를 지연 준비하며 Complete Play가 전체 raid dependency를 요청한다.
peer의 V1 준비는 계속 진행됐고 단일 시네마틱 Effect 준비19.516초, 준비 중 main-thread
13~26초 정지 기록이 있다. 최소화 창도 Update를20Hz로 수행하므로 숨김 자체를 무진행 원인으로
기록하지 않는다. MainApp은 owner ACK 이후 구체적인 준비 인원 대신 일반 대기 문구를 표시한다.
Server는 모든 참가자 READY 전에는 재생하지 않는 기존 계약을 지키고 있으며 이번 진단에서
barrier를 완화할 근거는 없었다. 진단 중 제품 코드·데이터·EXE 변경이나 Client/UI 조작은 없다.

## G22. 3관문 LUT01 복구와 색감 적용 경계

사용자가 LUT01을 먼저 복구하고 환경광은 비교 후 판단하도록 승인했다. Rendering 원본84를
다시 읽어 compare.gate3의 qualityOverride와 g1.base/g3.dark/source-rendering의 region48에
과거 rev80의 lv_lut_midnightc_lut_01.dds 연결 네 곳만 복구했다. revision은85다. 그 외 모든
JSON 값과 source의 무관한 바이트를 보존했다. base 노출2·scene multiplier0.5, G3 ambient0,
Bloom OFF와 Mario FXAA OFF도 그대로다. LUT DDS 실물 존재를 확인했다.

후보를 공식 Rendering publisher로 검증/게시한 뒤 최신 source/runtime hash를 다시 확인했다.
백업과 원자 교체로 source를 설치하고 공식 publisher로 runtime을 게시했다. 최종 두 문서는
의미상 동일하며 네 LUT scalar와 revision만 달라졌음을 대조했다. publisher exit0 및 대상
diff check PASS다. 원본 SHA256은bf0f1964766f4d9955a0324ea580873a93408361a90a36d46b7025b1c5df7201,
runtime은16030dd3c2baeb22b9147e39faa5cd68b15d879182158a108c81390464fb7929다.
근거와 적용 전 백업은out/KoukuLutRestore20260925/의candidate-receipt.json,
install-receipt.json,final-verification.json,source.before.json,runtime.before.json에 있다.

현재 LUT는 SceneHDR의 배경·캐릭터·NonLight/Blend Effect를 합성한 뒤 최종 RGB에 적용된다.
따라서 조명을 안 받는 Effect도 LUT의 색 변환을 받으며, light receiver 조정과 LUT 선택적
제외는 다른 문제다. LUT 제거는 특정 대상의 파란 기운뿐 아니라 배경의 전체 색감도 바꾼다.
현재 G3 밋밋함의 각 원인 기여나 캐릭터 색 보호의 최종 해법은 사용자 화면 비교 전 미확정이다.

이동 외곽의 읽기 조사에서 노란 hit는0.12초 뒤 해제되며 연속 피해만 갱신한다. 신규 Kouku
SourceTrail은action4219776 돌진에만0.2초 간격/0.4초 수명/alpha0.6으로 발생하고, 매 프레임
비활성부터 재판정하며 기존 잔상도 수명 만료로 제거한다. 일반 이동으로 설정이 유출되는
경로는 발견하지 못했다. 노란 hit가 현재 이동 현상의 원인이라고 확정하거나 shader를 바꾸지 않았다.

데이터만 변경했으므로 EXE 재빌드는 수행하지 않았다. 실행 중 Client/UI·메모리 draft는
조작하지 않았다. 사용자가 F1 Rendering의 Authoring Pipeline → Reload Runtime을 누르거나
Client를 다음에 실행하면 게시본을 읽는다. LUT 복구 후 실제 색감과 이동 외곽 비교는 사용자 확인이다.

## G23. 호버 판정과 빨간 외곽선

사용자는 이동 중 외곽 변화가 호버임을 확인했다. 기존 전체 pose AABB 선택을 현재 skin
palette의 삼각형 교차로 좁히고 표시되는 body/hat/weapon만 검사한다. CPU mesh 입력은
prototype/clone이 공유한다. 이 판정은 기하 교차이며 텍스처 알파 구멍의 GPU pixel picking은 아니다.

기존 mesh 직후 hull draw는 뒤의 바닥에 덮였다. CNpc는 기존 DEFERRED_OVERLAY에서
visible silhouette을 stencil0x80에 표시한 뒤8방향4px offset 외곽을 그리고 사용한 bit만 지운다.
기존 shader pass18/19는 유지하고20/21을 추가했다. 노란 hit와 LUT/조명 값은 바꾸지 않았다.
Mesh/Model, Npc/DeferredMaterialRenderUtils/ClientReplication, animated shader 및 기존 probe의
11개 파일을 변경했다. 별도 모델 renderer나 새 프로젝트는 추가하지 않았다.

최소 C++5개와 FXC main/group25 컴파일을 통과했다. WARP slab fixture에서 옛 draw 순서는
바닥 뒤 red0, 수정은 움직이는 camera5위치에서484~492pixels였다. 내부0, depth 가림,
alpha discard, D24S8 전체 바이트 및 기존 stencil5 보존을 확인했다. 이것은 실제 아레나 화면
검증이 아니다. 증거는out/HoverFix20260926/gpu-receipt.json과gpu-run.log다.

새 Product Engine/SDK와 실제 BossCatalog preScale(Saydon/Kouku.017,Valtan.0001)을 사용한
3모델×2pose×400ray도 exit0/PASS다. 표면923개를 선택하고 AABB 안의 빈 공간1477개를
거부했다. 비균일 root의 world 거리, 바깥/zero/singular/NaN 입력과 miss 출력 보존을 확인했다.
Debug CPU fixture의 평균 query는Saydon2.37~2.43ms,Kouku.40~.43ms,Valtan.30~.32ms다.
이는 전체 게임 FPS 수치가 아니다. 최초 fixture는 Valtan preScale을 빠뜨려 거대한 단위의
float 오차로 실패했으며 tolerance를 넓히지 않고 제품 catalog 경로로 고쳐 재검증했다.
제품 소스 추가 변경은 없고 Product CSO도 복사/실행하지 않았다. pick-receipt.json에 기록했다.

## G24. 빙고 소품 크기와 본체·심지

F1 Bingo Board 재생 버튼 밑 Bomb Size/Hammer Size/Save를 기존 WorldObjectTool의 resource
scale과 Save_Source에 연결했다. 다른 미저장 Object 편집도 같은 문서로 저장되고 freshness
거절·공식 publish를 유지한다. 값은 다음 재생에서 소비하며 미저장 draft가 있으면 Reload Saved를
수행하지 않는다. 망치 scale 변경 시8개 선행 warning의 local offset/scale을 역보정하여 월드
범위를 유지한다. 기존 scale1.8/rootY9로 바닥 아래였던 warning pivot을Y0.15로 올렸다.

MARKED 머리 위 폭탄은 planted와 같은 bingo_bomb.original CModel을 사용하고 원본 심지를
함께 재생한다. carrier의 Server snapshot 위치에Y2.4로 부착하며 phase/carrier/death/reset
변경 때 WorldSequence owner가 모델과 V1 root를 같이 정리한다. resource 준비와 실패 재시도를
기존 계약에 연결했다. V2가 animatedMesh를 지원하지 않는 것이 원인이라는 주장은 하지 않는다.
planted motion/fuse의3000ms를 Server의4000ms에 맞췄으며 폭탄5칸 판정은 바꾸지 않았다.

사용자의 저장/종료 및 반영 승인 뒤 최신 world2217에서 stable ID 필드만 합쳐2218로 교체했다.
교체 전 hash 재확인·실제 교체 backup 대조·원자 교체·실패 시 자기 바이트 rollback 절차를 사용했다.
공식 Map WorldSequences Validate/Publish 및 Kouku projector Publish가 exit0이며
source/runtime SHA256 모두628a8a89f1530284f7557c10ef88ebe6bf60bc45924a0d1c11e9c8a476ceb37e다.
LUT85 source/runtime hash는 G22 값과 동일하다. 관련 리소스2개/템플릿6개/인스턴스25개의
실제 native codec31검사에서 Load/Validate/Save/Reopen, hammer.05/50 warning불변, marked LOOP,
4000ms fuse, invalid save의 기존 파일 보존을 확인했다. UI 실행·GPU 실제 소품 표시는 사용자 확인이다.
증거는out/KoukuBingoVisual20260926/{install-receipt,installed-verification}.json 및native-review/다.

## G25. 보호 폰트·77줄·최종 엔딩 편집

블랙홀 BINGO_DETONATION의 유효 무적 보호 분기에서 기존 invulnerability contact/pulse를
폭발 tick에 게시한다. Client는 파1빨2와 같은 무적 폰트 경로를 사용한다. Retail 실제77줄/HP는
이미 연결됐지만 Client HUD가 읽는 base BossProfiles가160줄이었다. 이 필드만77로 수정하고
공식 provenance 동기화1필드 및 Gameplay Publish를 완료했다. Server는HP1868133028/77줄이다.

정상 Raid의 최종P9→관문 클리어→MVP 순서는 유지한다. F1 standalone Bingo 처치가 즉시
클리어하던 경로를 기존 Begin_KoukuRaidPreparation의 전원 READY와 정확한 P9로 연결했다.
G3의 기존5초 hold는 유지하며 Bingo 최종 연출은 같은52042ms 종료 뒤 gate4 clear/MVP를 보낸다.

사용자 확인에 따라 최종 P9의4개 배우 animationTracks를25개 박스(11/4/4/6)로 나눴다.
원본 혼합·얼굴 제어가 들어간 installed baked WANM을 그대로 사용하고 clock은 기존과 같다.
137개 native channel 표본에서 최대 float32 tick차이는0.000061035(약2µs), 마지막1473tick은
동일했다. 자동 반복/시간 연장/사운드 이동은 하지 않았다. saydon1/2와kouku1/2는 원본의
서로 다른 actor와 등장 구간이며 중복 생성 오류가 아니다. P10 앵콜 진입은 수정하지 않았다.

Animation Clips의 시작/sourceStart/속도 및 Duplicate로 구간을 조절할 수 있다. Duplicate는
해당 actor의 후속 animation track만 밀며 WORLD transform, 다른 배우, camera/audio,
상위 Composition duration을 자동 조절하지 않는다. ClipLoop는 분할 구간 반복이 아닌 기존
baked clip loop다. 원본12966.862ms 인근 동작을 늘릴 때 사용자가 이 타이밍을 함께 편집해야 한다.
원본 대조/편집 제약/수치 증거는out/KoukuEndingTimeline20260926/의P9_EDITING_READONLY.md,
source-clip-timelines.json,split-pose-audit.json에 있다.

이번 Gameplay 게시 SHA256은f34943e5c2f5f91b581c6abb4c2d67fbb7087eff178c038446267df7da33372e다.
사용자가 저장한 Sequence175를 네 RAIDGATE 행에 pin한 것만 이전171과 다르다. 이 네 필드를
메모리에서171로 바꾸면 기존1e35464... hash와 byte 동일함을 확인했다. 다른 runtime balance
행은 보존했다. P9 원본·게시 clearDurationMs 모두52042이며 새 테스트도 이 게시값으로
종료 tick을 계산한다. 근거는out/KoukuBingoFinalClear20260926/gameplay-hash-difference.json.

새 Product Server runtime OBJ를 연결한 집중 회귀는242 PASS/0 FAIL/exit0(35.174초)다.
1~4인 정상 Raid와 실제 typed F1 재소환에서 P9 완료 전 clear/MVP 부재, completion tick에
Sequence→gate clear→참가자별 MVP 정확1회를 확인했다. 기존 Encore→Bingo P129→P108
실제 tick 재생도 포함한다. 정식 Server의 --bingo-contract-test는140 PASS/0 FAIL이며
빨간 새 줄/이난나 보호 pulse, 만료·비참가자 경계를 포함한다.

최초 집중 fixture의28실패는 runtime 결함이 아니라 비어 있는 nickname 때문에 MVP 직렬화가
거부되고, 내부 Spawn_GatePlacement 호출로 실제 F1의 clear-bit 초기화가 빠진 것이었다.
유효 nickname, 실제 Handle_SpawnWorldEntity, 매 tick outbound decode/drain을 적용해 원래
순서·정확히1회 조건을 유지한 채 재검증했다. coalescing 때문이라는 초기 의심은 취소했다.
수정은 ServerGameplayContractTests_KoukuRaid.cpp의 fixture에 한정했고 원래 실패 로그는
failed-initial-*로 보존했다. 최종 증거는out/KoukuBingoFinalClear20260926/의
product-receipt.json,product-bingo-receipt.json이다. 실제 Client 폰트·카메라·MVP 화면 판정은 별도다.

### G23~G25 최종 제품 빌드와 설치 확인

정상 Debug Product Build/Deploy를 완료했다. Engine→Shared→Server→Client 모두PASS,
Client는205 OBJ·24 CSO와 EXE를 생성했다. 제품 셰이더는 /O1 최적화를 포함하므로 격리
/Od 최소 컴파일43초와 시간을 직접 비교하지 않는다. 첫 Client 단계는850.262초다.
이후 fixture 정정분은 같은 정상 증분 Product Build로 Server OBJ1개만 다시 컴파일했으며
Client OBJ/CSO 재생성은0개다. Clean/Rebuild나 실행 중 프로세스 강제 종료를 사용하지 않았다.

최종 빌드 증거는out/BuildPipeline/runs/20260925T155707272Z-debug-product.json,
전체 재컴파일 증거는20260925T155653089Z-debug-product.json이다. 기존 PhysX PDB,
문자 인코딩 및 native material shader 경고는 남지만 컴파일·링크 오류는 없다.
최종 Server.exe SHA256 d5acfd1632b900765d456060adc1186a47a8cae1b263a552b6e4d2da288c7ed2로
정식 Bingo140검사를 다시 실행해PASS/exit0(8.544초)를 확인했다. 설치된 world/source 일치,
Client·Server77줄, LUT85 양쪽 hash 보존, 변경 프로젝트 XML parse 및 git diff --check도PASS다.
최종 파일 반영과 게시·빌드는 완료했으며 Client/UI/메모리 draft는 자율 실행·조작하지 않았다.

## G26. WORLD 원본 클립 편집과 손 뻗기 후보 검증

G25의25개 박스 분할은 기존 baked 시계를 그대로 나눴으며 원본 animation key의 되감기를
수정하지 않았다. 이 동등성 검사만으로 손 뻗기 동작을 복구했다고 설명할 수 없다. 실제
원본 SCENE01B B 키의12966.862ms 전환에서 source7633.528→7009.368ms로 되감기며,
설치 WANM389→390 프레임의 오른손 local 회전은15.718도였다. 원작 GPU의 추가 혼합
동작까지 확인한 것은 아니다.

WORLD 상세에 Edit Animation Clips와 Animation 정보 행 더블클릭을 연결했다. 기존 Object/
Motion을 열어 실제 모델 clip 교체, Source In/Out, 범위 loop/hold, Clip Start/End 및 분할을
편집한다. 다른 Character owner나 배우를 추가하지 않는다. sourceEndMs는 optional0이면
기존 끝 의미이며 codec, Client pose/부착 FX, Map/Composition 검증과 본 Collider 투영에
같은 범위 계약을 연결했다. Split은 loop를 끈 움직이는 원본 구간에서만 허용한다.

실제 WorldSequenceDocument codec/sample147 checks, Map/Composition 및 본 Collider32 tests,
실제 WorldObjectTool 편집 함수12 cases/44 checks가PASS다. 편집 검증은 실제 헤더와 함수
본문을 사용하고 UI 및 catalog 획득만 metadata fixture로 대체했다. 잘못된 범위·종료·없는
클립의 Split·loop Split·held tail·NaN·다음 경계 침범 거절과 draft/dirty/revision 보존,
다른 배우와 Effect 유지 및 인스턴스 배속을 포함한 Split 시계를 확인했다. 관련 Workbench,
MainApp, WorldObjectTool, WorldSequencePlayer와_Objects 최소 TU compile이 모두PASS다.
증거는out/WorldAnimationClips20260926/editor-native-receipt.json 및
out/KoukuWorldClipTrim20260926/verification.json이다.

out/KoukuHandReach20260926에 독립 `kouku.bingo.ending.handreach` 후보를 만들었다.
12,900~16,400ms만3,500ms clip으로 연결하며52개 원본 제어와168개 bone 채널을 유지했다.
원본 source clock을 양끝 속도가 연결되는 단조 clock으로 교정해 오른손 최대 frame 회전은
15.718→0.814도, 팔 위쪽15.375→1.576도, 팔꿈치15.294→1.246도다. 양끝168개 bone key와
기존 모델·재질·모든 기존 animation payload는 동일하다. 자동 반복과 전체 연장은 추가하지
않으며 P9 WORLD49083ms/Server52042ms, 다른 배우·camera/audio 시계는 유지한다.

후보 WORLD는 stable sequence와 기존 slot/start/clip으로 두 animation 행의7개 필드만 병합한다.
최초 rev2220 후보는 공식 Map publisher의 입력 경로만 out 후보로 바꾼 Validate/WorldSequences
검사를 통과했다. 사용자가 이후 저장한 rev2231에서도 같은7필드 병합이 가능했다. 최종 저장
후에는 다시 최신 hash/필드를 읽고 검증한다. 이 후보 검증 시점에는 Resources·Data 설치와
publish/Product 링크가 대기 중이었고 실행 중 편집은 보존했다. 아래 최종 적용에서 설치·
게시·빌드 완료를 기록하며, 사용자 화면 확인은 별도다.


## G27. Parent 시간 편집과 전방 배우 회전 수정

기분나빠/알비온 저장 거절은 자식의 새 길이와 기존 complete Parent 참조 길이가 달라져
Commit validation에서 막힌 것이다. publish 실행 전에 발생하는 오류이므로 publish 시간
최적화가 원인이라고 단정하지 않는다. complete sequential/loop 참조만 자식 길이 변화에
맞춰 갱신하고 기존 간격과 뒤쪽 시작점을 유지한다. 고정 부분 구간·다른 사용자 변경은
보존하며 충돌/overflow는 candidate 전체를 거절한다. 오류에는 실제 Parent/child ID와
예상/현재 시간이 표시된다.

기분나빠 전방 source의 접촉 시작3203ms에34ms BOSS_TRACK_TARGET 하나를 연결한다.
해당 actor ledger에서 가장 가까운 살아 있는 Raid 참가자를 한 번 선택하고 real/clone의
실제 pose와 packet yaw에 반영한다. 다른 세 방향과 Parent537은 유지한다. 현재 저장본은
clone이1600ms에 끝나므로3203ms 공격까지 남는 실제 전방 배우만 해당 회전을 실행한다.

실제 Document/Workbench 회귀6 groups가PASS다. P52 13009→9500과 Parent P92 재조정,
알비온과 양쪽 Parent, 빈 간격·부분 구간 보존, atomic 거절, front Trigger 저장/재로드 및
빙고 Loop의 board 종료 갱신을 포함한다. 실제 Server44 checks, Preview sampler14 checks,
Python 방향 projection5 tests도PASS다. 증거는out/KoukuParentTiming20260926와
out/KoukuFrontFacing20260926에 있다. 검사에서 사용한9500ms를 사용자 저장본에 자동
적용하지 않았다.

## G28. 일반 칼날 offset·즉사 칼날2배와 아이언 메이든 정합성

사용자가 publish 진행 중 Save가 거절됐음을 확인하고 일반 칼날X=-0.42, 즉사 칼날2배,
아이언 메이든을 세이튼 정면에 배치하고 플레이어 포박/도착점을 함께 맞추도록 명시했다.
첫 scale key만2이고 나머지173개가1→3 곡선이었던 것이 일정 크기로 보이지 않은 원인이다.
동일 motion을 다른 시작점/yaw에서 재사용한 둘째 칼날은 기존 감옥을 지나 다른 위치로
갔고, 표시 감옥과 Logic89 anchor도 약1.67m 떨어져 있었다.

기존 보스→감옥 거리19.969795745m를 유지한 정면점
(-5.814691867,1.320000052,934.460853292)을 감옥 FX와 Logic89에 함께 적용한다.
기존2m/s 속도와 두 occurrence의 시작시각·창·배치는 유지한다. 첫 칼날은9985ms,
둘째는8866ms에 같은 감옥XZ에 도착하고 이후 hold한다. 두 motion의 전체175개 key를
2배로 맞추며 둘째 occurrence만 독립 motion/instance/WORLD definition으로 연결한다.
일반 칼날은 Effect X offset만-0.42이며 기존2→3 크기 곡선은 보존한다.

후보의 실제 WORLD collider를 사용한 Server75 checks가PASS다. 3·4인 포박 위치,
두 칼날의 물리 접촉 사망, STAGGER 성공 시 두 hazard 중단·포박 해제·생존을 확인했다.
중앙 도착 사망 타이머나 새 runtime 경로를 추가하지 않는다. 기존 KoukuRaid 검사의 옛
anchor 상수는 typed Logic89와 navigation 결과를 확인하도록 교정했다. 증거는
out/KoukuMarioBlade20260926/mario-blades-candidate-*다.

최종 후보는 최신 WORLD2268→2269, Action2364→2365에G26/G27/G28 필드만 병합했다.
독립 전체 semantic diff에서 그 외 P9 배우·audio·rendering·모든 기존 occurrence의
시작/창/배치를 보존함을 확인했다. 공식 Map WorldSequences Validate는PASS이며 설치와
전체 게시 상태는 아래 최종 적용 항목에 기록한다.


## G26~G28 최종 설치·게시·제품 검증

최신 WORLD2268와 Action2364의 hash를 재확인하고 Resources WModel 및 두 source 문서를
백업·원자 교체했다. WORLD2269/Action2365와 손 뻗기 모델SHA714c247f…가 실제 설치됐다.
7개 hand animation 필드, 전방 Trigger 및 두 칼날/감옥 필드 외의 semantic 값은 독립 비교로
보존을 확인했다. 설치 영수증과 원본 backup은out/KoukuFinalApply20260926에 있다.

공식 Map WorldSequences publish, Kouku composition publish, Gameplay balance publish가
모두PASS다. 전체121개 저장 패턴과 ordinal 검증, P33→P42 실제 projection, 두 칼날의
scale2/정확한 도착점 검증도PASS다. 최초 Gameplay publish는 PS bootstrap의 옛 child
trigger 전면 금지 guard에서 거절됐고 생성 bootstrap은 유지됐다. Cross Direction과 일반
Summon이 공통 narrow helper를 사용하도록 고친 뒤 재게시했다. 실제 PS56 checks와 기존
Python actor-local projection2 tests가PASS다. 일반 Summon의 Albion 예외는 유지한다.

정상 Debug Product 빌드는 Engine→Shared→Server→Client 모두PASS다. 첫 빌드는
Client187 OBJ/0 CSO를 생성했고113.373초 걸렸다. 정식 fixture의 고정 anchor 기대값 교정은
같은 Product 증분 빌드로 Server만 다시 반영했다. 기록은
out/BuildPipeline/runs/20260925T170858709Z-debug-product.json 및
20260925T171018149Z-debug-product.json이다. Clean/Rebuild나 사용자 EXE 강제 종료는 없다.
PS-only guard 교정은 새 EXE 컴파일이 필요하지 않다.

정식 게시 bootstrap으로 후보 geometry/anchor 주입 없이 실제 Server 실행 경로75 checks가
PASS/0 FAIL/exit0(13.026초)다. 3·4인 포박 위치, 두 칼날 접촉 사망, 무력화 성공 시 두 hazard
중단·포박 해제·생존을 확인했다. 증거는
out/KoukuMarioBlade20260926/mario-blades-product-receipt.json이다.
추가 정식 --kouku-raid-contract-test 광역 검사는 다른 관문 반복까지 포함해 길어져 작업이
직접 생성한 headless process만 중단했다. 그 부분 로그는 완료/PASS로 계산하지 않으며
out/KoukuFinalApply20260926/optional-raid-test-status.json에 별도 기록했다.

최종 Client.exe SHA c7f3fbf2fea846ab1652184e4c3d76f8aa0be286def93b810239ef9c7e22b5ce,
Server.exe SHA 3c60df9cdbf5614e4fcaafe0101350a78f27263ba2aff28955308d931f3066b5,
Gameplay.bootstrap SHA 26680d699c95225875bff016d5079ba68e4548a18d3db216ca3a7bf0768500a9다.
WORLD source/runtime 전체 동등성과 두 projection의 sourceRevision2365를 확인했다.
렌더링 authoring/runtime hash는 작업 전과 같아 LUT·FXAA 설정을 보존했다. 변경 JSON22개와
project/filter XML4개 parse도PASS다. 최종 근거는
out/KoukuFinalApply20260926/final-verification.json 및evidence-index.json이다.

파일 설치·게시·빌드와 위 구조/실행 수치 검증은 완료했다. Client/UI를 자율 실행하거나 Reload
하지 않았으며 최종 크기·위치·손 뻗기 연출 화면과 사용자 편집 감각은 사용자가 확인한다.

## G29. 세이튼 등장 원본 안내 음성 추가

원본 SCENE02A의 subtitle cin.37081_29_01와
s_scene_ocean3_3.scene_midnightc_ed_appeargiantsaton 이벤트를 대조했다. Wwise bank2755353966,
event3575728479의 Korean media120512111은 요청한 두 문장이 들어 있는11.552018초 PCM이다.
기존99665081은3.366667초 출현 SFX로 그대로 유지했다. 설치 전 Sound 전체5304개 파일의
이름과 무관한 파일/PCM 비교에서 동일 음성은 없었다. 현재 누락은 확인했지만 어느 변경에서
삭제 또는 덮어쓰기 됐는지는 입증하지 않았다.

사용자의 원본 추가·GRResources 전달 요청에 따라 아래 동일 상대 경로에 원본을 설치했다.
`Sound/KoukuSaton/Events/scene_midnightc_ed_appeargiantsaton.voice.120512111.wav`
실제 위치는 프로젝트 Client/Bin/Resources와 Desktop/GRResources2다. 두 설치본과 추출 원본의
SHA256은2db988691a467a29df4ed02b14faf57a70083eddd8035157eda9176ae7aba1e2로 일치한다.

최신 Action2375를 다시 읽고2376으로 필드 병합·백업·원자 교체했다. Sound Resource
`sound.kouku.scene.appeargiantsaton.voice.120512111`을 “세이튼 등장 원본 대사 / 뿅망치 살인마”로
등록하고 CharacterSoundCatalog의 원본 이벤트에 단일 Korean WAV를 연결했다. 대형세이튼 등장
P9.presentation.10은50ms에서11553ms 동안 source0부터 재생하며 P9 duration은11603ms다.
기존3개 Stage2647/2667/2667ms와996 SFX, 자막, 애니메이션, 다른 패턴, 배열 순서는 모두 보존했다.
설치본에서 요청 필드를 역적용하면 백업과 semantic equality가 성립하는18개 독립 검사가PASS다.

공식 composition publish와Gameplay balance publish가exit0/PASS다. 두 projection의
sourceRevision2376, presentation duration11603/stageDuration7981, bootstrap의
PATTERNTIMELINE P9 11603과 실제 WAV·catalog·resource·cue 연결을 확인했다. JSON parse 및
변경 대상 git diff --check도PASS다. C++ 변경이 없어 새 EXE 빌드는 필요하지 않았다.

bundle의 저작/Preview 길이는9950→11603이지만 Server Flow는 실제 Stage 완료 receipt로
다음 entry를 진행한다. 기존 natural-completion presentation tail이 재생 중인 음성을 유지하며
명시Stop/새epoch는 기존 정책대로 취소한다. Client/Server를 종료·Reload하지 않았고 현재
메모리까지 갱신한 것으로 기록하지 않는다. 실제 Client 청취는 사용자 확인이 남아 있다.
근거·백업·게시 로그는out/KoukuSaydonAppearVoice20260926의install-receipt,
delivery-receipt, installed-semantic-audit, final-verification 및publish-*.log에 있다.

## G30. 쇼타임 폭탄 판정과 고정 사각 예고

네 회전 추적 Duration에 본체/폭발/부채꼴의 stable occurrence 참조와 원본 반경·반각을
연결하는 선택적 계약을 구현했다. 기존 회전 Logic은 노란 부채꼴을 만들지 않으므로 Effect
행은 유지한다. Server가 본체/심지의 원래 생성 시각 및 추적 종료 판정을 소유한다. 종료의
최종 방향에서 폭탄이 타원 부채꼴 안이면 제거, 밖이면 원본 파란 폭발과 참가자 전멸이다.
전멸은 shield/일반 무적을 관통하고 비참가자는 제외한다. 폭발 tail은 자연 수명을 유지한다.

사각형 10쌍 중 고정 BOSS 9쌍은 예고 생성의 보스 basis를 함께 사용한다. 기존 MAP 예고
한 쌍은 그 절대 위치에 폭발을 맞춘다. 이펙트 자체의 원본 180도 yaw 차이와 사용자가 해제한
그룹, 각각의 시각·수명·scale를 보존한다. Source/runtime에 두 번째 Effect renderer나 새
Shared packet을 추가하지 않았다. 일반 Preview는 정적 저작 타임라인이며 판정은 Server Play다.

Python focused 4 tests와 canonical PowerShell 37 fixtures, Client 3개 TU 컴파일과 native
save/reopen/last-good/Parent remap 38 checks, 격리 Server 전체 빌드와 실제 판정 25 checks가
PASS다. 독립 Python→PS 18필드→실제 Catalog/Brain admission도 6 checks PASS다. 기존
projector 두 테스트의 6 fail/1 error는 실제 pre-G30 백업에서도 동일하게 재현했다. MAP
COLLIDER/SOUND의 옛 거부 기대 및 NaN digest 예외로 이번 범위에서 수정하지 않았다.

중간 설치2385는 실행 중 구 Client가 새 필드를 읽지 못하는 저장 경로와 충돌했다. 요청한
추가 레이저 편집을 보존하기 위해 자기 설치 hash가 그대로임을 확인한 뒤 원래 사용자2384를
원자 복원하고 G30 후보를 별도 보존했다. 사용자는 이후 레이저를 새2385로 저장하고 종료를
확인했다. 같은 revision의 서로 다른 중간 복사본은 hash로 구분하며 최종 게시 검증에서는
숫자 revision만 비교하지 않는다. 기존 atomic Save 오류 자체는 Win32 원인 코드가 없어
확정하지 않았고, 사용자의 중단 요청대로 별도 Save 구현 수정은 하지 않았다.

최신 Debug Product Engine/Shared/Server/Client 빌드는 PASS(80.681초)다. 실제 결과는
out/BuildPipeline/runs/20260925T184307022Z-debug-product.json에 있다. Client를 자동 실행하거나
Reload하지 않았다. 최종 데이터 병합·게시와 사용자 화면 확인 상태는 아래 최종 적용 항목에
기록한다. G30 검증 근거는out/KoukuShowtimeAudit20260926, KoukuShowtimeClient20260926,
KoukuShowtimeBomb20260926에 있다.

## G31. 빙고 Complete Play와 저장된 레이저

F1의 Gate=BINGO / Complete Play - Bingo Loop는 typed START의 BINGO 분기로 Encore를
건너뛰고 board-start(P129)에 진입한다. 일반 Gate3 clear는 기존 Encore P10을 거쳐 같은
빙고 흐름으로 간다. 기존 정식 fixture를 실제 Server 경로로 좁혀 4인 준비·진입·3초 대기,
35개 최초 항목과 마지막14개 반복, 30초 첫 폭탄/후속20초, 3·6번째 폭탄 특수 패턴 두 번,
특수 종료 후 원래 항목 재시작 및 보드/epoch 유지까지80 checks가PASS다. 첫 실행은 기존
게시 bootstrap2376 기준이므로 최종 레이저 게시 이후 재검증과 구분한다.

사용자가 저장한 P123 레이저의 두 번째 Stage는3509→2500ms이며 source FX 내부 element와
크기·회전도 바뀌었다. 이전 tilted FX의 collider 측정값을 그대로 재사용하지 않고 최신
저장 asset의 native 모델·Effect 평가로 양눈 형상을 다시 측정한다. 판정은 기존 두 BOX 및
연결 Logic 경로를 사용하고 바람 방구 Result134의10% 대미지·5.1m·2161ms ballistic 반응을
재사용한다. 실제 설치·측정값과 검증은 최종 적용 항목에 기록한다.

## G30~G31 최종 적용과 검증

사용자가 저장·종료한 Action2385(SHA30d58f7b…)를 다시 읽고 G30/G31 요청 필드만2386으로
병합했다. writer lock, 교체 전 hash 재확인, durable stage와 Windows 원자 교체/백업을
사용했다. 독립1934 checks에서 P35 폭탄4회·사각형10쌍, P62/P123의 두 collider/owner,
GATE3 일반 HP 반복6구간의 P62 entry 외 모든 기존 값과 순서를 보존했다. 바람 방구 공유
정의는 바꾸지 않고 기존 ENTER_AREA KNOCKBACK Trigger507과 Result134를 재사용한다.

최신 Effect 원문SHA28d5fbe4630ccdf2e066ffff22c3ca5a7aa780150982ed84b47fcd06344939b0은
그대로다. V1 source-direct catalog가 이 문서를 제품에서도 소비한다. 최신 제품 객체로
링크한 headless CModel/particle sampler에서168개 실제 본과573개 입자 sample을 얻었고,
공격창2050~3050ms의 primary core27 및 남은 beam29에서449 sample/1796 quad 정점을
검사했다. 왼눈 BOX center(18.662520,0,0.231638), yaw89.503093°, half(1.160221,.5,18.152327),
오른눈 center(18.619742,0,-.843573), yaw93.325514°, half(1.801348,.5,18.174946)로 저장했다.
이전24m beam29만의 값 대신 사용자가 저장한 약36m core 형상을 포함한다. 이는 CPU 렌더
quad의 XZ 범위이며 texture alpha의 정확한 윤곽이나 GPU 화면 확인을 뜻하지 않는다.

P123 두 번째 Stage2500ms, FX3451ms, P62 FX3454ms와 각 패턴의 음성·Animation·Effect TRS는
그대로다. 기존 중복 콜라이더를 추가하지 않고 두 눈의 기존 BOX를 각각 교정했다. 최신
native 문서 Parse/Validate, Python 전체 문서와 scoped projection이PASS다. 실제 canonical
PS→Catalog/Brain→Logic window→HitReaction 경로46 checks에서 양눈 안/밖, 보스 yaw0/90,
중복1회,10% 피해와5.1m/2161ms 강제 포물선 및 무적 차단이PASS다.

공식 전체 Composition publish와Gameplay balance Publish가exit0/PASS다. 정식 게시본의
P35/P62/P123 encounter와presentation은 검증 후보와 정확히 같고 sourceRevision2386,
bootstrap PATTERNTRACKBOMB18필드4행 및 GATE3레이저6entry를 확인했다. Gameplay bootstrap은
110237행/32575150byte이며 마지막 Source와Effect hash도 동일하다. 최신 제품 Server.exe를
실제 설치 DataFiles로 실행한 --kouku-showtime-bomb-contract-test도25 PASS/0 FAIL이다.

Client.exe SHA0de82b2bd416c2d58eec2a610b43fbb0cdf077e71522486c3ce3329579f48292,
Server.exe SHA6637ba9a9f84e5f2b3088c4c515789ffecab53b6572d1067b61731303683d623다.
JSON parse 및 변경 대상git diff --check가PASS다. 설치/게시/검증 영수증은
out/KoukuShowtimeAudit20260926의install-receipt, final2386-independent-preservation,
final-verification, product-showtime-receipt 및publish-final 로그와
out/KoukuBingoLaser20260926의verification-receipt, server/focused-receipt에 있다.
Client/UI를 실행하거나 Reload하지 않았으며 화면에서의 최종 크기·연출과 조작 확인은 사용자 몫이다.

최종2386의 실제 게시 bootstrap으로 빙고 focused 실행도80 PASS/0 FAIL/exit0(33.41초)다.
P123 source/Encounter/PATTERNSTAGE가 모두1000+2500=3500ms로 일치하며 최초 검사 때의
중간 게시 불일치는 해소됐다. 4인 진입·3초 idle·35개 최초 패턴과14개 반복 tail·블랙홀 특수
두 회·30/20초 폭탄 시계·원래 entry 재시작·보드 소유권을 재확인했다. 실행 전후 source,
렌더링, Effect 및 두 EXE hash가 동일하다. 증거는out/KoukuBingoCompletePlay20260926의
final-product-preflight, final-product-static-receipt, final-product-receipt와final-product-test.log다.

## 2026-09-26 Resources 전달

사용자 지정 Desktop/GBResources에 Resources 상대 경로를 유지하여2개 파일을 복사했다.
새 경로는 G29 세이튼 등장 음성1개이며, 오늘 갱신한 G26 최종 클리어 손 뻗기 clip 내장
MN_RPCT_05.wmodel도 함께 전달했다. WModel은 신규 파일이 아니라 기존 파일 교체다.
두 대상은 설치 영수증·기존 모델 백업과 독립 대조했고, 복사본SHA256이 제품과 일치한다.
총286651604byte다. G30/G31 및 레이저 source-direct JSON은 Code/Data 변경이므로 별도
Resources 파일을 만들지 않았다. 전달 증거는out/GBResourcesDelivery20260926/delivery-receipt.json,
독립 감사는out/GBResourcesAudit20260926/resource-delta-receipt.json이다.

## G32. 쇼타임 Product 읽기 실패가 전체 관문 FPS를 떨어뜨린 회귀

### 원인과 적용

사용자 PID32052의 5-frame capture에서 전체 CPU frame 평균1284.455ms 중
Presentation.Prepare가98.661%를 차지했다. 각 frame의 준비1219.042~1416.565ms,
CameraShots.Load207.666~224.797ms, Client.Render CPU7.231~16.001ms였다.
같은 PID의 EffectFailure 로그는 `Invalid presentation string: anchorPresentationOccurrenceId`
오류를1/2/4/8/16회로 기록한다. 단일 Client에서도 발생한 실패 재시도이며
4-client GPU 포화라고 단정할 근거로 사용하지 않는다.

G30 쇼타임 조건부 본체/폭발 템플릿6개는 선택 anchor를 빈 문자열로 직렬화했다.
기존 native reader가 이를 비어 있지 않은 필수 문자열로 취급하여 공유115-pattern
Product 전체를 거절했다. 공유 Product는 관문별로 분리되지 않아1관문부터 영향을
받았다. 9/24 기존 run 준비 코드는 성공 후에만 epoch를 기록하여 실패하면
카메라와 Product를 다음 frame에서 다시 읽었다. G26 최종 클리어 animation의
재생 비용이 원인이라는 증거는 없다.

`KoukuSaydonPresentationPlayer.cpp`는 선택 anchor의 생략/빈 문자열을 허용하고
타입·길이·NUL 검사는 유지한다. targeted visual의 비어 있지 않은 shared birth
참조는 projector와 동일하게 거부한다. 정상 pattern의 고정 Effect 참조는 유지한다.
동일 run/source/draft의 성공·실패를 저장하고 admission이 아직 도착하지 않았으면
파일 I/O 없이 기다린다. 실패 identity는 매 frame 재시도하지 않고 새 identity,
명시적 successful Reload 또는 Reset으로 회복한다. Product 성공 뒤에만 카메라를
한 번 읽는다. 새 profiler scope는 Product reload와 run preparation을 구분한다.

### 실제 검증

- `out/KoukuPresentationRetry20260926/diagnosis.json`: 사용자 capture/실행 로그와
  정확한2386 게시본 SHA,6empty anchor 확인.
- Debug Product 최종 Build PASS,23.692초.
  `out/BuildPipeline/runs/20260925T194919211Z-debug-product.json`.
  첫 컴파일은 추가 profiler header 누락으로 실패했고 include 교정 후 통과했다.
- 최신 Product OBJ로 실제 Reload_Product/asset admission/run preparation을 호출한
  `retry-run.log`:29PASS/0FAIL. 독점 잠금한 scratch Product를 같은 실패 identity로
 1000회 호출해 원래 parser 오류 및 기존 Product 보존, 총0.1946ms. 재읽기하면
  파일 읽기 오류로 바뀌므로 같은 실패를 재파싱하지 않는 실행 경계를 확인했다.
  늦은 admission, 새 epoch/source/draft, explicit Reload 회복과 Reset도 통과했다.
- `git diff --check` PASS. 제품 JSON/렌더링 옵션/게시 데이터는 변경하지 않았다.
  Product SHA는 수정 전후 동일하고 domain publish를 실행하지 않았다.
- 설치 Debug Client SHA256:
  `cfeb8ce481dbae9fcb3798cabdb37cbe97de6c8fd424987589cd4c1093486444`.

### 남은 확인

실제 아레나 FPS 회복과 쇼타임 화면은 사용자가 새 Debug 실행 파일로 확인한다.
Client GUI는 자동 실행하지 않았다. Release 소스는 공통 수정되었으나 이번에는
Debug만 빌드했다. Debug의 Complete Play cold resource 준비 시간은 이번
매-frame 실패 재시도와 별도이며, 그 로딩 스케줄 변경은 이번 수정에 포함하지 않았다.

### G32 최종 게시본 native 전후 대조

`probe_before.log`/`probe_after.log`는 같은2386 게시본(4,277,178bytes/115patterns)을
수정 전/후 실제 Product OBJ에 각각 넣은8case native 회귀다. 각각8PASS/0FAIL.
수정 전 exact 게시본은 로그와 같은 빈 anchor 오류로 실패하고 수정 후는 전체
로드에 성공했다.6empty anchor를 생략한 문서는 양쪽 모두 통과하여 기존37개
정상 참조의 보존도 확인했다. targeted missing/self 참조는 수정 전 잘못 통과하던
경계를 수정 후 거부한다. 비문자열(number/null), 내장NUL 및 일반 pattern의
dangling 참조는 계속 거부한다. probe는 입력 파일·제품 파일을 변경하지 않았다.

## G33. 4인 준비 중 활성 WORLD의 정상 대기를 실패로 보내던 문제

### 확인된 원인과 소스 수정

네 Client의 최신 로그에서2386 Product115개 패턴 로드는 성공했다. 첫 runEpoch1의
Player4만 resources.prepare 단계에서 활성 WORLD 때문에 실패했고, Server는 참가자
실패를 받아 전원 준비를 취소했다. 정확한 활성 WORLD ID는 당시 로그에 없다.
runEpoch2는 별도 WORLD 실패로 단정하지 않는다. 로그에는 owner의 명시 Stop이 있다.

Level_KakulSaydonArena.cpp의 새 준비 요청 분기는 활성 WORLD가 있으면
`true/ready=false`를 반환한다. MainApp의 로컬 사전 준비와 Server PREPARING은
이 값을 기다림으로 처리하여 START/READY/FAILED를 아직 보내지 않는다. Level의
기존 WORLD Update는 다음 프레임에도 진행하고, 연출 종료 뒤 최신 게시 문서 reload와
리소스 준비를 이어간다. 대기 상태에는 활성 stable ID 최대4개와 초과 표시를 넣었다.
현재 재생·문서·모델 pool·기존 준비 상태는 대기 때문에 바꾸지 않는다.

유한 입장 연출의 준비 경합을 수정한 것이며 무한 반복·일시정지된 WORLD를 자동 Stop하지
않는다. 실제 데이터 오류와 준비 중 revision 변경은 계속 실패로 처리한다. 저작/게시
JSON, 리소스, 렌더링 옵션은 이번 수정에서 변경하지 않았다.

### 검증·빌드 상태

소스와 호출자 검토에서 두 MainApp 경로의 pending 분기, 기존 WORLD Update 선행,
20분 준비 제한과 준비 완료 후에만 시작하는5초 reply 안내를 확인했다.
실제 Arena 생성자·준비 함수의 수정 전후 native 회귀8개씩16PASS/0FAIL이다.
활성 상태에서 false가 pending true로 바뀌고 ready=false가 유지되며, 문서·active
clock·held·모델 cache·pool·이전 preparation은 그대로였다. 같은 대기 요청500회는
4.494ms였다. 활성 항목을 제거한 다음 호출이 기존 reload 경로로 진행하고 실제
target 오류를 숨기지 않는 것도 확인했다. GPU/GUI 없는 두 Arena의 호출 검증이며
실제4인 네트워크·빙고 최종 클리어 실행을 대신하지 않는다.
`out/KoukuCompletePlayWorldWait20260926/receipt.json`, `run_before.log`, `run_after.log`에
증거를 보존했다. 변경 TU 격리 컴파일과 `git diff --check`도 통과했다.

Release 설치 빌드는 최종 PASS다. 최초 Release Product 실행은
`ProductOutputGuard.psm1`이 실행 중 Debug Server PID24184를 감지하여 컴파일 전에
중단했다. `out/BuildPipeline/runs/20260925T200725736Z-release-product.json`은 이
미완료 기록이며 성공 증거가 아니다. 사용자 프로세스는 자동 종료하지 않았다.

Debug Product의 후속 확인은 PASS다.
`out/BuildPipeline/runs/20260925T201523816Z-debug-product.json`은 현재 출력이 이미
최신이어서 OBJ/CSO/binary 쓰기0이었다. 설치 Client의 수정 시각은05:09:50이며,
EXE에 새 WORLD pending 문구가 있고 구 실패 문구는 없으며 G32 준비 scope도 있었다.
따라서 이 실행 파일이 G33 이전이라고 한 중간 안내는 부정확했고 사용자에게 정정했다.
설치 SHA256은 `4b75161ca294018f12764690e1ec16376580c5e777d126b2e78ffcf07baad063`이며
`debug-installed-verification.json`에 기록했다.

Release Product의 최종 영수증은
`out/BuildPipeline/runs/20260925T204806447Z-release-product.json`이다.
총2132.515초이며 이전 Release 이후의 변경분을 함께 반영하여 Client 단계에서
OBJ279개·CSO48개가 갱신됐다. 이번 G33은 shader를 수정하지 않았고 새 publish도
하지 않았다. 기존 shader/encoding 및 DirectXTK PDB 경고가 있었지만 빌드 오류는 없었다.
Client·Server 링크/배포와 필수 runtime 검사 모두 통과했고 missing/invalid 항목은 없다.

Release Client는05:48:05 생성본이고 SHA256은
`6eddfb4571cd374a6ff61d4efea6dff9f6a1b77c56fcbe0118080823620b7fcd`이다.
Debug/Release 설치 EXE 모두 새 WORLD pending 문구와 G32 run-preparation scope가
있으며 구 WORLD 실패 문구는 없다. 최종 소스 hash도 native 회귀 때와 같았다.
`out/KoukuCompletePlayWorldWait20260926/final-build-verification.json`에 두 구성의
Client/Server hash와 설치 확인을 기록했다. Release GUI는 실행하지 않았다.

### 사용자4클라 실제 준비 완료와1관문 연출 시작

05:30:49 KST의 최신 로그에서 PID39576(generation2),43532,10608,14044가 모두
runEpoch1 ready=true와 phase2 CINEMATIC을 기록했다. Player1은05:25:07.640에
먼저 준비됐고 Player4는05:29:33.061, Player2는05:29:36.905, Player3은
05:29:50.392에 완료했다. 그 사이 나머지 참가자의 V1 준비 완료 로그가 계속 추가돼
실제 cold resource 준비였음을 확인했다. 이미 준비된 소유자의 일반 대기 문구만으로
전체가 멈췄다고 판단하지 않는다.

사용자도 “실행됐어”라고1관문 실제 재생을 확인했다. 정확 세대의 네트워크 이벤트와
`Client/Default/EffectFailure.user.log` 진행 증거는 `runtime-progress.json`에 기록했다.
1관문 준비와 연출 시작 확인이며 빙고 진행·최종 클리어·4클라 최종 FPS 확인은 남아 있다.

## G34. 빙고 종료 손 뻗기 원작 선택지 추가 — 2026-09-26

사용자가 확인한 장면은 P9 빙고 종료 후 쿠크·세이튼 엔딩이다. 같은3.5초 창을
기준으로 원작 선택지를 추가했고 continuous는 유지했다. 후보만 생성했던
pre-continuous WORLD 복구안은 설치하지 않았다. 현재 WORLD2277과 Sequence177의
저장 내용·카메라·사운드·크기·위치는 이번 작업으로 바뀌지 않았다.

Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel에
kouku.bingo.ending.handreach.original을 설치했다. 원래 baked
kouku.bingo.ending.saydon1의12.900~16.400초(tick387~492)를 복사한
3.5초/30Hz/168채널/106프레임 클립이다. P/Q/S 값 bytes는 그대로이고 timestamp만
0~105로 옮겼다. 원본 slot 혼합·표정/팔 SkelControl 및 원래 source 되감기를
포함한다. raw rpct00_evt2_atpain01 전체12초와 구분한다.

기존 build_bingo_ending_actors.py의 --original-hand-reach는 후보만 생성한다.
이번 요청의 설치는 최신 hash 재확인·백업·원자 교체·설치 후 parser 검사로 수행했다.
기존259개 animation과 geometry/rest/material 등 모든 section payload가 동일하며
설치 catalog는260개다. 동일 내용 재추가0개와 같은 이름/다른 내용 거절을 확인했다.
53,424개 key와211개 frame/중간 시각 pose를 비교했고 최대 component 오차는
2.9686453517641098e-8이다. 새 C++ 변경이나 제품 빌드는 없다.

설치 전 SHA256은714c247f7cb6d6d9016ddf3a2810774f057db9297db0e385f42605e064100219,
설치 후는afbb1fb67fab098c4a9c43554d7b36a79967ac1d13a32ffd19ef0e9f7f93b3bf다.
out/KoukuEndingRecovery20260926/original-handreach의 후보/설치 영수증과
714c247f...backup.wmodel에 근거와 백업을 보존했다. 원작 full clip payload도
continuous 설치 전 백업과 동일하다.

### 사운드 시간 대조와 남은 사용자 화면 확인

현재 손 동작은25.133~28.633초로 원작 창보다12.233초 늦다. 뒤 animation 행들도
동일하게 밀렸고 saydon1 template은61.316초지만 P9 WORLD parent는49.083초다.
두 SOUND의 start/sourceStart는 모두0이고 다섯 자막은 원본 시각을1ms 이내로 유지한다.
원래14.467초의 “일어나, 발연기 그만하라고.”와 겹쳤던 손 동작이 현재는27.267초의
세 번째 대사 구간과 겹친다. 새 native clip을 선택해도 박스 시작 시각은 바뀌지 않는다.
전체 음원을 이동하면 다른 배우·카메라·자막의 관계도 바뀌므로 이를 자동 조정하지 않았다.

원작 SCENE01B의 voice/FX event trigger는100ms, 현재 SOUND trigger는0ms다.
이는 메타데이터 차이이며 렌더된 WAV 내부 delay와 실제 청취 없이100ms의 가청
동기 오차라고 단정하지 않는다. 두 WAV의 실제 길이와 runtime sourceStart 소비까지
대조한 증거는out/KoukuEndingRecovery20260926/p9-sound-alignment-receipt.json이다.
현재 시간 배치는 원작과 일치하지 않으므로 최종 동기화 완료로 기록하지 않는다.

World Object의 Saydon1에서 Refresh Native Clips는 디스크 catalog를 읽으므로
새 이름을 선택할 수 있다. 실행 중 이미 준비된 CModel은 파일 추가만으로 바뀌지 않는다.
사용자가 저장·적용한 뒤 idle WORLD의 다음-play reload는 model cache를 비우지만
활성 연출은 기존 준비본을 유지한다. 사용자 Client 조작·Reload·재생·청취는 하지 않았다.
원래 배치 기준은Start12900/End16400, Source In0/Out3500, speed1이다.
사운드 앞뒤 자르기는 기존 SOUND Source In/Source Out과 양끝 드래그가 지원한다.

Python 구문 검사와 변경 범위 git diff --check를 통과했다. 설치 전후 두 저작 JSON의
SHA256이 같음을 확인했다. 설치, 원작 pose 수치 동등성, 사용자 실제 화면/청취 판정은
서로 다른 상태이며 최종 화면과 타이밍 편집은 남아 있다.

## G35. 원래 WORLD 복원과 손 뻗기 구간 한 번 Duplicate — 2026-09-26

사용자가 “원래 world 애니메이션에서 손뻗는 그 클립만 duplicate”를 명시하고
즉시 반영하도록 승인했다. G34의 native 선택지 추가와 교정 클립 재배치는 최종
요청이 아니었다. 별도 만들어 둔 world-hand-restore 후보는 설치하지 않았다.

P9 sequence.kouku.bingo.ending.saydon1 하나에서 원래11개 animationTracks와
76개 transform key를 복원했다. 그 안의 원작 baked clip12967~16333ms를 기존
CWorldSequenceDocument::Duplicate_Track과 같은 방식으로 한 번 복제했다.
복제본은16333~19699ms, sourceStart12967, clipName kouku.bingo.ending.saydon1,
speed1/loopfalse다. 표시 이름에(Duplicate)만 덧붙였다. 뒤쪽 같은 배우 animation은
3366ms 밀리고 Motion duration은52449ms, 마지막 held transform key1개가 붙는다.
최종12개 animationTracks와77개 transform keys다. 새 native/continuous clip을
선택한 결과가 아니라 원래 WORLD 구간을 한 번 Duplicate한 결과다.

WORLD2277→2278을 원본 및 실행 파일에 반영했다. 전체 JSON에서 이 템플릿의
durationMs/animationTracks/tracks와 revision 외 값은 동일하다. 원래76개 transform
key의 위치·회전·배율·가시성은 그대로다. 다른 네 엔딩 배우와 모든 다른 WORLD,
카메라·사운드·다른 패턴은 수정하지 않았다. Sequence Composition의 WORLD 부모
49083ms도 유지했으므로 원래 음향과 복제 이후 동작의 동기나 전체 tail 표시까지
완료했다고 주장하지 않는다. 이번 범위는 사용자가 최종 지정한 한 번의 Duplicate다.

공식 WorldSequences 후보 Validate와 단일 파일 Publish가 exit0이다. source/runtime
SHA256 일치, 전체 문서의 요청 밖 값 동등성과9개 보호 입력의 hash 불변을 확인했다.
보호 입력은 Action/Sequence Composition, patternbindings, encounter, 렌더링 source/
runtime, Server Gameplay/world/spawngroups bootstrap이다. Composition/Gameplay
publish 및 Client/UI 조작·강제 Reload는 실행하지 않았다. 사용자가 열어 둔 편집기는
Reload Source로 새 WORLD를 읽어야 한다. 제품 C++ 변경·새 빌드는 필요 없다.

검증과 source/runtime 백업은out/KoukuEndingRecovery20260926/original-world-duplicate/
의receipt.json,install-receipt.json,validate.log,publish.log와world.*.backup.json에 있다.
변경 JSON parse와git diff --check를 확인했다. 실제 Client 재생 화면은 사용자 확인이다.

### G35 반영 후 Debug Product 빌드

사용자 요청으로 정식 Debug Product Build/Deploy를 실행하여 PASS했다. 영수증은
out/BuildPipeline/runs/20260925T223840970Z-debug-product.json이며 SkipBuild=false,
총3012ms다. Engine/Shared/Server/Client 모두 최신 출력으로 OBJ/PCH/CSO/binary
쓰기0이었다. 필수 runtime missing/invalid는 모두0이다. 빌드 후에도 설치 WORLD
양쪽hash와 보호한9개 패턴/게시 입력hash가 같았다. GUI/청취 검증은 사용자 실행으로
진행하며 final-verification.json에 EXE hash와 빌드 연결을 기록했다.

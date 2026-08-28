# 발탄 Trash 포획·카운터·전멸 구현 계획

작성일: 2026-08-27 / 구현 승인 반영: 2026-08-28

초기 구현 기준: 병합된 main `cd120501`에서 분기 후 PR #251 `a6871a2f` 통합, 브랜치 `codex/valtan-arena-navigation-next`
초기 구현 폴더: `C:/w/valtan-arena-next`
최종 적용 기준: PR #251 `a6871a2f` 기반 `codex/valtan-arena-next-desktop`, `C:/Users/user/Desktop/LostArk/Framework.sln`

## G00. 이번 승인 범위와 기존 초안의 변경

2026-08-28 사용자가 붙여넣은 Next Pattern/Trash 계약을 구현 기준으로 삼는다.
이 문서는 이전 초안의 `ALL_GRABBED_RELEASE`와 보조 패턴·기본 Flow 삭제 제안을 대체한다.
실제 빌드·실행 증거와 남은 수동 검증은 대응 RESULT에 기록한다.

- 전원 포획은 안전 해제가 아니라 1500ms impact의 Server 전멸 처리다.
- 부분 포획은 잡힌 대상 damage와 해제를 commit하고 animation tail 뒤 재조준한다.
- 보조 패턴 `VALTAN_TRASH_CATCH_SUCCESS/FAIL/IF`와 저장된 Flow를 보존한다. FBF 퇴출 통합으로 해당
  `slot.000017`만 제거한 통합 당시 28슬롯/ordinal30에서, 이후 사용자가 저장한 현재 22슬롯/ordinal31을
  그대로 유지한다. 사용자 추가·삭제·재정렬을 초기 배열로 되돌리지 않는다.
- Next Pattern은 별도 1칸 예약이며 기존 Ordered Slots에 삽입하지 않는다.
- 기존 left-hand bone presentation과 hit 위치 기준 boss-local fallback offset은 유지한다.
  이전 초안의 canonical grip pose bake와 보조 Effect/sound cue 이관은 이번 첨부 계약의 구현 범위에 넣지 않는다.

관련 계획은 [무리셋 Next Pattern 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md)이다.

## G01. 현재 소비자와 데이터 정본

`Data/Valtan/Valtan.gameplay.json`이 stage graph와 typed event를,
`Valtan.presentation.json`이 animation occurrence를 소유한다.
`author_valtan_phase_two_mechanics.py --pattern-id VALTAN_TRASH`로 해당 패턴만 저작·검증하고
`Project-ValtanPatternMaster.ps1 -Mode PublishV2`로 기존 Product projection을 생성한다.
생성된 Encounter와 pattern bindings는 직접 편집하지 않는다.

Server는 `CValtanBrain -> CGameRoom`으로 포획 request와 stage action을 전달한다.
Shared XZ overlap과 Server damage가 실제 LANDED인 hit만 포획 request를 만든다.
분기는 request 수가 아니라 `Capture_PlayerAttachment`가 commit한 attachment를 읽는다.
Client는 snapshot의 `GRABBED/owner/slot`과 기존 Character presentation을 사용한다.

## G02. 단일 Trash 내부 graph

현재 split gameplay와 presentation, 생성 Encounter의 Trash는 모두 14개 stage다.
기존 `STEP_01..08` 뒤에 포획·counter·impact·miss·GROGGY 분기 6개를 연결하며, 8단계 직렬 재생으로 종료하지 않는다.

| Stage | 시간 | 역할·다음 경계 |
|---|---:|---|
| `STEP_01..05` | 순서대로 1200 / 1000 / 1200 / 2333 / 667ms | 현재 project-authored 도입 유지 |
| `STEP_06` | 4100ms | `13_02-1` setup, counter 없음 |
| `STEP_07` | 1000ms | `13_03` 재조준 |
| `STEP_08` | 667ms | `13_04` root motion + BOX CAPTURE; ANY이면 counter, NONE이면 miss |
| `CATCH_COUNTER` | 200ms | `13_05-1` source 0..200; COUNTER_HIT → GROGGY, TIMEOUT → pre-impact |
| `CATCH_PRE_IMPACT` | 1300ms | 같은 clip source 200..1500; deadline에서 ALL 재판정 |
| `CATCH_SLAM` | 1500ms | source 1500..3000; ENTER에서 부분 포획 damage+해제, tail 뒤 STEP_07 |
| `EXECUTE_TAIL` | 1500ms | source 1500..3000; ENTER에서 전원 처형, tail 뒤 terminal |
| `RUSH_MISS` | 1000ms | `13_05-2`, tail 뒤 STEP_07 |
| `GROGGY` | 4433ms | 실제 420631 start 1833 + loop slice 600 + end 2000; ENTER에서 해제 |

`ALL_GRABBED_RELEASE` stage는 만들지 않는다. `13_05-1`을 0..200 / 200..1500 / 1500..3000으로
잘라 impact 경계를 정확히 저작한다. stage motion은 기존 root-motion projection을 사용한다.
새 이동 경로, Client damage, 패턴 이름에 따른 targetless 예외는 추가하지 않는다.

Counter는 기존 counter-power와 boss-local proxy admission을 그대로 사용한다.
`COUNTER_HIT`을 deadline보다 먼저 소비하며 유효 hit가 들어오면 counterable flag를 즉시 닫는다.
잡힌 플레이어 자신은 skill/counter를 제출할 수 없다.

## G03. 포획 분류와 수명

`CValtanBrain::Classify_GrabbedPlayers` 한 함수가 NONE/PARTIAL/ALL을 만든다.
살아 있는 참가자는 `HP > 0`, action이 DEAD/FALLING이 아닌 player다.
`isCombatReady=false`라는 이유로 분모에서 빼지 않는다. 빈 생존 집합은 ALL이 아니다.

유효 포획은 다음 조건을 모두 만족한다.

- action `GRABBED`, owner는 현재 boss의 stable NetEntityId
- slot은 `BOSS_LEFT_HAND`
- attachment의 pattern sequence는 현재 boss occurrence와 동일
- fallback offset과 yaw는 finite

다른 boss/slot, stale sequence, invalid offset은 ALL로 인정하지 않는다.
ServerPlayer의 attachment sequence는 서버 내부 freshness 상태이며 wire 저장 ID를 새로 만들지 않는다.
Capture request는 소유권과 생존 여부를 재검사하고 성공 시 action/projectile/combo/pending move/skill을 취소한다.

Capture pulse의 마지막 due tick은 rush deadline보다 앞서야 한다.
현재 0..600ms의 7개 pulse와 667ms deadline은 기존 30Hz 실행 순서를 유지한다.

## G04. stage action의 원자적 commit

추가 typed action은 `DAMAGE_GRABBED_PLAYERS`와 `EXECUTE_GRABBED_PLAYERS`다.
Gameplay bootstrap은 v25로 올리고 `PATTERNAUTHORINGMANAGED` row에 실제 split 소유 집합을 함께 publish한다.
현재 관리 집합은 29개이며, 공용 Tool 27개 inventory나 사용자가 편집하는 저장 Flow 목록으로 대체하지 않는다.
Server/Client parser와 publisher 모두 unknown kind, 잘못된 필드 조합, 잘못된 reference를 거부한다.

### 부분 포획

`DAMAGE_GRABBED_PLAYERS`는 `damage.valtan.charge-grab-roar`를 참조한다.
기존 500% damage profile을 사용하며 값을 새로 복제하지 않는다.
유효하게 잡힌 대상 전체와 damage-event 여유를 검사하고 player 사본에 damage+attachment 해제를 준비한다.
성공하면 stage/boss/player 결과와 damage event를 함께 commit한다.

### 전원 포획

`EXECUTE_GRABBED_PLAYERS`는 impact 진입 시 ALL을 다시 검증한다.
일반 world hit 대상 필터를 우회하는 별도 typed transaction으로 모든 대상 HP를 0으로 만들고 DEAD 및
attachment 정리를 사본에 준비한다. 모든 검증이 성공한 뒤 한 번에 commit하고 damage/death snapshot으로 복제한다.
두 번째 대상 또는 event capacity 검증이 실패했을 때 첫 번째 대상만 죽이거나 attachment만 해제하지 않는다.

### 카운터와 실패

기존 `RELEASE_GRABBED_PLAYERS`도 같은 player-copy staging을 사용한다.
HOLD와 OPPOSITE_KNOCKBACK을 서로 다른 임시 commit 경로로 나누지 않는다.
전이 실패 시 boss를 rollback하고 room을 not-ready로 두며 부분 전이 snapshot을 내보내지 않는다.
잡힌 대상 transaction 실패는 기존 attachment를 보존한다.

전멸 commit 후에는 `iGrabExecutionCommittedPatternSequence`와
`iGrabExecutionCommittedStageIndex`가 현재 occurrence와 stage에 일치할 때 targetless animation tail을 진행한다.
실제 `FinishPattern`의 성공/실패 receipt가 terminal 원인이며 빈 patternId만 보고 성공으로 처리하지 않는다.

## G05. Next와 Client 입력

Trash가 전원 처형 뒤 animation tail을 끝내면 A COMPLETED를 발행한다.
예약된 B는 생존한 실행 대상이 없으면 WAITING_FOR_PLAYER로 유지한다.
자동 부활·중앙 이동·벽/바닥/prop 복구 없이 사용자 revive 뒤 다음 fixed tick에서 B가 시작한다.
A와 B는 같은 pinned gameplay revision을 사용한다.

`CPlayerController`는 GRABBED snapshot 중 ground preview, hold, BA 상태를 취소하고 command sink 전에 반환한다.
ReleaseSkill을 포함한 새 입력 packet은 0건이어야 한다.
키/LMB/RMB를 계속 누른 상태로 해제되어도 자동 재실행하지 않고 실제 물리 release 뒤 다시 입력할 수 있다.
공개 `Request_MoveToPoint` 경로에도 GRABBED admission을 적용한다.

## G06. 변경 파일과 등록

| 영역 | 실제 변경 위치 |
|---|---|
| graph/presentation | `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json` |
| projection | `Data/Encounters/Valtan/ValtanEncounter.json`, authored pattern bindings, root-motion JSON |
| 저작·검증 | `author_valtan_phase_two_mechanics.py`, `valtan_tuning_pipeline.py`, `Publish-GameplayBalance.ps1` |
| Server | `ValtanBrain`, `GameplayCatalog`, `GameRoom`, `ServerPlayer`, `ServerWorldEntity` |
| Client | `ValtanPatternTree`와 `EncounterPatternReference` strict parser, `PlayerController` input gate |
| 회귀 | 기존 master V2/tree tests, `ServerGameplayContractTests.cpp`, PlayerController header harness, Action harness의 `ClientPartyRegression.cpp` |

새 제품 C++ 파일은 없으므로 기존 .vcxproj/.filters에 중복 등록하지 않는다.
Next service 실행형 프로젝트 추가와 Shared protocol 42는 연계 Next 계획에서 다룬다.

## G07. 검증 계약

최종 빌드·실행 대상은 `C:/Users/user/Desktop/LostArk/Framework.sln`이다.
단계별 자동 검증은 재개했으며, 개별 성공과 최종 Desktop 전체 회귀 완료를 구분한다.
최종 Desktop Debug/Release 전체 자동 검증과 사용자 시각 판정은 미완료이며, 선행 worktree의 검증 이력과 구분한다.

자동 검증은 다음을 포함한다.

- NONE/PARTIAL/ALL/COUNTER, empty roster, non-combat-ready 생존자, wrong boss/slot/stale sequence/NaN
- 실제 13_04 hit → capture → 1500ms impact → 전멸 → targetless tail
- damage-event 용량 부족과 두 번째 대상 오류에서 HP/attachment 부분 commit 없음
- GROGGY 진입에서 실제 420631 occurrence와 release 동시 반영
- 전원 처형 뒤 Next WAITING 유지, typed revive 뒤 B 한 번 시작
- root-motion source slice, exact graph/projection, aux ID와 저장 Flow 보존
- GRABBED packet 차단과 물리 입력 release 전 재실행 차단
- 실제 Client Encounter 로드와 ANY_PLAYER_GRABBED/DAMAGE/EXECUTE의 잘못된 필드·중복·hit 충돌 거부 및 기존 reference 보존
- publisher Validate, 기존 Python suite, Shared/Server/Client Debug/Release, 정본 regression, diff/JSON/XML 검사

Runtime root motion은 기존 30Hz stage 전환 순서를 사용한다.
source raw final displacement와 실제 fixed tick에서 소비한 이동을 같은 값이라고 가정하지 않는다.
Client 실행·UI 조작·화면 캡처는 하지 않는다. 사용자가 실제 카운터, 전멸, Next 이어짐과 시각 표현을 확인한다.

# Valtan Trash 포획·카운터·전멸 구현 결과

작성일: 2026-08-28

현재 적용 브랜치: `codex/valtan-arena-next-desktop`

현재 적용 위치: `C:/Users/user/Desktop/LostArk`

분기 기준: PR #251 main `a6871a2f`. 선행 구현은 `cd120501`에서 시작한
`C:/w/valtan-arena-next`에서 진행했으며, 다른 작업의 확정 변경과 함께 Desktop에 통합했다.

## G01. 현재 상태

최신 붙여넣기 계약의 Trash 코드·데이터와 다른 작업의 확정 변경을 실제 Desktop에 반영했다.
Desktop 통합 시점의 projection·root motion·receipt·벽 파괴·navigation·balance publisher는 모두 exit 0이다.
이는 소스와 생성 데이터의 적용 증거이며 최신 통합 바이너리의 전체 검증 완료를 뜻하지 않는다.
선행 worktree의 Debug/Release 빌드·Server 계약 PASS는 아래에서 통합 전 이력으로 구분한다.

후속 Desktop Server Debug/Release 계약은 모두 failures 0이며 Release core·Server 빌드,
Release live 회귀, Next service와 native 검증도 아래 로그 범위에서 통과했다.
Client Release 첫 빌드의 LNK2001/LNK1120은 최신 소스 재빌드로 해소했다.
최신 Client·Server D/R 빌드와 점프찍기→휠윈드 방향 수정의 실제 Server tick 회귀도 통과했다.
정본 Debug/Release 전체 자동화는 실행 중이며, 사용자 시각 판정은 별도로 남아 있다.

관련 정본은 [Trash 구현 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_VALTAN_TRASH_CAPTURE_COUNTER_SERVER_FLOW_IMPLEMENTATION_PLAN.md)과
[Next Pattern 구현 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md)이다.
벽·navigation·body collider 변경 및 Next UI 전체 결과는 이 문서의 검증 범위와 구분한다.
최종 통합 검증과 다른 작업의 반영 상태는
[통합 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_RESULT.md)를 따른다.

| 구분 | 현재 상태 |
|---|---|
| Trash graph·typed action·Server 연결 | Desktop 적용 완료, 14 stage 보존 |
| NONE/PARTIAL/ALL/COUNTER 및 Next 전멸 연계 fixture | Desktop Server D/R에서 Trash 통과. 실제 Next 전멸 연계는 Debug에서 통과하고 Release는 Next command 거부 계약을 확인 |
| RELEASE rollback 보강 fixture | Desktop Server D/R에서 counter ENTER·terminal EXIT 모두 PASS, 전체 failures 0 |
| Client GRABBED 입력 및 물리 release gate | Desktop 적용 완료. static_assert·최신 Client D/R 빌드 PASS; 사용자 입력 관측은 미실행 |
| Desktop 통합 시점 publisher chain | 모두 exit 0; 아래 통합 로그 기준 |
| Python master·native harness | FOUR 복구 후 Master V2 53 tests PASS, Action의 Trash parser 15개 회귀 D/R PASS. 후속 변경 전체 검증과는 구분 |
| 최신 Debug/Release 전체 regression | 최신 Client·Server D/R 빌드 및 방향 수정 계약 PASS. 최종 정본 전체 자동화 실행 중 |
| Client 실행·시각 판정 | 사용자 확인 대기; 에이전트 실행·조작·캡처 없음 |

## G02. 구현한 계약

### Trash 내부 분기와 animation 시간

기존 `VALTAN_TRASH` 안에 14개 stage로 닫았다. 보조 패턴
`VALTAN_TRASH_CATCH_SUCCESS/FAIL/IF`는 보존했다. 다른 작업의 확정된
`VALTAN_SEQUENCE_FRONT_BACK_FRONT` 퇴출을 통합하면서 저장 Flow에서는 해당
`slot.000017`만 제거했다. 초기 통합 당시 남은 28개 슬롯의 stable ID와 순서,
`slot.000028`, `slot.000029`, `nextSlotOrdinal=30`을 보존했다. 이후 사용자가 저장한 현재 Flow는
22슬롯/`nextSlotOrdinal=31`이며, 후속 추가·삭제·재배열을 초기 통합 목록으로 되돌리지 않는다.

- `13_04`의 실제 LANDED hit가 Server capture request를 만들고 room이 attachment를 commit한다.
- NONE은 `13_05-2` 실패/회복 뒤 `STEP_07`로 재조준한다.
- PARTIAL은 `13_05-1`의 0..200ms counter, 200..1500ms pre-impact를 거쳐
  1500ms에 잡힌 대상만 기존 `damage.valtan.charge-grab-roar` 500% profile로 damage+해제한다.
  1500..3000ms tail 뒤 재조준한다.
- ALL은 같은 counter 구간을 거친 뒤 impact 시 ALL을 재검사한다.
  `EXECUTE_GRABBED_PLAYERS`가 HP 0·DEAD·attachment 정리를 commit하고 tail 끝에서 패턴을 종료한다.
- 유효 counter는 실제 420631 start/loop/end occurrence와 attachment 해제를 사용하는 GROGGY로 진입한다.

`ALL_GRABBED_RELEASE`는 만들지 않았다. animation stage slice와 root motion은 기존 projection 경로를 사용한다.
기존 left-hand bone presentation과 hit 위치 기준 boss-local fallback offset을 유지했다.
canonical grip pose bake와 보조 Effect/sound cue 이관은 이번 최신 첨부 계약의 구현 범위에 포함하지 않았다.

### 분류·freshness·원자적 impact

`CValtanBrain::Classify_GrabbedPlayers`가 살아 있는 비어 있지 않은 참가자 집합을 분모로 삼는다.
HP 0, DEAD, FALLING은 제외하지만 `isCombatReady=false`는 제외 사유가 아니다.
현재 boss의 `BOSS_LEFT_HAND`, 현재 nonzero pattern sequence, finite attachment transform만 유효 포획이다.
wrong owner/slot, stale sequence, malformed attachment는 ALL로 인정하지 않는다.

typed damage/execute는 대상·profile·event capacity를 먼저 검사한 후 player 사본을 준비한다.
모두 성공해야 player 상태와 damage event를 commit하며, 두 번째 대상 오류나 capacity 부족 시
첫 번째 대상만 죽거나 attachment만 풀리지 않는다. 처형은 일반 damage 비율 조정으로 대신하지 않는다.

전멸 뒤 targetless clock의 근거는 실제 commit한
`iGrabExecutionCommittedPatternSequence`와 `iGrabExecutionCommittedStageIndex`다.
패턴 이름만으로 허용하지 않는다. `FinishPattern`의 typed terminal receipt가 COMPLETED와 ABORTED를 구분한다.

`RELEASE_GRABBED_PLAYERS` 역시 player-copy staging을 사용한다. 마지막 검토에서 발견한
실패 후 cleanup 문제를 보강하여, 이전 stage EXIT 또는 다음 stage ENTER의 grab action preflight 실패 시
기존 attachment를 보존하고 boss를 rollback한다. room은 not-ready가 되며 부분 전이 snapshot을 내보내지 않는다.
counter ENTER 및 terminal EXIT의 두 번째 대상 오류를 실제 `Update_WorldEntities` fixture에 추가했다.
EXIT fixture는 임시 bootstrap을 정상 hash/loader로 admission하며 제품 bootstrap이나 live catalog를 직접 수정하지 않는다.

### Client 입력과 Next 연결

GRABBED snapshot 중 `CPlayerController::Update`는 ground preview·held skill·BA 상태를 취소하고
command sink 전에 반환한다. ReleaseSkill도 보내지 않는다.
잡혀 있을 때 누르고 있던 키/LMB/RMB는 attachment 해제만으로 재실행되지 않으며 실제 물리 release 뒤 재입력해야 한다.
공개 `Request_MoveToPoint`, `Request_Revive`, `Request_DebugKillSelf` 경로에도 GRABBED admission을 적용했다.
Next 예약은 일반 gameplay 입력과 별도 typed tool command이므로 포획 중에도 유지된다.

Trash 전멸 tail이 끝난 뒤 예약 Next는 살아 있는 대상이 없으면 WAITING_FOR_PLAYER로 남는다.
실제 typed revive 이후 다음 fixed tick에 같은 pinned revision의 B를 한 번 시작하는 통합 fixture를 추가했다.
자동 revive·중앙 이동·월드 복구는 추가하지 않았다.

### Data와 publisher

split source의 Trash만 scoped authoring한 뒤 기존 Product projection과 root-motion 문서를 갱신했다.
Gameplay bootstrap은 v25이며 `PATTERNAUTHORINGMANAGED` row가 split 정본의 관리 패턴 집합을 전달한다.
Next admission이 사용할 목록을 C++에 개수나 ID 배열로 하드코딩하지 않았다.
Server/Client parser와 publisher는 새 typed action의 trigger/value/duration/target 및 profile reference를 검증한다.

현재 Desktop의 split 관리 패턴은 29개, 공용 Tool inventory는 27개다. 저장 Flow는 통합 당시
28슬롯/ordinal30에서 사용자가 저장한 22슬롯/ordinal31로 바뀌었으며 현재 저장값을 그대로 유지한다.
projection 후 전체 Encounter는 53 patterns/234 stages이며 Trash 14 stage와 보조 패턴을 유지한다.
저작 스크립트는 Trash scoped 경로와 기본 경로의 저장된 cue/Unlink 보존을 함께 유지한다.

추가 native 검증에서 Client의 별도 읽기 전용 `CEncounterPatternReference`가 새 branch와 typed action을
거부하는 누락도 확인했다. 같은 v4 계약을 이 소비자에 연결하고 기존 Action harness에 실제 문서 로드와
15개 잘못된 입력·rollback 회귀를 추가했다. 통합 후보에서도 이 회귀와 호출부를 보존했으며,
최신 Desktop 재빌드·실행 결과는 통합 RESULT에 기록한다.

## G03. 실제 수행한 검증

### Desktop 통합 후 publisher 실행

증거는 [Desktop 통합 publish 로그](C:/w/valtan-arena-next/_work/valtan-arena-next/desktop-integrated-publish.log)다.
로그의 출력 대상은 `C:/Users/user/Desktop/LostArk`이며 다른 worktree의 생성물을 완료 증거로 대신하지 않는다.

| 검증 | 실제 결과 |
|---|---|
| split source `PublishV2` projection | exit 0; 관리 패턴 29개 validation 포함 |
| root-motion 생성 | exit 0; 43 patterns/101 stages/5274 samples |
| provenance receipt 동기화 | exit 0; 추가 PROJECT_TUNED 변경 0 field |
| WorldDestruction·Navigation publish | 각각 exit 0 |
| Gameplay balance 및 world/item runtime set Validate·Publish | exit 0; 6 player profiles, 230 skill rows, 109 damage profiles, 3 bosses, 53 patterns/234 stages, 52 timeline rows |

### Desktop 후속 빌드·실행 증거

아래는 각 로그의 실행 시점별 결과다. 최신 Effect·조기 회전 수정의 후속 결과와 정본 전체 자동화 상태는 문서 말미에서 구분한다.

| 검증 | 확인한 결과·로그 |
|---|---|
| Server 전체 계약 | [Debug](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-contract-Debug.log)·[Release](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-contract-Release.log) 모두 failures 0/exit 0. Trash NONE/PARTIAL/ALL/COUNTER, freshness·원자성 및 RELEASE ENTER/EXIT rollback PASS |
| Release core·Server 빌드 | [core](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-core-build.log)의 Engine·UpdateLib·Shared·관련 harness와 [Server](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-release-build.log) 모두 exit 0 |
| Network protocol·Next service | Network [D](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-NetworkProtocolHarness-Debug.log)/[R](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-NetworkProtocolHarness-Release.log) failures 0. Next service [D](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-ValtanPatternAuditionServiceHarness-Debug.log)/[R](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-ValtanPatternAuditionServiceHarness-Release.log) 각각 14/14, exit 0 |
| Release live 회귀 | [실행 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-live-regression.log): FourPlayer, CharacterSelect Core, Party2, Party4 PASS; 두 wrapper 모두 exit 0 |
| Client native 계약 | [실행 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-native-regression-first-pass.log): Action D/R 및 Trash parser 15 rejection/rollback PASS. EffectRender·PointLight Debug exit 0 |
| shader/resource 및 Release native | [후속 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-shader-resource-release-native.log): 최초 Debug shader 검사 exit 1 뒤 D/R 재실행 exit 0, resource-root 각각 8 cases PASS, EffectRender·PointLight Release exit 0 |
| native exit 전달 | [D/R 검사](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-native-exit-propagation.log) exit 0. 의도한 native 실패 코드 37 및 실제 잘못된 resource root의 코드 2를 보존하는 검사 포함 |
| FOUR 복구 후 Master V2 | [별도 재실행](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-followup-python-master-v2-after-four-recovery.log) 53 tests PASS, 342.299초; [종료 결과](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-followup-python-master-v2-after-four-recovery.result.json) exit 0 |
| Client Release 첫 빌드 | [빌드 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-client-release-build.log) 최초 LNK2001/LNK1120, exit 1 이력. 이후 최신 소스의 Debug/Release 재빌드는 모두 exit 0 |

Master V2의 앞선 실행은 테스트 도중 source revision이 바뀌어 failures 5/errors 6으로 끝났다.
이를 숨기거나 PASS에 합산하지 않고, FOUR 복구 후 별도 53개 재실행 결과와 구분한다.

### 전체 Desktop 통합 전 검증 이력

다음은 `C:/w/valtan-arena-next`에서 수행한 이력이다. 경로가 `_work/`로만 적힌 로그도 이 worktree 기준이다.
54 patterns/236 stages와 44 root-motion patterns는 FBF 퇴출 통합 전 수치이며 현재 Desktop 수치가 아니다.

| 검증 | 실행 결과·범위 |
|---|---|
| `python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py` | 52 tests PASS, 317.737초; `_work/trash_master_v2_tests.log` |
| 같은 suite의 `-k per_set_weight` | 관리 패턴 집합 assertion 보강 후 1 test PASS, 13.452초; `_work/trash_managed_inventory_tests.log` |
| `author_valtan_phase_two_mechanics.py --pattern-id VALTAN_TRASH --mode Validate` | PASS; Trash scoped source 검증 |
| `Project-ValtanPatternMaster.ps1 -Mode PublishV2` | PASS; 기존 projection 경로 사용 |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS; 당시 6 player profiles, 230 skills, 109 damage profiles, 3 bosses, 54 patterns, 236 stages, 52 timeline rows |
| root-motion 생성 | PASS; 기존 Resources를 읽기 전용으로 사용하고 새 worktree에 44 patterns/103 stages/5421 samples 생성 |
| 실제 `GameRoom.cpp` + 전체 `ServerGameplayContractTests.cpp` MSVC `/Zs` Debug | 마지막 RELEASE rollback fixture 포함 PASS; `_work/trash_followup_syntax.log` |
| 실제 `PlayerController.cpp` MSVC `/Zs` Debug·Release | PASS; physical release 및 공개 command fake-sink static_assert 포함; `_work/trash_input_syntax.log` |
| 변경 C++ 파일 `git diff --check` | PASS; 전체 파일 EOL 정규화는 통합 담당 단계로 남김 |

위 선행 worktree의 Python 실행 횟수는 **53회 = 서로 다른 52개 + 보강 후 1개 재실행**이다.
이는 후속 Desktop의 서로 다른 53개 Master V2 검사와 별개다.
Client 문법 검사는 기존 mixed-encoding SDK header의 경고를 남겼으며 해당 header를 변환하지 않았다.
문법 검사와 compile-time 검증은 제품 링크·network packet 관측·시각 검증을 대신하지 않는다.

## G04. 통합 실행과 남은 확인

선행 worktree의 초기 전체 Server failures 14는 수정 전 이력이다. 그 worktree의 마지막
Shared/Server/Client Debug·Release 빌드는 통과했고, Server 전체 계약은
Debug 932개 PASS 행, Release 855개 PASS 행으로 둘 다 failures 0이었다.
Release의 Debug 전용 검사 제외를 실행 PASS로 세지 않는다.
RELEASE counter ENTER와 terminal EXIT rollback도 실제 `Update_WorldEntities` 실행에 포함됐다.
EXIT fixture는 loader가 요구하는 `Gameplay.bootstrap` 파일명을 고유 임시 폴더에서 사용하도록 수정했다.
실제 Next service 14/14 및 ValtanFourPlayer/CharacterSelectIsolation Debug·Release 네 실행도 통과했다.

초기의 정본 Debug 실행에서 발견한 worktree `Resources/Fonts` 누락은 이후 해결한 과거 문제다.
현재 Desktop 빌드 중지 원인으로 기록하지 않는다.

Desktop Lerp 수정 전후 Debug 빌드와 사용자 빌드 때문에 자동 실행을 중지했던 기록은 선행 이력이다.
현재 확인한 후속 빌드·실행 결과는 G03의 Desktop 로그 표를 따른다.

Client Release 로그의 확정 오류는 `CActionPresentationTimeline::Resolve_CuePreviewTimelineTime`의
LNK2001과 최종 LNK1120/exit 1이다. 통합 담당의 소스·산출물 대조 인계에서는 함수 정의가 존재하며
이전 Timeline.obj와 새 Effect_Tool.obj가 혼재한 것으로 확인했다. 소스 정의가 있다는 사실만으로
링크 성공을 대신하지 않았다. 이후 최신 Client D/R 재빌드도 모두 exit 0을 확인했다.
점프찍기→휠윈드 방향 수정의 실제 Room tick 회귀도 D/R 모두 통과했으며 기존 Trash PASS로 대체하지 않았다.

사용자 시각 확인에서는 `F1 -> Boss Tool -> Pattern Flow`의 Trash isolated 재생으로
포획 손 위치, 0..200ms counter, 실제 GROGGY, 부분 포획 내려찍기·해제·재조준,
전원 전멸 후 animation tail, Next WAITING 및 사용자 revive 뒤 이어짐을 확인해야 한다.
포획 중 입력한 키와 마우스를 계속 누른 채 해제돼도 새 gameplay command가 시작되지 않는지 함께 확인한다.
사용자가 관찰·판정하기 전에는 visual PASS로 기록하지 않는다.

## 현재 통합·사용자 확인 경계

Server 06:52:32·Client 06:57:55의 사용자 Debug 링크·배포 확인은 이전 시점의 기록이다.
사용자 Server/Client 종료 뒤 최신 source로 D/R 빌드를 완료했다. 정본 전체 D/R 자동화는 실행 중이며
사용자 Client/UI를 자율 실행하지 않았다. 사용자 visual PASS는 선언하지 않는다.

### 최신 Desktop D/R 빌드 및 후속 수정

`_work/valtan-arena-next/desktop-client-debug-latest-build.log`와
`desktop-latest-publish-and-client-release.log`에서 최신 Client Debug/Release 링크·배포 exit 0을 확인했다.
빌드 전후 Client/Shared source hash도 같았다. Server는
`desktop-server-whirlwind-build-regression.log`에서 D/R 빌드와 전체 계약이 모두 exit 0/failures 0이다.
이 실행에는 점프찍기·준비·휠윈드 중 방향 고정, 종료 동작 ENTER 재조준, 재실행·대상 부재 회귀를 포함한다.
Six Pizza 카메라 제거와 다른 작업의 빈 발악 Draft·사자후/3시 첫 클립 0초 연결도 정본에 보존했다.

최종 `Invoke-BuildAndRegression.ps1 -Configuration Debug/Release -SkipBuild`는 별도 구성별 로그로
진행 중이다. `-SkipBuild`는 이미 실제로 완료한 최신 D/R 빌드를 다시 반복하지 않기 위한 옵션이며
빌드 실패를 우회한 것이 아니다. 전체 종료 결과는 통합 RESULT와 같은 상태로 갱신한다.

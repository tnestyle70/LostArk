# 발탄 파괴 벽·Nav·몸체 반경 수정 결과

작성일: 2026-08-28. 현재 적용 브랜치는 `codex/valtan-arena-next-desktop`,
위치는 `C:/Users/user/Desktop/LostArk`, 기준 main은 PR #251 `a6871a2f`다.
선행 worktree의 벽·Nav·몸체 변경과 다른 작업의 확정 변경을 Desktop에 통합했다.
통합 시점 publisher chain과 최신 Client·Server Debug/Release 빌드·Server 계약은 통과했다.
Client Release의 첫 LNK2001/LNK1120은 최신 소스 재빌드로 해소했고,
점프찍기→휠윈드 조기 회전 수정도 실제 Server tick 회귀를 두 구성에서 통과했다.
UI PR #252/기능 PR #253 병합 뒤 정본 Release 전체 자동화는 exit 0이다. Debug 빌드·계약·native도
통과했으며, 실행 중인 사용자 세션 보호로 Debug 전체 live 재실행과 시각 판정은 별도로 남아 있다.

계획은 [벽·Nav PLAN](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_DESTROYED_WALL_NAVIGATION_IMPLEMENTATION_PLAN.md),
전체 빌드·다른 작업 통합·사용자 대기는
[통합 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_RESULT.md)에 기록한다.

## G01. 벽이 안 보이는데 돌진과 이동을 막던 원인

### 입구 source와 receiver가 다른 leaf인 경우

frontwallA/B의 source collider는 숫자 leaf이고 impact receiver는 같은 group 아래의 형제 leaf다.
기존 파괴 mutation은 source leaf만 비활성화하여 receiver 두 개가 남을 수 있었다.
Server swept collision은 그 receiver를 계속 읽으므로 벽의 presentation이 사라져도 플레이어와 발탄을 막는다.
따라서 사용자가 지적한 **기존 벽의 보이지 않는 충돌이 남는 원인은 실제로 있었다.**
다만 모든 돌진 정지를 이 원인 하나로 단정하지 않는다. 살아 있는 다른 벽, 지형 경계 및 큰 몸체 반경도
정지 위치에 영향을 준다. 아래 Nav와 반경 문제를 별도로 확인·수정했다.

[Publish-ValtanWorldDestruction.ps1](C:/Users/user/Desktop/LostArk/Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1)은
입구 mutation에 source와 receiver를 함께 소유하는 정확 group prefix를 사용한다.
다른 group의 collider가 그 prefix 아래 섞이면 publish를 거부한다.
99개 벽의 141개 collision box가 정확히 한 mutation에 소유되고 최종 해제 범위에 포함되도록 검증한다.

BREAKING 동안에는 receiver만 끄고 실제 벽의 이동 차단은 남긴다.
DESPAWNED의 due tick에서 source collision과 해당 Nav를 함께 해제한다.
아직 서 있는 이웃 벽까지 여는 변경은 하지 않았다.

### 실제 벽과 다른 셀을 막던 Nav authoring

기존 nearest-wall/nearest-cell 배정은 벽과 떨어진 셀도 특정 벽의 소유로 넣을 수 있었다.
XZ만 비교하는 경우에는 다른 층 벽이 현재 바닥의 통로를 막을 수도 있었다.

새 [ValtanWallNavigation.ps1](C:/Users/user/Desktop/LostArk/Tools/WorldPipeline/ValtanWallNavigation.ps1)은
source collision의 XZ OBB와 셀 사각형의 실제 교차, base walkability,
셀 ground Y에서 시작하는 Server body 수직 범위를 함께 검사한다.
BossProfile radius 1.4m의 body span 2.8m와 기존 player span 1.8m를 소비한다.
없는 겹침을 nearest cell로 대신하지 않는다. grid 구조와 비정상 높이도 거부한다.

기존 split tool의 `RebuildNavigation`/`CheckNavigation`과 publisher가 이 계산을 공유한다.
repair는 stage/rollback을 거쳐 정본을 교체한다. 별도의 Client 이동 경로를 만들지 않았다.

### 경로 단축이 동적 blocker를 가로지르던 경로

[ServerNavigation.cpp](C:/Users/user/Desktop/LostArk/Server/Private/ServerNavigation.cpp:495)의 `Has_LineOfSight`가
현재 dynamic blocker를 함께 검사하도록 수정했다.
고정 간격 표본은 셀 모서리를 짧게 가로지르는 구간을 놓칠 수 있어 DDA/supercover로 실제 교차 셀을 모두 검사한다.
정확한 모서리·경계에서는 양쪽 셀을 확인한다.

이는 플레이어 path smoothing과 발탄 navigation 소비 경로에 같이 적용된다.
`Sample_Position`의 높이 조회와 낙하 판정용 소비는 그대로 유지했다.

## G02. Desktop 적용 결과와 보존한 경계

| 항목 | 현재 값·상태 |
|---|---|
| 파괴 가능한 독립 벽 | 99개 |
| collision box | 141개, mutation 소유 유일성 검사 |
| wall Nav region | 98개 |
| wall cell 참조 / 서로 다른 cell | 3905 / 2109 |
| floor collapse region | 기존 6개 보존 |
| 전체 blocker region | 104개 |
| base grid | 392×312, cell 0.5m, walkable 21524개, 원본 bytes 변경 없음 |

`outerwall109.1090000000000012`는 같은 층의 base-walkable cell과 실제 교차가 없어 wall Nav region을 만들지 않는다.
그 벽의 collision과 파괴 상태는 유지한다. 99개 벽에 99개 Nav region을 억지로 맞추지 않는다.

109 이벤트의 기존 **외벽 30 + 내벽 67** 파괴 스케줄은 보존했다.
109만 끝났다고 입구 두 벽까지 자동으로 파괴된 것으로 취급하지 않는다.
실제 입구 파괴가 완료되면 각 source와 receiver가 함께 사라진다.

6개 floor region의 condition, cell 순서와 `BLOCK_WHILE_FRACTURED` polarity는 base와 같다.
벽을 파괴했다고 붕괴된 바닥까지 이동 가능하게 만들지 않는다.

authoring은 `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`와
`Data/Encounters/Valtan/ValtanWorldEvents.json`이다. Client 쪽 DataFiles는 publisher로 생성했다.
Desktop 통합 후 WorldDestruction publish revision은
`d4c6de9061f579a8cd945002308f5dd5cb21bbc2708be2cd4d54ede1fcc70f4b`다.
양쪽 작업의 통합 뒤에도 위 99개 벽·141개 collision·98개 wall Nav와 기존 6개 floor region을 유지했다.

## G03. 현재 발탄 크기에 맞춘 body collider

| 항목 | 변경 전 | 반영한 값 |
|---|---:|---:|
| BossProfile body radius | 3.0m | 1.4m |
| body diameter | 6.0m | 2.8m |
| presentationScale | 1.0 | 1.0 유지 |

[BossProfiles.json](C:/Users/user/Desktop/LostArk/Data/Balance/BossProfiles.json)의 radius를 바꿨다.
기존 Client Debug collider가 Server spawn radius를 소비하므로 Client에 다른 반경이나 추가 scale을 넣지 않았다.
예전 인계의 0.75 scale 값을 현재 모델에 다시 곱하지 않는다.

실제 body WModel과 AnimSet의 idle/run 99개 frame을 skin 계산한 결과 다리 최대 XZ 반경은 1.3781m,
몸통은 0.982m였다. 이 body 범위를 올림하여 1.4m로 잡았다.
무기·뿔·뻗은 팔을 이동 몸체 반경에 포함하지 않았다.
측정 입력·결과는 [_work/body-measurement.json](C:/w/valtan-arena-next/_work/valtan-arena-next/body-measurement.json)에 있다.

provenance receipt를 PROJECT_TUNED와 실제 측정 근거로 갱신하고 balance publisher를 사용했다.
공격 hit shape, 사거리, 장판 반경은 함께 축소하지 않았다.
body가 작아지면 player hit 접점·miss와 wall sweep 접점은 달라지므로 관련 fixture도 실제 profile 기준으로 바꿨다.

## G04. 실제 검증과 남은 확인

### Desktop 통합 후 실행한 publisher

증거는 [Desktop 통합 publish 로그](C:/w/valtan-arena-next/_work/valtan-arena-next/desktop-integrated-publish.log)다.
출력 대상은 실제 Desktop 저장소이며 이 단계로 최신 바이너리의 실행 검증까지 완료 처리하지 않는다.

| 검증 | 실제 결과 |
|---|---|
| split source projection·root motion·receipt | 모두 exit 0; root motion 43 patterns/101 stages/5274 samples |
| WorldDestruction publish | exit 0; 위 revision 유지 |
| Navigation publish | exit 0; Valtan grid 392×312, cell 0.5m, walkable 21524개 |
| Gameplay balance·world/item runtime set Validate·Publish | exit 0; body radius 1.4m 유지, Encounter 53 patterns/234 stages |
| 최신 통합 Debug/Release regression | UI 병합 후 Release 정본 전체 exit 0, Debug Client 빌드 오류 0 및 Protocol/Next/Server/Action/Effect/PointLight exit 0. Debug 전체 live 재실행은 사용자 실행 중이므로 보류 |
| 사용자 시각 판정 | 미실행; visual PASS 없음 |

현재 통합 데이터는 split 29개, 공용 Tool inventory 27개다. 통합 당시 Flow는 FBF의
`slot.000017`만 제거한 28슬롯/ordinal30이었으며, 이후 사용자가 저장한 현재 Flow는
22슬롯/`nextSlotOrdinal=31`이다. 사용자의 추가·삭제·재정렬을 보존하고 초기값으로 되돌리지 않았다.
Trash 14 stage도 보존했다. 이 개수는 아래 선행 worktree 실행 시점과 구분한다.

### Desktop 후속 검증 로그

| 검증 | 확인한 결과·로그 |
|---|---|
| wall geometry·floor 보존 | [Wall navigation Check](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-wall-navigation-check.log): 99 exact wall footprints와 floor regions 보존, 대응 exit 파일 0 |
| WorldDestruction | [Validate](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-world-destruction-validate.log)·[ContractTest](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-world-destruction-contract.log) PASS, 각 exit 0; 위 revision 유지 |
| Server navigation | [Validate](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-navigation-validate.log) 4개 Area 통과, exit 0. Valtan 392×312·0.5m·walkable 21524 유지 |
| Server 전체 계약 | [Debug](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-contract-Debug.log)·[Release](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-contract-Release.log) 모두 failures 0/exit 0 |
| Release core·Server | [core 빌드](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-core-build.log)의 Engine·UpdateLib·Shared·관련 harness 및 [Server 빌드](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-server-release-build.log) exit 0 |
| Release live 회귀 | [실행 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-release-live-regression.log): FourPlayer, CharacterSelect Core, Party2, Party4 PASS |
| shader/resource·native | [첫 native 실행](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-native-regression-first-pass.log)에서 Action D/R·EffectRender/PointLight Debug exit 0. [후속 실행](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-shader-resource-release-native.log)은 최초 Debug shader exit 1 뒤 D/R shader·resource 검사와 Release EffectRender/PointLight exit 0 |
| Client Release 첫 빌드 | [빌드 로그](C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/desktop-client-release-build.log) 최초 LNK2001/LNK1120, exit 1 이력. 이후 최신 소스의 Debug/Release 재빌드는 모두 exit 0 |

native/자동 검증은 사용자 화면 판정을 대신하지 않는다. Next service D/R 14/14와 Trash parser 15개 회귀의
실행 증거는 [Trash RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_VALTAN_TRASH_CAPTURE_COUNTER_SERVER_FLOW_RESULT.md)에 기록했다.
최신 Effect 소스 재빌드와 조기 회전 수정의 D/R native 통과는 아래 후속 검증에 구분해 기록한다.

### 전체 Desktop 통합 전 검증 이력

아래 PASS는 `C:/w/valtan-arena-next`에서 수행한 증거이며 최신 Desktop 합본의 재실행 결과가 아니다.

| 검증 | 결과 |
|---|---|
| Navigation publish/check | 실제 geometry 기반 authoring 일치 PASS |
| WorldGameplay publish | PASS |
| WorldDestruction ContractTest | prefix ownership, 최종 collision 해제, 높이·잘못된 수치 거부 포함 PASS |
| 실제 Server 첫 통합 실행 | 109 후 입구 보존, BREAKING, frontwallA/B 해제, 99벽 최종 collision/Nav 0, 통과 이동 PASS |
| 실제 Server path 회귀 | 동적 벽 앞 우회, 해당 벽 파괴 뒤 직선 단축 PASS |
| DDA 집중 probe | 9개 경계 검사 PASS. 고정 표본의 corner miss 44건이 새 검사에서 0건 |
| DDA 독립 비교 | 20만 segment에서 셀 교차 판정·역방향 대칭 불일치 없음 |
| 실제 모델 composition suite | 11 tests PASS |
| Dash fixture 수정 후 실제 GameRoom 집중 실행 | 기존 20개 및 stale receiver 보강 21/21 PASS |
| 선행 worktree 마지막 Server Debug/Release 전체 계약 | 둘 다 failures 0. 위 navigation·벽·돌진 검사 포함 |
| 선행 worktree Client Debug/Release 빌드 | PASS. 최신 통합·사용자 시각 검증을 대신하지 않음 |
| publisher 반환값 회귀 | counter proxy 수치 3개가 artifact 반환값에 섞이는 오류 재현(4개 결과). validator 반환을 소비한 뒤 단일 artifact 검사와 전체 ContractTest/Validate PASS |

첫 Server 전체 실행은 다른 assertion을 포함해 failures 14였다. 이는 수정 전 이력이다.
그중 Dash 네 개는 고정 3m 예상 접점과 새 1.4m 접점의 불일치로 확인하여 fixture를 수정했다.
새 product charge 동작을 추가한 것이 아니며 main에 이미 있는 충돌 fixed tick의 stop/GROGGY 계약을 유지했다.

stale outer receiver 회귀는 목표 source/receiver 두 collider로 격리하고 실제 stable ID·접촉 tick/fraction을
검사하도록 보강했다. group version과 mutation/event 상태 불변도 확인하며 전체 collision 집합을 복원한다.
실제 집중 실행은 21/21 PASS, 격리를 제거한 대조는 identity assertion 실패로 오검출 방지 효과를 확인했다.

추가 publisher 검토에서는 수치를 반환하는 `Assert-JsonNumber` 세 호출이 compiler의 성공 출력에 그대로 섞이는
문제를 재현했다. 반환을 `$null`로 소비하고 기존 ContractTest에 단일 artifact 결과 검사를 추가했다.
`wall-compiler-return-before.log`의 실패(4개 결과) 뒤 `wall-compiler-return-after.log`에서 전체 검사가 통과했다.
revision `d4c6de9061f579a8cd945002308f5dd5cb21bbc2708be2cd4d54ede1fcc70f4b`는 동일하여 파괴 데이터 값은 바뀌지 않았다.

초기 worktree 정본 Debug 실행의 `Resources/Fonts` 누락은 이후 해결된 과거 문제다.
현재 Desktop의 검증 중지 사유로 취급하지 않는다.

Desktop Lerp 수정 전후 Debug 빌드와 사용자 빌드로 자동 실행을 중지했던 기록은 선행 이력이다.
현재 후속 검증 상태는 위 Desktop 로그 표를 따른다.
Client Release 로그는 `CActionPresentationTimeline::Resolve_CuePreviewTimelineTime`의 LNK2001 및
LNK1120/exit 1을 남겼다. 함수 정의 존재와 이전 Timeline.obj·새 Effect_Tool.obj 혼재는
통합 담당이 실제 source/hash로 확인했다. 이후 최신 Client D/R 재빌드도 모두 exit 0이다.
점프찍기→휠윈드 방향 수정은 별도 Room tick 회귀로 D/R 모두 통과했으며
이전 돌진·벽 PASS를 대신 근거로 사용하지 않았다.

에이전트는 Client/UI 실행·캡처로 시각 결과를 판정하지 않는다.
사용자는 파괴 전후 벽 통과, 입구 두 벽의 독립 해제, 살아 있는 이웃 벽과 붕괴 바닥의 차단 유지,
발탄의 이동 몸체 크기와 돌진 정지 위치를 직접 확인해야 한다. 사용자 판정 전에는 visual PASS로 기록하지 않는다.

## 현재 통합·사용자 확인 경계

Server 06:52:32·Client 06:57:55의 사용자 Debug 링크·배포 확인은 이전 시점의 기록이다.
사용자 Server/Client 종료 뒤 최신 source로 D/R 빌드를 완료했다. 이후 UI 병합본 Release 전체 검증까지 완료했으며
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

최종 PR #253/main `eec64f8d` 통합 및 08:49 Release 전체 완료 증거, 이후 사용자 저장값과 Debug 실행 경계는
`2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_RESULT.md`의 G07을 따른다.

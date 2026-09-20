# 쿠크 게시와 입장 이펙트 준비 시간 개선 결과

## G00. 기준선과 보존

main을 `9f2a9feb0dff04164f23bbc78ac50222c91aa326`까지 fast-forward한 뒤
`GB/koukubugfix-bingo`를 만들었다. 이전 브랜치는
`codex/terminal-drain-cleanup-20260919`의 `1316d572b`였다.
기존 Encounter와 patternbindings의 미커밋 변경은
`safety/2026-09-20-before-main-koukubugfix-bingo` stash에 보존하고 새 브랜치에
충돌 없이 복원했다. stash는 삭제하지 않았다. LAN 설정은 server-host/reachable이었다.

## G01. Save와 Publish의 실제 차이

revision1902의 Workbench 로그
`out/KoukuSaydon/KoukuSaydonComposition.51692.124482734.publish.log`는
projection → map REUSED → world publish → Gameplay 내부 projection 재검증을 보여준다.
다음 실행인 `124984625.publish.log`는 네 domain 모두 REUSED다.
매 클릭마다 모든 domain을 무조건 다시 게시한다는 설명은 현재 구현과 다르다.

| 기존 실제 receipt의 action | 시간 |
|---|---:|
| koukusaydon.product | 179597ms |
| world.gameplay | 4714ms |
| gameplay.balance | 240316ms |

map.kakulsaydon의80354ms는 전날 receipt이며 위 실행은 재사용했으므로 합산하지 않는다.
새 read-only PowerShell5.1 실측에서 domain fingerprint는 각각204/61/72/702ms,
source revision JSON 읽기는208ms였다. 원본 snapshot, input hash와 출력 fingerprint
검사를 지워 해결할 문제가 아니다.

| revision1902 데이터 | 크기·수량 |
|---|---:|
| Composition | 2239731bytes |
| Encounter | 27934425bytes |
| Encounter compact JSON | 10531195bytes |
| Collider worldTrack | 147개 / 32791keys |
| worldTrack JSON | 8484613bytes |
| Gameplay.bootstrap | 13301358bytes |
| Collider key bootstrap 행 | 11407768bytes |

Encounter의62.30%는 공백이지만 나머지 대부분은 실제 Server collider의 위치·scale·
visibility·grip 궤적이다. `world_object_collider.py`가 모델·본·방출·배치 변환을 bake하고
`GameplayCatalog.cpp`와 `KoukuSaydonLogicRuntime.cpp`가 소비한다. Save는 저작 문서를
저장하고 Publish는 이 궤적을 추가 계산하므로 둘의 작업량이 같지 않다. 수분 지연을
파일 크기 하나로 설명하거나 키를 임의로 줄이지 않는다.

## G02. 기존 실제 Debug 이펙트 시간

`Client/Default/EffectFailure.user.log`의 PID51692에서 기록된 느린 쿠크 V1은116개다.
시작·완료 구간은333646ms(23:57:24.265~00:02:57.911), 클래스·marker를 포함한
161개 기록 구간은398001ms다.9/19의5개 세션에서도 쿠크321.6~360.0초,
전체 기록386.0~433.2초가 반복됐다. 개별 무거운 scene04a는85.156초/79.390초,
flame wave는65~67초, spinning.card는70.172초였다.

이는50ms 이상인 V1 준비 기록의 interval이다. 병렬 작업의 개별 시간을 합산한 값이
아니며 V2와 World activation, map/character 전체 시간을 포함하지 않는다.
`164개 전체=600초`를 실제 계측 결과로 확정하지 않는다. 기존 V1 경로는 이미
3개 window 및 공용 budget4로 병렬 처리한다. 근거 파일은
`out/KoukuEffectLoad20260920/existing-runtime-timings.json`이다.

## G03. 게시 진행 로그

`Invoke-BuildDomainOwner.ps1`은 기존 Workbench 보존 로그에 owner lock 대기 시간,
domain START와 PASS/REUSED의 elapsedMs, owner 전체 완료 시간을 추가한다.
게시 순서·입력 검증·rollback은 유지한다. 기존
`KoukuDomainOwnerTransactionTests.test_final_domain_failure_restores_patterns_world_and_receipts`
1개를 실행하여 최종 domain 실패 시 Pattern/World/receipt 복구를 확인했다.
PowerShell parser와 해당 diff 검사를 통과했다.

## G04. Debug lazy 적용과 CPU 실측

현재 sourceRevision1902 closure는117 V1+45 V2=162개다. 어제119+45=164개에서
`4219801.full.restore`, `4219877.full.restore` 두 참조가 빠졌다. BossCatalog와
겹치는 bluecircle1종을 유지하므로 Debug가 추가로 선준비하지 않는 V1은116종이다.
`current-closure.json`에 실제 대상 목록을 보존했다.

`Level_Loading.cpp/.h`, `Level_KakulSaydonArena.cpp`,
`KoukuSaydonPresentationPlayer.cpp`를 변경했다. Debug는 전체 raid closure 수집,
V2 전체 선준비, enabled World326개 전체 준비를 생략하고 기존 first-use 경로를
사용한다. class/marker/BossCatalog 및 Joker/card7개 clone 준비는 유지한다.
Release는 기존 전체 준비와 필수 실패의 입장 거절을 유지한다.

기존 standalone probe를 복구해 어제164 closure의 V1 JSON119파일/220.8MB를 실제
`CDataJson.cpp`로 읽었다. warm file cache, codec decode와 GPU 준비를 제외한 실험이다.

| 설정 |1worker|3workers|
|---|---:|---:|
| Debug 제품 DataJson(`/O2`, `/MDd`) |32.695초|87.466초|
| Debug 역순 재측정 |34.979초|98.665초|
| Release(`/O2`, `/MD`) |3.269초|1.232초|

Debug에서 worker 추가가 오히려 느려지는 경향을 재현했다. heap/allocator 경합과
부합하지만 stack profile 없이 그 원인을 확정하지 않는다. 이 실험을 실제119개
prewarm 총시간으로 설명하지 않는다. Release는 기존 병렬 경로를 유지한다.
근거는 `parse-debug.json`, `parse-debug-reverse.json`, `parse-release.json`이다.

기존 `EffectFailure.user.log`에 `Kouku.Loading.Begin`의 설정·정책,
Collect/V1Settled/V2Prepared/WorldPrepared/GateReady의 수량·시간,
`Kouku.Arena.Ready`의 Initialize 시간을 추가했다. Loading 시작부터 실제 Arena Ready
timestamp까지 비교하면 다음 사용자 실행에서 전체 경계를 확인할 수 있다.
일부 단계는 병렬이므로 각 elapsed를 무조건 합산하지 않는다.

독립 읽기 검토에서 V1 Pending은 row.failed로 고정되지 않고 재시도되며 World cue도
보존·재시도·경과시간 보정하는 것을 확인했다. V2는 기존 snapshot/Play_Group에서
준비한다. Joker/card template의 effectTracks는 비어 있었다. 이 검토는 GPU 실행
또는 frozen commit 승인 판정이 아니다.

변경3개 TU를 정본 VS18 Insiders/14.44 x64 환경에서 Debug/Release `/Zs`로 각각
검사했고 모두 성공했다. 인코딩과 BOM/CRLF를 보존했다. 로그는
`out/KoukuEffectLoad20260920/compile-changed-{Debug,Release}.log`다.
표준 EXE 링크는 별도 Product 결과로 기록한다. Client/UI 실행·종료·화면 캡처는
수행하지 않았다.

## G05. Product 빌드와 사용자 확인 경계

표준 Release Product 명령을 실행했으나 `ProductOutputGuard.psm1:85`에서
실행 중인 Debug Client PID51692와 Debug Server PID11692를 발견해 컴파일 전에
중단했다. 이 guard는 선택한 Release뿐 아니라 Debug/Release의 실행 프로세스를
모두 검사한다. 따라서 Release EXE 자체의 잠금을 확인한 결과로 설명하지 않는다.
명령은 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
-MaxCompilerProcesses 2`이며, 실패 receipt는
`out/BuildPipeline/runs/20260920T003417810Z-release-product.json`이다.

사용자에게 저장 후 두 프로세스 종료를 요청했고 자동 종료하거나 표준 guard를
우회하지 않았다. 현재 C++ 변경은 Debug/Release 최소 컴파일 성공 상태이며,
새 EXE 링크·설치와 새 실행 파일의 실제 아레나 진입 시간은 아직 미확인이다.
다음 사용자 실행에서는 기존 `EffectFailure.user.log`의 `Kouku.Loading.Begin`부터
`Kouku.Arena.Ready`까지를 같은 PID/진입별로 비교한다. 화면상 첫 패턴과 이펙트
판정은 사용자가 직접 한다.

## G06. 게시 계산 최적화와 동등성

기존 코드의 함수 profile에서 반복 deepcopy, 전체 문서 직렬화, WModel 선택·경로 확인,
본 pose와 animation root curve 재계산이 큰 비중을 차지했다. Windows PowerShell5.1의
27MB Encounter 읽기는1.500초였고 전체 bootstrap 정렬은5.657초였다. JSON 변환만으로
수분의 지연을 설명할 수 없다. cProfile 총시간은 오버헤드가 있으므로 최종 전후 시간과
섞지 않는다.

`project_kouku_saydon_composition.py`는 선택한 Pattern만 private copy로 만들고
Parent 확장에 같은 private 문서를 사용한다. 내부 memo는 모든 field를 포함한 compact
digest를 사용하고 제품 직렬화 형식은 유지한다. immutable World 문서 digest는
publication마다 한 번 계산하며, root curve는 actor·clip·vertical scale로 재사용한다.

`world_object_collider.py`의 모델·pose 캐시는 같은 publication과 repository root에
한정된다. 모델과 sequence 문서의 강한 참조를 유지하고 pose key는 sequence 객체를
구분하므로 동일 stable ID를 가진 다른 문서의 pose를 혼동하지 않는다.
`verify_dimensionmaster_summon_bind_pose.py`의 vector/quaternion sampling은
timestamp 목록을 매번 생성하지 않고 기존 key 배열에서 동일한 이분 탐색을 한다.
native 입력의 최초 검증과 승격 전후 freshness 검사는 그대로 유지한다.

`Publish-GameplayBalance.ps1`은 worldTrack의 key 배열을 한 번 얻고 숫자 허용 type,
finite·범위 검사와 invariant R 서식을 한 순회로 계산한다. grip 트랙의 불변조건만
첫 key에서 검사하고, 모든 key의 정확한 field·grip 존재·좌표 수·숫자·범위·회전·scale
검사는 계속 수행한다. `Test-GameplayWorldTrackNumbers.ps1`로 실제 함수의 기존
숫자 허용·거부와 출력 서식을 비교한다.

검증은 `out/KoukuPublishPerf20260920/repository` snapshot에서 수행했다. 현재 저장
Composition을 원본 코드와 변경 코드로 각각 정상 생성한 결과를 비교했으며,
G01의 live 파일 크기와 이 정상 재생성 snapshot의 크기를 혼동하지 않는다.
실제 저장소의 기존 미커밋 Encounter와 patternbindings는 초기 SHA256와 같았다.
publisher를 실제 저장소 대상으로 실행하거나 실행 중 draft를 Reload하지 않았다.

최종 동결 코드의 실측은 다음과 같다. 같은 revision1902 snapshot이며 원본 projection은
별도 무프로파일 실행, 원본 Gameplay는 stopwatch 단계 표시만 넣은 성공 실행이다.
변경 후에는 실제 스크립트를 추가 계측 없이 두 단계 연속 실행했다.

| 게시 action | 원본 | 최종 변경 후 |
|---|---:|---:|
| Kouku projection publish |189.179초|66.691초|
| Gameplay balance publish |276.164초|104.497초|
| 두 action 시간 합 |465.343초|171.188초|

두 action 합은63.2% 감소했다. 원본 합은 각각의 성공 실행을 합한 값이며 Workbench
전체 stopwatch가 아니다. map/world publish, owner lock·backup·fingerprint는 이 표에
포함하지 않았다. 기존 실제 receipt의419초대와 새 snapshot 원본465초대를 같은 실행으로
설명하지 않는다. 변경 후 projection은 별도 반복에서도65.928초,67.324초였다.

최종 Encounter27135765bytes, patternbindings3255315bytes,
Gameplay.bootstrap13308273bytes의 SHA256 세 개가 원본 정상 생성 결과와 정확히
일치했다. 데이터 축소나 수치 정밀도 변경으로 시간을 줄이지 않았다.
근거는 `original-projector-unprofiled.json`, `gameplay-before-pass.log`,
`final-sequential.json`, `final-kouku_projection_publish.log`,
`final-gameplay_balance_publish.log`다. 중간 runner의 존재하지 않는 parameter 실패는
`after-final-*`에 별도 보존했으며 성공 시간으로 사용하지 않았다.

현재도 전체 충돌 궤적 계산과 Gameplay의 projector 재검증이 남아 약2분51초가 걸린다.
기존 domain receipt는 실제 모델·일부 catalog 입력 전체를 담고 있지 않으므로 이를
근거로 재검증을 생략하지 않았다. 다음 Workbench Publish는 변경된 디스크 스크립트를
사용하며, 이번 실험이 실행 중 Server나 편집 draft에 반영된 것은 아니다.

## G07. 게시 회귀 검증

집중 회귀32개(ObjectCollider16, WModel selective read11, 입력 변경·rollback5)는
모두 성공했다. 숫자 helper9개 사례도 성공했다. 별도 읽기 검토에서는 Parent 확장과
선택 publication의 기존 결과 동등성, 서로 다른 sequence 객체의 cache 격리,
animation sampling1492개 수치의 정확한 일치, PowerShell 숫자162개와 실제 World
track 검증22개 사례의 허용·거부 및 성공 행 일치를 확인했다. 이는 변경 중 코드 검토이며
frozen commit/SHA 승인으로 기록하지 않는다.

확장해서 실행한 기존 fixture 두 개는 원본 코드에서도 같은 값으로 실패했다.
`test_publish_then_validate_uses_the_same_complete_inventory`는 임시 repository에서
기대 수량 `(3,2,2,1)`과 실제 `(3,1,2,0)`가 달랐다.
`test_albion_takeoff_balances_installed_repeated_landings_without_source_changes`는
기대 높이6.556622863264095와 실제13.678813305200356이 달랐다. 이번 최적화로 고친
것으로 처리하지 않으며 전체 suite 성공으로 보고하지 않는다. 재현 로그는
`focused-inventory[-baseline].log`, `focused-rootmotion[-baseline].log`에 보존했다.

변경 Python4개 AST와 PowerShell3개 parser 검사를 통과했다. 관련 owner transaction
실패 복구1개도 G03과 같이 성공했다. 최종 publisher 도구6개 SHA256는
`out/KoukuPublishPerf20260920/final-tool-hashes.json`에 기록했다.

## G04. Debug Complete Play 준비 완료 후 시작

개별 Pattern/Bundle/Flow의 시작 경로는 `CKoukuSaydonBossTool::Prepare_ServerPlay/Update`에
연결했다. 같은 Gate가 이미 활성화되어 있어도 준비 장벽을 거친다. 모든 선택 루트를 보존하고
게시 Encounter와 patternbindings의 정확한 sourceRevision을 검사해 child/followup/direction clone,
Summon Pattern, Fear/targeted visual, 모든 Effect row와 WORLD group/NEXT를 수집한다.

`CLevel_KakulSaydonArena::Debug_PrepareCompletePlayResources`는 V1 priority worker queue와
실제 prepared 수, catalog revision, failed/unavailable/blocking failure를 확인한다. V2는 기존
Prewarm_Group, actor는 기존 Ensure_Prototypes, WORLD는 기존 Prepare_InstanceResources를
소비한다. 한 update에 V2/actor/WORLD 항목 하나씩 준비하며 V2 snapshot은 같은 generation에서
기존 준비분을 보존한다. 성공 전에는 Play 요청을 제출하지 않는다. Gate 승인 20초와 resource
준비 20분을 분리했고 timeout은 부분 재생을 허용하지 않는다. ID별 준비 실패와 진행 수치를
기존 상태 UI에 표시한다. 취소 시 공유 캐시를 제거하지 않고 미제출 요청만 취소한다.

Sequence + Pattern Flow는 요청 전에 Release 전체 raid collector와 enabled WORLD 준비를
재사용한다. 파티의 Debug 참가자들도 기존 Server PREPARING 상태에서 실제 리소스 준비가 끝난
다음 READY를 회신한다. Server Debug PREPARING deadline만 20분으로 변경했다. Release는 기존
10초를 유지하며 일반 요청 응답 5초와 실제 Stage/Effect/전투 시간은 바꾸지 않았다. READY 전
actor/카메라 재생을 시작하지 않으며 기존 owner STOP, FAILED, 이탈, epoch/revision 검사는 유지한다.

검증:

- 변경 Client 5 TU Debug `/Zs` 및 Release `/Zs` PASS. Release 검사에서 기존 Encore UI의
  Debug 전용 getter 두 호출에 guard가 없던 문제도 해당 두 지점만 고쳤다.
- Server `GameRoom_KoukuRaidFlow.cpp`와 `ServerGameplayContractTests_KoukuRaid.cpp` `/Zs` PASS.
  기존 raid 검증에 10초를 지나도 준비 중에는 cinematic이 시작되지 않는 경우와 최종 deadline
  abort 기대값을 추가했다. 이 Server 실행 테스트는 이 작업자가 실행하지 않았으며 통합 owner가 취합한다.
- 실제 production collector 함수와 실제 DataJson parser를 추출한 격리 콘솔에서 현재 revision1909의
  85 Pattern + 8 Bundle 입력 PASS: Pattern closure85, V1 98, V2 38, WORLD root68.
  이는 Level이 추가하는 WORLD chain/BossCatalog까지 합산한 최종 GPU 수량은 아니다.
- 실제 clone/followup/Bundle 수집, pinned revision 거절, Stage 이후 future Effect 수집,
  누락된 clone presentation 전체 거절, missing Bundle 거절의 7검사 PASS.
- 로그: `out/RaidRegression20260920/complete-play/{compile.log,compile-release.log,server-compile.log,
  collector-all.log,collector-contract.log}`. Collector의 예상 실패 케이스는 FAIL 사유를 출력하되
  테스트 자체는 PASS로 기록한다.

Authoring Data와 설치 Resources는 변경하지 않았다. Client/UI를 실행하지 않았고, 실제 GPU 화면과
파티 동시 시작 확인은 사용자 검증으로 남는다. 이 변경은 준비 지연 때문에 이미 지난 cue가 사라지는
시작 문제를 막는 것이며, 지원하지 않는 원본 emitter나 실행 중 GPU capacity 실패를 복구한 것으로
기록하지 않는다. Product 설치 결과는 통합 RESULT를 따른다.

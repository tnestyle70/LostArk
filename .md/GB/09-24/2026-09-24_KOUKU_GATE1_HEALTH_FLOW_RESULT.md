# 쿠크 1관문 HP 구간 반복 Flow 결과

## G00. 현재 구현 상태

기존 Flow의 optional `entryGroups` 저장·표시·게시·Server 실행 구조에 실제 G1 HP 구간을 적용했다. 최초 구조 검증 당시 revision 2237에는 기존 28 entry와 그룹 0개만 남아 있어 실제 화면과 보스 순서는 선형이었다. 사용자의 후속 적용 요청에 따라 최신 저장본을 revision 2238 / 159 entry / 13 group으로 갱신했다. 같은 문서의 P81/P118 counter 성공 후속과 다른 관문은 보존했다. 디스크 게시 및 실행 중 메모리의 경계는 G07에 기록한다.

## G01. 구현한 계약

- Client Composition은 stable group ID, 표시 이름, 시작·끝 entry ID, optional HP 임계와 `PATTERN_END`/`GROUP_END`를 parse/validate/serialize한다. 잘못된 범위·참조·중복·겹침·legacy 반복 혼용은 저장 전에 거절한다.
- Python projection과 PowerShell publisher는 동일 필드를 검증하고 `RAIDFLOWGROUP`을 기존 Flow step 뒤에 게시한다. Server catalog는 endpoint stable ID를 검증한 entry index로 연결하고 실패 시 기존 catalog를 보존한다.
- Server raid scheduler는 패턴과 동적 counter 후속의 정확한 COMPLETED receipt 뒤에 정수 HP 비율로 다음 entry를 선택한다. 임계 미도달이면 해당 그룹만 반복하고, 도달하면 지정 완료 경계에서 다음 구간으로 이동한다. 이미 넘긴 임계의 아직 시작하지 않은 일반 구간은 건너뛰고 다음 기믹에서 멈춘다. 따라서 여러 임계를 넘긴 공격은 각 기믹을 순서대로 한 번씩 실행한다. 그룹의 첫 entry로 돌아가도 기존 boss HP·epoch·catalog를 유지한다.
- Boss Tool은 Saved Pattern Flow를 접을 수 있는 그룹 목록으로 표시하고 Flow Groups에서 범위·이름·조건을 편집한다. HP 반복 flow의 재생은 Complete Play callback을 통해 Server raid admission으로 전달한다. MainApp callback 연결은 root 담당자가 반영했다.
- Release의 게시된 Pattern/Bundle 재생 준비·취소·완료와 typed raid START/STOP을 공통 경로로 연결했다. 연결·session/world·target·revision·owner/epoch 검증은 유지하며 memory draft와 Mario test override는 Release에서 거절한다. 준비 기한은 Debug/Release 공통 최대 20분이다.

## G02. 실행한 검증

- `Tools/KoukuSaydonPipeline`에서 `python -X utf8 -m unittest test_raid_flow_projection.py`: 13 tests, PASS. HP 그룹의 필드 보존·독립 복사, 0/1000 임계, 누락/역전/겹침/잘못된 임계/전환 경계 거절, 실제 PowerShell row 생성과 정렬을 포함한다.
- 최초 Composition의 `validate_pattern_flows`: PASS. 당시 G1 28개 entry와 그룹 미설정 상태를 확인했다. 실제 HP 구간 반영 후 검증은 G07에 기록한다.
- 담당 C++ 파일: UTF-8 BOM 상태 보존, CRLF 통일 확인. 증거: `out/KoukuHealthFlow20260924/encoding-audit.json`.
- `git diff --check`: PASS.

## G03. 통합 담당자가 실행한 검증

C++ compile/Product build/native 실행은 공유 출력 경합 방지를 위해 root가 단독 수행했다. 2026-09-24 07:48 KST의 최종 Debug/Release Product receipt를 읽어 두 구성의 실제 build PASS와 skippedBuild=false를 확인했다. 증거는 `out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`, `20260923T224850148Z-release-product.json`이다.

기존 `Run_KoukuDraftContracts`에 HP 임계 정확 경계·상회 반복·패턴 완료 경계·그룹 완료 경계·0HP 종료·잘못된 cursor 범위·여러 임계 통과 후 기믹 순서를 검사하는 native fixture를 추가했다. 같은 함수에 실제 `RAIDFLOWGROUP` stable endpoint resolve와 잘못된 참조의 catalog rollback 검사를 추가했다. `Server.exe --kouku-draft-contract-test`로 focused 실행하며, `--contract-test`의 Kouku Product에서도 같은 함수를 호출한다. `--kouku-raid-contract-test`의 준비 기한 fixture도 공통 20분으로 교정했다. 최종 focused 로그에서 Debug 20 PASS / 0 FAIL, Release 12 PASS / 0 FAIL 및 failures:0을 확인했다. 증거는 `out/ReleaseRaidTools20260924/debug-kouku-draft.log`, `release-kouku-draft.log`이다. 같은 디렉터리의 최종 전체 native 로그 `debug-debug-teleport-verified.log`는 8109 PASS, `release-debug-teleport.log`는 7576 PASS이며 둘 다 failures:0이다. 이 결과는 코드·native 계약 검증이며 실제 Client 화면 검증은 아니다.

## G04. 남은 작업과 화면 확인

1. revision 2238의 데이터 저장과 공식 게시를 실행 중 Server나 Client 메모리의 갱신으로 해석하지 않는다. 실행 중 프로세스와 편집 draft는 종료·재실행·Reload하지 않았다.
2. 사용자가 새 데이터와 일치하는 Server catalog 및 Client saved/published revision으로 F1 그룹 표시와 Load Pattern/Complete Play, HP별 기믹 진입, 공굴리기 성공 후속을 확인한다. 에이전트는 Client/UI를 실행하지 않았다.
3. 최초 구조의 Debug/Release Product build와 focused native 검사는 G03의 증거다. 이번 데이터 반영은 C++를 변경하지 않았으며 새 프로세스 실행 없이 schema·projection·공식 게시로 검증한다. UI 후속 변경의 컴파일과 PR 전달은 통합 담당자가 수행한다.

## G05. 통합 중 최종 정적 검사

2026-09-24 07:37~07:38 KST의 변경 파일 스냅샷을 읽기 전용으로 검사했다. 다른 담당자가 진행 중인 native fixture 수정과 Product build·Server 실행·domain publish는 수행하지 않았다.

- 변경 JSON 3개와 Client project/filter XML 2개: 5/5 parse PASS. JSON의 중복 key와 비유한 숫자 token도 거절하는 파서로 확인했다. 증거는 `out/KoukuHealthFlow20260924/final-parse.json`이다.
- Flow focused test 재실행: 13/13 PASS, exit 0. 실제 PowerShell HP 그룹 schema·행 정렬 fixture를 포함한다. 증거는 `final-flow-focused.log`, `final-flow-focused.json`이다.
- 변경 PowerShell 6개: PowerShell AST parser 오류 0. 증거는 `final-powershell-parse.json`이다.
- 변경 C++ 62개: HEAD와 비교한 인코딩·BOM 변경 0, CRLF/LF 혼합 파일 0. Git 저장소의 LF와 Windows checkout의 CRLF는 별도로 기록했다. 파일별 SHA-256과 증거는 `final-cpp-encoding-audit.json`이다. 소스 파일을 재인코딩하거나 개행을 변경하지 않았다.
- `git diff --check HEAD`: exit 0. 증거는 `final-diff-check.log`, 전체 요약은 `final-static-summary.json`이다.
- 이 검사 시점의 Composition Flow validator도 PASS였다. 당시 원본은 revision 2237 / G1 28 entry / entryGroups 0개였으며, 그 결과를 실제 HP 구간 적용으로 해석하지 않는다. 이후 적용은 G07과 구분한다.

## G06. 최초 사용자 검토용 후보

사용자 답변 전 검토할 수 있도록 source revision 2237의 복사본만 `out/KoukuHealthFlow20260924`에 만들었다. 이 후보 준비 단계에서는 실제 `Data` 원본, 생성된 runtime, publisher를 변경하거나 실행하지 않았다. 후보의 revision 2237은 출처 표시이며 실제 저장 revision이 아니다. 후속 적용에서는 G07처럼 최신 원본을 다시 읽고 revision을 증가시켰다.

- 후보 파일은 `g1-health-flow.candidate-composition.json`, `g1-health-flow.candidate-flow.json`이며, 구간별 stable ID와 검증 결과는 `g1-health-flow.candidate-review.md`, `g1-health-flow.candidate-validation.json`에 기록했다. 생성 스크립트는 `prepare_review_candidate.py`다.
- 추천 순서는 160→130 일반 반복, P1, 130→110 일반 반복, P2, 110→85 일반 반복, P6, 85→60 일반 반복, P1, 60→50 일반 반복, P7, 50→30 일반 반복, P2, 30→0 일반 반복이다. 전환 경계는 제안값 `PATTERN_END`다.
- 매 일반 묶음은 P58, P102, P81, P82, P47, P100, P80, P103, P48, P79, P78, P83의 같은 순서다. 기존 첫 20 entry의 P101 추적 8회와 P78 뒤 P104 전환 1회까지 보존하고 마지막에 P83을 추가했다. 기존에 없는 P83 뒤 추적은 새로 만들지 않았다. 기믹마다 기존 P104 전환을 유지했다.
- 기존 28개 entry의 stable ID와 필드는 전부 보존했다. 반복 복사본과 P83에는 충돌하지 않는 deterministic entry ID를 부여했다. 총 `21×7 + 2×6 = 159 entry`, 13 group이며 현 상한 256 entry / 64 group 이내다. Client, Python/PowerShell, Server catalog, Shared flow index 소비자의 상한을 변경할 필요가 없다.
- 후보에 대한 `validate_pattern_flows`와 전체 `validate_document`: 둘 다 PASS. 생성·검증 전후 실제 source SHA-256이 일치하고, 후보의 G1 flow를 원래 것으로 대체하면 나머지 전체 document가 원본과 동일함을 확인했다. 다른 관문·패턴·logic 및 별도 counter 수정은 보존된다.
- `PATTERN_END`로 HP 임계에 도달하면 다음 독립 P101/P104 전에 기믹으로 이동할 수 있다. 현재 패턴 내부의 동적 counter groggy는 COMPLETED까지 기다린다. 이 후보 검증과 이후 실제 데이터 적용은 구분한다.

## G07. 사용자 후속 요청에 따른 실제 HP 구간 적용

2026-09-24 08:30 KST에 최신 source bytes와 revision 2237을 다시 읽고 G1의 `entries`와 `entryGroups`만 병합해 revision 2238로 저장했다. 원본 백업은 `out/KoukuHealthFlow20260924/applied/Composition.revision2237.before.json`이다. validation 후 교체 직전 bytes를 다시 확인하고 같은 디렉터리의 flush된 임시 파일을 원자 교체했다. 다른 세션이나 사용자 변경을 덮지 않으며 생성물을 직접 수정하지 않았다.

7개 일반 반복 구간은 같은 12개 일반 패턴에 기존 추적 entry를 유지한 21 entry씩이다. 130/P1 무력화, 110/P2 진짜 세이튼, 85/P6 댄스, 60/P1 무력화, 50/P7 룰렛, 30/P2 진짜 세이튼의 6개 일회 기믹 그룹을 사이에 배치했다. 기존 28개 entry ID와 모든 필드가 그대로 존재하고, G1 flow 및 revision을 제외한 전체 문서가 원본과 동일한 것을 검사했다. source SHA-256은 `d6249c5d95b2fdf226031799e517ab9ed71699ec461c1ac8fe4ea9a11ee3985f`에서 `31199637f19c54586c9c828e7897b51f801c2b0e4162a2e5d7e1fc6b1213db73`로 바뀌었다.

- `validate_pattern_flows`와 전체 `validate_document`: PASS. 결과는 `applied/apply-validation.json`이다.
- 기존 `python -B -X utf8 -m unittest test_raid_flow_projection.py`: 13 tests PASS. 결과는 `applied/flow-tests.log`다.
- 독립 읽기 검토에서 159 entry / 13 group, 기존 28 entry 보존, 기믹 순서와 `PATTERN_END`를 확인했다. Server는 같은 epoch의 COMPLETED 뒤에만 HP 전환하고, 동적 counter groggy의 member 재생을 마친 후 COMPLETED를 보내므로 조기 전환하지 않는다. 30→0 구간은 살아 있는 동안 반복하며 실제 0HP는 기존 boss death / WAIT_GATE 처리가 우선한다.
- `Prepare_PatternFlow`는 saved source와 published revision을 맞춘다. `Play_PatternFlow`는 HP 반복 그룹이 있으면 기존 flat audition 대신 `m_CompletePlayAdmission`으로 Server raid를 요청한다. 이번 실제 데이터가 이 분기를 활성화한다.

공식 게시 owner `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2238`은 exit 0 / PASS로 완료했다. `koukusaydon.product`, `map.kakulsaydon`, `world.gameplay`, `gameplay.balance` 네 단계 모두 PASS이며 전체 278.875초다. 실행 로그는 `applied/publish.log`에 보존했다.

원본, 생성 Encounter와 patternbindings의 source revision이 모두 2238이다. Encounter의 G1 entries/groups는 원본과 필드까지 같고 Server `Gameplay.bootstrap`에는 G1 step 159행 / group 13행이 있다. 모든 step의 stable ID·target·wait를 원본과 대조했다. G1 gate의 entry count, G1 steps/groups, product revision, bootstrap 총 행 수를 제외한 모든 bootstrap 행은 HEAD와 동일하여 Retail 수치·다른 관문·counter 결과가 보존됐다. bootstrap SHA-256은 `c15859d1a74147f3551783152a6aa10217dbd4e556a9b0bd1aea04d8ee359358`이며 검증은 `applied/published-validation.json`에 기록했다.

변경 데이터와 본 PLAN/RESULT의 `git diff --check`는 PASS다. 이번 데이터 단계는 C++를 변경하지 않았으며 새 Client/Server 실행, 종료, Reload 없이 완료했다. 화면·live room 검증은 수행하지 않았다.

## G08. 공식 게시가 함께 갱신한 World와 Map 출력

공식 Kouku owner의 World 단계는 `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap`도 갱신했다. G07의 행 보존 검사는 `Gameplay.bootstrap`에 한정한다. 이 World 출력에는 기존 main에서 누락된 Retail 몬스터 HP/AP가 실제로 반영되었으므로 사용자 요청의 Retail 적용 범위로 보존한다.

| PROFILE | 기존 HP → Retail HP | 기존 AP → Retail AP |
|---|---:|---:|
| `MONSTER_KOUKU_CLOWN_BOX` | 500 → 587,993 | 1 → 1,843 |
| `MONSTER_KOUKU_CMDGR_03` | 5,000 → 46,439,333 | 35 → 1,152 |
| `MONSTER_KOUKU_CMDUP_02` | 900 → 2,906,365 | 70 → 1,105 |
| `MONSTER_KOUKU_REUP_04` | 900 → 2,906,365 | 30 → 460 |
| `MONSTER_KOUKU_RHKP_06` | 900 → 2,906,365 | 70 → 1,105 |

다섯 행의 기존 값은 HEAD의 base `MonsterProfiles.json`, 새 값은 HEAD의 `Retail.balanceprofile.json`과 모두 정확히 같다. 두 원본과 `Publish-WorldGameplay.ps1`은 이번 작업에서 변경하지 않았다. 해당 bootstrap은 다섯 행의 HP/AP 열 외에는 모든 행과 필드를 보존했다. 로그의 `world.gameplay` 단계가 이 파일의 게시 성공을 기록한다.

이전 통합 게시의 누락 원인은 `Publish-BalanceRuntimeSet.ps1`의 promote 대상이다. 이 스크립트는 World 전체를 staging에 생성하지만 기존 최종 targets에는 네 개 `worldbootstrap`만 있고 Kouku world와 `spawngroupsbootstrap`은 없었다. 따라서 이전 "gameplay + 4 worlds + items" 성공이 모든 몬스터 Retail 배포를 뜻하지 않았다. 이번 Kouku owner의 scoped World publish가 위 다섯 profile을 복구했다. 이후 통합 담당자의 요청으로 공용 publisher의 대상 누락도 보정했으며, 격리 검증과 실제 최종 게시 경계는 [Retail 결과 G06](2026-09-24_RETAIL_BALANCE_RUNTIME_COOLDOWN_RESULT.md)에 기록한다.

`Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.camerashots.json`과 `LV_LUT_MIDNIGHTC_ED.worldsequences.json`도 08:34:27 KST에 공식 Map publisher가 재기록했다. 검사 시 두 파일은 raw bytes, LF 개행, JSON 의미가 HEAD와 모두 같고 `git diff --numstat`가 비어 있다. 이전 checkout의 CRLF bytes가 게시 정본 LF로 바뀌어 status에는 M이 남지만 저장소 내용 차이는 없다. 수동으로 파일을 되돌리거나 개행을 다시 바꾸지 않았다.

실제 내용 변경 목록에는 G07의 원본·Encounter·patternbindings·Gameplay bootstrap 및 PLAN/RESULT 외에 위 Kouku spawn-group bootstrap 한 개를 포함한다. Map 두 출력의 재기록은 게시 이력이며 의미 변경으로 집계하지 않는다.

이후 수정한 공용 Balance publisher의 최종 실제 실행도 PASS했다. 이 단계에서는 Character Select와 Valtan의 spawn-group bootstrap 두 개에 남은 Retail 몬스터 수치도 반영됐다. 따라서 전체 작업의 최종 runtime diff는 Gameplay bootstrap과 Character Select/Kouku/Valtan의 세 spawn-group bootstrap이다. 최종 Retail 대조와 실행 증거는 위 Retail 결과 G06을 따른다. G1 2238 / 159 entry / 13 group과 직전 Gameplay bytes는 그대로 보존됐다.


## G09. F1 HP 그룹 표시와 소비 경로 확인

`Render_SavedPatternFlow`는 저장된 entryGroups에서 HP 기믹 요약을 만들고 반복 7개를 청색 `[Repeat]` 기본 접힘, 일회성 기믹 6개를 금색 `[Once]` 기본 펼침으로 구분한다. 각 임계와 행 수, 0줄의 처치 종료를 표시한다. 기본적으로 기믹과 후속 12행이 펼쳐진다. stable flow/group ID로 트리를 식별하며 헤더는 펼침만 바꾸고 Pattern/Bundle 행만 기존 stable target 선택을 바꾼다. 그룹이 없는 관문은 기존 평면 목록이다.

소스 경로를 독립 확인했다. 보스 생성은 최초 combat 시작에만 일어나고 이후 패턴 전환은 live boss의 HP를 보존한다. 카운터 성공 후 1167+1667+1333ms 후속 무력화 완료 receipt 뒤에만 다음 HP entry를 선택한다. 0HP 사망 통지는 같은 tick의 raid update보다 먼저 clear를 확정하므로 마지막 반복을 재시작하지 않는다. 이 내용은 소스 검토이며 추가 실행 검증을 대신하지 않는다. UI 컴파일·제품 빌드·native 결과는 RELEASE_F1_RAID_TEST RESULT의 최종 통합 기록을 따른다.

# 쿠크 1관문 HP 구간 반복 Flow 결과

## G00. 현재 구현 상태

기존 Flow에 optional `entryGroups` 저장·표시·게시·Server 실행 구조를 연결했다. 실제 G1 일반 패턴 묶음과 전환 경계는 사용자 답변 대기 중이므로 원본 Flow 순서를 변경하지 않았다. 확인 시점 Composition revision은 2237, G1은 기존 28 entry, entryGroups는 0개다. 같은 문서의 P81/P118 counter 성공 후속 변경은 별도 담당자의 수정이며 보존했다.

## G01. 구현한 계약

- Client Composition은 stable group ID, 표시 이름, 시작·끝 entry ID, optional HP 임계와 `PATTERN_END`/`GROUP_END`를 parse/validate/serialize한다. 잘못된 범위·참조·중복·겹침·legacy 반복 혼용은 저장 전에 거절한다.
- Python projection과 PowerShell publisher는 동일 필드를 검증하고 `RAIDFLOWGROUP`을 기존 Flow step 뒤에 게시한다. Server catalog는 endpoint stable ID를 검증한 entry index로 연결하고 실패 시 기존 catalog를 보존한다.
- Server raid scheduler는 패턴과 동적 counter 후속의 정확한 COMPLETED receipt 뒤에 정수 HP 비율로 다음 entry를 선택한다. 임계 미도달이면 해당 그룹만 반복하고, 도달하면 지정 완료 경계에서 다음 구간으로 이동한다. 이미 넘긴 임계의 아직 시작하지 않은 일반 구간은 건너뛰고 다음 기믹에서 멈춘다. 따라서 여러 임계를 넘긴 공격은 각 기믹을 순서대로 한 번씩 실행한다. 그룹의 첫 entry로 돌아가도 기존 boss HP·epoch·catalog를 유지한다.
- Boss Tool은 Saved Pattern Flow를 접을 수 있는 그룹 목록으로 표시하고 Flow Groups에서 범위·이름·조건을 편집한다. HP 반복 flow의 재생은 Complete Play callback을 통해 Server raid admission으로 전달한다. MainApp callback 연결은 root 담당자가 반영했다.
- Release의 게시된 Pattern/Bundle 재생 준비·취소·완료와 typed raid START/STOP을 공통 경로로 연결했다. 연결·session/world·target·revision·owner/epoch 검증은 유지하며 memory draft와 Mario test override는 Release에서 거절한다. 준비 기한은 Debug/Release 공통 최대 20분이다.

## G02. 실행한 검증

- `Tools/KoukuSaydonPipeline`에서 `python -X utf8 -m unittest test_raid_flow_projection.py`: 13 tests, PASS. HP 그룹의 필드 보존·독립 복사, 0/1000 임계, 누락/역전/겹침/잘못된 임계/전환 경계 거절, 실제 PowerShell row 생성과 정렬을 포함한다.
- 현재 Composition의 `validate_pattern_flows`: PASS. 기존 G1 28개 entry와 그룹 미설정 상태를 확인했다.
- 담당 C++ 파일: UTF-8 BOM 상태 보존, CRLF 통일 확인. 증거: `out/KoukuHealthFlow20260924/encoding-audit.json`.
- `git diff --check`: PASS.

## G03. 통합 담당자가 실행한 검증

C++ compile/Product build/native 실행은 공유 출력 경합 방지를 위해 root가 단독 수행했다. 2026-09-24 07:48 KST의 최종 Debug/Release Product receipt를 읽어 두 구성의 실제 build PASS와 skippedBuild=false를 확인했다. 증거는 `out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`, `20260923T224850148Z-release-product.json`이다.

기존 `Run_KoukuDraftContracts`에 HP 임계 정확 경계·상회 반복·패턴 완료 경계·그룹 완료 경계·0HP 종료·잘못된 cursor 범위·여러 임계 통과 후 기믹 순서를 검사하는 native fixture를 추가했다. 같은 함수에 실제 `RAIDFLOWGROUP` stable endpoint resolve와 잘못된 참조의 catalog rollback 검사를 추가했다. `Server.exe --kouku-draft-contract-test`로 focused 실행하며, `--contract-test`의 Kouku Product에서도 같은 함수를 호출한다. `--kouku-raid-contract-test`의 준비 기한 fixture도 공통 20분으로 교정했다. 최종 focused 로그에서 Debug 20 PASS / 0 FAIL, Release 12 PASS / 0 FAIL 및 failures:0을 확인했다. 증거는 `out/ReleaseRaidTools20260924/debug-kouku-draft.log`, `release-kouku-draft.log`이다. 같은 디렉터리의 최종 전체 native 로그 `debug-debug-teleport-verified.log`는 8109 PASS, `release-debug-teleport.log`는 7576 PASS이며 둘 다 failures:0이다. 이 결과는 코드·native 계약 검증이며 실제 Client 화면 검증은 아니다.

## G04. 남은 작업과 화면 확인

1. 사용자가 현재 12개 일반 패턴의 공통 반복 또는 구간별 분할, 주사위 포함 여부, 기믹 전환 경계를 선택한다.
2. 선택한 순서를 최신 Composition의 `patternFlows[GATE1]`에만 병합하고 revision을 증가시킨다. 별도 counter 수정과 다른 관문은 보존한다.
3. 현재 코드의 Debug/Release Product build와 focused native 검사는 완료했다. 실제 HP Flow 순서를 적용한 뒤에는 해당 최신 데이터의 검증·publisher와 PR 전달이 필요하다.
4. 사용자가 Client에서 F1 그룹 표시와 Load Pattern/Complete Play, HP별 기믹 진입, 공굴리기 성공 후속을 확인한다. 에이전트는 Client/UI를 실행하지 않았다.

## G05. 통합 중 최종 정적 검사

2026-09-24 07:37~07:38 KST의 변경 파일 스냅샷을 읽기 전용으로 검사했다. 다른 담당자가 진행 중인 native fixture 수정과 Product build·Server 실행·domain publish는 수행하지 않았다.

- 변경 JSON 3개와 Client project/filter XML 2개: 5/5 parse PASS. JSON의 중복 key와 비유한 숫자 token도 거절하는 파서로 확인했다. 증거는 `out/KoukuHealthFlow20260924/final-parse.json`이다.
- Flow focused test 재실행: 13/13 PASS, exit 0. 실제 PowerShell HP 그룹 schema·행 정렬 fixture를 포함한다. 증거는 `final-flow-focused.log`, `final-flow-focused.json`이다.
- 변경 PowerShell 6개: PowerShell AST parser 오류 0. 증거는 `final-powershell-parse.json`이다.
- 변경 C++ 62개: HEAD와 비교한 인코딩·BOM 변경 0, CRLF/LF 혼합 파일 0. Git 저장소의 LF와 Windows checkout의 CRLF는 별도로 기록했다. 파일별 SHA-256과 증거는 `final-cpp-encoding-audit.json`이다. 소스 파일을 재인코딩하거나 개행을 변경하지 않았다.
- `git diff --check HEAD`: exit 0. 증거는 `final-diff-check.log`, 전체 요약은 `final-static-summary.json`이다.
- 현재 Composition Flow validator도 PASS. 사용자 선택은 여전히 대기 중이며 원본은 revision 2237 / G1 28 entry / entryGroups 0개다. HP 기믹별 일반 패턴 묶음이 실제 데이터에 적용됐다고 기록하지 않는다.

## G06. 사용자 검토용 후보 — 실제 데이터에는 미적용

사용자 답변 전 검토할 수 있도록 source revision 2237의 복사본만 `out/KoukuHealthFlow20260924`에 만들었다. 실제 `Data` 원본, 생성된 runtime, publisher를 변경하거나 실행하지 않았다. 후보의 revision 2237은 출처 표시이며 실제 저장 revision이 아니다. 반영 시 최신 원본을 다시 읽어 사용자 선택과 함께 병합하고 revision을 증가시켜야 한다.

- 후보 파일은 `g1-health-flow.candidate-composition.json`, `g1-health-flow.candidate-flow.json`이며, 구간별 stable ID와 검증 결과는 `g1-health-flow.candidate-review.md`, `g1-health-flow.candidate-validation.json`에 기록했다. 생성 스크립트는 `prepare_review_candidate.py`다.
- 추천 순서는 160→130 일반 반복, P1, 130→110 일반 반복, P2, 110→85 일반 반복, P6, 85→60 일반 반복, P1, 60→50 일반 반복, P7, 50→30 일반 반복, P2, 30→0 일반 반복이다. 전환 경계는 제안값 `PATTERN_END`다.
- 매 일반 묶음은 P58, P102, P81, P82, P47, P100, P80, P103, P48, P79, P78, P83의 같은 순서다. 기존 첫 20 entry의 P101 추적 8회와 P78 뒤 P104 전환 1회까지 보존하고 마지막에 P83을 추가했다. 기존에 없는 P83 뒤 추적은 새로 만들지 않았다. 기믹마다 기존 P104 전환을 유지했다.
- 기존 28개 entry의 stable ID와 필드는 전부 보존했다. 반복 복사본과 P83에는 충돌하지 않는 deterministic entry ID를 부여했다. 총 `21×7 + 2×6 = 159 entry`, 13 group이며 현 상한 256 entry / 64 group 이내다. Client, Python/PowerShell, Server catalog, Shared flow index 소비자의 상한을 변경할 필요가 없다.
- 후보에 대한 `validate_pattern_flows`와 전체 `validate_document`: 둘 다 PASS. 생성·검증 전후 실제 source SHA-256이 일치하고, 후보의 G1 flow를 원래 것으로 대체하면 나머지 전체 document가 원본과 동일함을 확인했다. 다른 관문·패턴·logic 및 별도 counter 수정은 보존된다.
- `PATTERN_END`로 HP 임계에 도달하면 다음 독립 P101/P104 전에 기믹으로 이동할 수 있다. 현재 패턴 내부의 동적 counter groggy는 COMPLETED까지 기다린다. 실제 순서와 경계는 여전히 사용자 선택 대기이며 후보 검증 PASS를 실제 데이터 적용으로 해석하지 않는다.

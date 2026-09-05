# 2026-09-05 쿠크 모델별 패턴·Append·Sequence 편집 결과

## G00. 확인한 원인과 구현

사용자 첨부 화면의 세이튼 `MN_RPCT_05` Action `4219811`은 기존 UI의
`MN_RPCZ_00` 전용 Append 조건 때문에 비활성화됐다. 별도로 Action ID 0을 미선택 또는
RAW clip으로 취급해 실제 기본 동작을 선택·추가하지 못하는 오류도 확인했다.

사용자 지정 브랜치는 `koukysaydon-pattern3`다. 작업은 격리 worktree에서 진행했다.
상위 Workbench의 발탄/쿠크세이튼 구분을 유지하고, 쿠크세이튼 Pattern 모델 선택을
Kouku(RPCZ_00), Saydon(RPCT_05), Large Saydon(RPCT_06)으로 나눴다.
RPCT_07은 Saydon 모델을 공유하며 source profile은 원래 값으로 보존한다.

- Resources Action/slot을 선택하면 실제 모델 카테고리로 이동한다. 같은 모델의 Pattern에
  Append하고 해당 모델에 Pattern이 없으면 하나의 candidate에서 새 Pattern과 Stage를 만든다.
  실패한 Append는 원래 draft와 ordinal을 보존한다.
- 각 Pattern의 `actorProfileId`를 formatVersion 2에 저장한다. v1은 실제 row에서 owner를
  유도하고 빈 기존 Gate1 Pattern은 쿠크로 읽는다. 잘못되거나 서로 다른 모델이 섞인 Pattern은
  오류 항목으로 보존한다. 오류 항목을 정상 카테고리 filter 때문에 숨기지 않는다.
- Action ID 0과 RAW는 `sourceActionId == 0 && sourceStageId == RAW`로 구분한다.
- DRAFT Stage 한도는 reference와 같은 1024개다. 실제 249개 Stage Action도 추가할 수 있다.
  PRODUCT는 기존 쿠크 본체와 64개 Stage 제한을 유지한다. 본체가 다른 Pattern의 Product 승격은 거절한다.
- Sequence의 순차 Animation은 같은 행에 놓고 겹친 구간만 추가 행을 사용한다. Append 후 Fit한다.
  클릭/Ctrl+클릭/빈 공간 사각 드래그로 stable ID를 선택하고 Delete Selected 또는 Delete로 지운다.
  Stage와 그 자식이 함께 선택돼도 한 번씩 삭제하며, 없는 ID가 섞이면 전체 삭제를 거절한다.
- Sequence에 Save/dirty/status를 제공한다. 기존 Save_Atomic의 validate/CAS/atomic replace/reopen을 사용한다.
  저장과 삭제는 새 문서 복사본이나 별도 runtime으로 우회하지 않는다.
- 명시 Kouku publisher의 source revision guard도 v1/v2를 지원하고 stale revision 거절을 유지한다.

현재 canonical JSON은 v2 모델 owner 메타데이터만 추가했으며 기존 Pattern의 시간·배치와 생성된
Product 내용은 바꾸지 않았다. 별도 schema 파일은 없고 실제 native/Python parser를 함께 수정했다.

## G01. 실행한 자동 검증

| 검사 | 실제 결과 |
|---|---|
| 기존 ValtanPatternAuditionServiceHarness Debug 빌드 | 성공, 실제 Workbench/Composition/ActionDocument/ResourceTree CPP 포함 |
| 같은 executable의 `--kouku-composition-editor-contract` | 성공, 종료 코드 0 |
| 기존 projector 및 관련 UI source 검사 | 44개 통과 |
| 변경 Composition JSON 및 harness project/filter XML | parse 성공 |
| publisher entrypoint PowerShell AST | parse 성공 |
| source revision guard 함수만 실행 | 실제 v2/rev2 승인, 잘못된 revision 거절 |
| `git diff --check` | 통과 |

Native 검사는 실제 `Reload`, `Select_ActorProfile`, `Create_Pattern`, `Append_ActionAsStages`,
`Append_ActionToStage`, `Delete_TimelineSelection`, `Save`를 호출했다. 화면의 Action 4219811,
Action 0, 07 alias, 별도 쿠크 Pattern, 빈 모델 카테고리의 원자적 자동 생성, 249 Stage Action,
교차 모델·PRODUCT 거절, 묶음 삭제·저장·재로드, v1 마이그레이션과 손상 row 원문 보존,
외부 파일 변경 시 CAS 실패 보존을 확인했다. 테스트 scratch는 TEMP 아래에 두고 원본 데이터는 보존했다.
ImGui 함수나 Client/Engine 초기화는 호출하지 않았다. 기존 harness에 focused entry를 추가했으며 새 harness는 없다.

최종 로그는 `out/KoukuPattern3/native-editor-final-build.log`와
`out/KoukuPattern3/native-editor-final-run.log`다. 최종 incremental 빌드는 오류 0,
기존 C4828 코드 페이지 경고 198건이었다. 실제 명령은 다음과 같다.

```text
MSBuild.exe Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m:1 /nodeReuse:false
Tools/ValtanPatternAuditionServiceHarness/Bin/Debug/ValtanPatternAuditionServiceHarness.exe --kouku-composition-editor-contract
```

## G02. 대형 미리보기 후속 상태

대형 이름 동작의 요청 배율 100배 및 WP_MN_RPCT_06 오른손 미리보기 부착은 같은 요청의
후속 변경으로 진행 중이다. 원본 Actor 크기 배율은 아직 입증되지 않았으므로 100배를 원본값으로
기록하지 않는다. source socket의 오른손 `b_wp_1`/`wp_1_20`과 무기 자체 skeleton은 확인했다.
최종 미리보기 구현·컴파일 증거와 수동 확인 경계는 해당 변경이 끝난 뒤 이 절에 갱신한다.

## G03. 사용자 화면 확인

실제 Client/UI 입력과 렌더링, 최종 크기·손 위치는 아직 확인하지 않았다. 에이전트는 UI를
실행·조작·캡처하거나 visual PASS로 판정하지 않았다.

새 소스를 받은 뒤 Server/Client를 같은 버전으로 다시 빌드하고, 이 PC의 `Server + Client`
profile을 사용자가 Ctrl+F5로 실행한다. F1 → Action Workbench → KoukuSaydon에서
모델 선택 → Composition Resources의 Action 선택 → Append Action as Stages →
Sequence의 사각 선택/Delete → Save → Reload를 확인한다.
실행 중인 원본 프로그램은 새 소스를 자동으로 반영하지 않는다.

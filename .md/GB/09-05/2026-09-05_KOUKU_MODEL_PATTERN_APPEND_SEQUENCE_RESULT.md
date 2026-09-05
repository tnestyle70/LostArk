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
- Sequencer 상단에 Save/Play/Stop과 dirty/status를 제공한다. 아래에는 Zoom, Full lifetime ms,
  Apply/Fit을, Selected Box에는 Delete/Duplicate를 표시한다. 미선택 상태에서도 버튼을 보인다.
  Play는 현재 draft를 cursor부터 기존 Animation Tool preview로 보내며 끝 위치에서는 처음부터
  재생한다. Stop은 pending 요청을 취소하고 cursor를 0으로 돌린다.
- 전체 lifetime은 Stage clock 합계다. 마지막 Stage의 길이만 바꾸고 기존 box의 끝보다 짧은 값과
  600초 초과를 거절한다. animation 속도나 앞 Stage timing을 자동으로 바꾸지 않는다.
- Duplicate는 선택 Stage와 자식을 함께 복제하고 별도로 선택한 그 자식은 다시 복제하지 않는다.
  단독 Animation 복제는 같은 Stage/time window에 새 stable ID로 넣는다. 실패하면 ordinal도
  보존한다. Append 직후 새 박스의 단일/다중 선택을 함께 설정하며 삭제/Reload 후 없는 선택 ID를 정리한다.
- Save는 기존 Save_Atomic의 validate/CAS/atomic replace/reopen을 사용한다.
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
`Append_ActionToStage`, `Delete_TimelineSelection`, `Duplicate_TimelineSelection`,
`Set_PatternDuration`, `Request_PatternPreview`, `Save`를 호출했다. 화면의 Action 4219811,
Action 0, 07 alias, 별도 쿠크 Pattern, 빈 모델 카테고리의 원자적 자동 생성, 249 Stage Action,
교차 모델·PRODUCT 거절, 묶음 삭제·저장·재로드, v1 마이그레이션과 손상 row 원문 보존,
외부 파일 변경 시 CAS 실패 보존을 확인했다. 테스트 scratch는 TEMP 아래에 두고 원본 데이터는 보존했다.
추가 controls 검사는 lifetime 저장/재로드, 점유 구간보다 짧은 값 거절, Stage+child 선택 중복 제거,
새 ID/모델 owner 유지, 최대 길이 초과 복제의 ordinal rollback, Play cursor/one-shot request를 확인했다.
ImGui 함수나 Client/Engine 초기화는 호출하지 않았다. 기존 harness에 focused entry를 추가했으며 새 harness는 없다.

최종 로그는 `out/KoukuPattern3/native-sequencer-final-build.log`와
`out/KoukuPattern3/native-sequencer-final-run.log`다. 최종 incremental 빌드는 오류 0,
기존 C4828 코드 페이지 경고 66건이었다. 기존 Python 44개도 최신 controls에서 통과했으며
로그는 `out/KoukuPattern3/sequencer-controls-python.log`다. Workbench header 변경의 직접
소비자 MainApp도 다시 최소 컴파일하여 오류 0/기존 C4819 경고 34건을 확인했다.
로그는 `out/KoukuPattern3/client-sequencer-final-mainapp.log`다. 실제 native 명령은 다음과 같다.

```text
MSBuild.exe Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m:1 /nodeReuse:false
Tools/ValtanPatternAuditionServiceHarness/Bin/Debug/ValtanPatternAuditionServiceHarness.exe --kouku-composition-editor-contract
```

## G02. 대형 동작 배율과 오른손 망치

기획 Action의 이름에 `대형` 또는 `large`가 있는 동작은 사용자 요청 100배의 로컬 preview
배율을 적용한다. RAW clip은 해당 clip 이름으로 판단한다. Sequence는 각 occurrence의 원래
Action 이름을 조회하며 일반 동작, 정지, 비활성화 시 기본 크기로 돌아간다. 매번 고정 기준
matrix에서 배율을 계산하므로 반복 재생 때 100배가 누적되지 않는다. 추출된 데이터에서 원본
Unreal Actor 크기 배율은 입증하지 못했으므로 이 값을 원본 Unreal 배율로 기록하지 않는다.

`MN_RPCT_06` preview는 `WP_MN_RPCT_06` animated CModel을 기존 CPart_Body 경로로
생성한다. source socket `wp_1_20`의 오른손 `b_wp_1`과 zero offset/rotation, unit scale을
사용한다. weapon의 별도 5-bone skeleton과 16개 대응 clip을 유지한다. family 3은 body/weapon
원본 길이가 달라 원본 seconds를 유지하고 weapon 마지막 pose를 hold한다. idle/1_01 등 대응
clip이 없는 동작은 저장한 bind pose로 복원한다. 별도 원본 시작 offset을 추측하지 않는다.

Engine의 body update 후 및 도구 seek 직후 손 matrix와 weapon pose를 동기화한다. 부착물에는
부모에 이미 적용된 배율/yaw를 다시 곱하지 않는다. 새 body/weapon 생성 실패 시 staged object를
제거하고 기존 target을 유지한다. 모델 변경과 preview release에서 부착물도 함께 제거한다.

필요한 Resources-relative asset ID는
`Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel`이다. 물리 폴더는
`Client/Bin/Resources/Character/KoukuSaton/WP_MN_RPCT_06`이며 기존 원본 PC에 모델과
인접 texture가 있다. 새 binary를 만들거나 Git에 추가하지 않았다. 다른 PC는 팀 Drive의 같은
폴더가 필요하며 이번 작업에서 Drive 전달 여부를 확인하거나 새 업로드하지 않았다.

Animation_Tool.cpp, CharacterPreviewPanel.cpp, 직접 소비자 MainApp.cpp를 순서대로
Client Debug `ClCompile`/단일 `SelectedFiles`로 확인했다. 모두 오류 0이며 기존 include의
C4819 경고는 각각 18/14/34건이다. 로그는
`out/PR316/compile/client-large-preview-{animation,panel,mainapp}.log`에 있다.
Client link/실행과 망치 모양·손 정렬의 visual PASS는 수행하지 않았다.

## G03. 사용자 화면 확인

실제 Client/UI 입력과 렌더링, 최종 크기·손 위치는 아직 확인하지 않았다. 에이전트는 UI를
실행·조작·캡처하거나 visual PASS로 판정하지 않았다.

새 소스를 받은 뒤 Server/Client를 같은 버전으로 다시 빌드하고, 이 PC의 `Server + Client`
profile을 사용자가 Ctrl+F5로 실행한다. F1 → Action Workbench → KoukuSaydon에서
모델 선택 → Composition Resources의 Action 선택 → Append Action as Stages →
Sequencer의 Play/Stop → Full lifetime ms/Apply → 사각 선택/Delete/Duplicate → Save → Reload를 확인한다.
대형 이름 Action의 100배 크기와 망치 손 정렬은 사용자가 재실행 후 직접 관찰한다.
실행 중인 원본 프로그램은 새 소스를 자동으로 반영하지 않는다.

## G04. 병합 후 Client Product 컴파일 복구

PR #318 병합 기준 `955f7971`의 Client Debug에서
`KoukuSaydonActionWorkbench.cpp:3080`은 C2440, 뒤의 두 그리기 호출은 C2664로 실패했다.
Windows의 `max(a, b)` 매크로가 `ImVec2 max(...)` 지역 변수 선언을 확장한 것이 원인이다.
Assimp include는 `min`만 해제하므로 `max` 오류만 드러났으며, 기존 native harness의
`NOMINMAX` 설정에서는 재현되지 않았다.

`Render_Timeline`의 지역 좌표를 `marqueeMin`/`marqueeMax`로 바꾸고 그리기와 교차 검사
소비부를 연결했다. C++ diff는 6줄 교체이며 좌표식, 선택 상태, public 계약과 데이터는 같다.
기존 UTF-8 BOM 없음과 CRLF를 유지했고 새 파일이나 project/filter 등록은 없다.

실행한 검증:

- 정본 Debug Product 빌드: Engine → Shared → Server → Client compile/link 및 MSBuild 배포 성공, 종료 코드 0.
- 명령: `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`.
- 로그: `out/KoukuCompileFix/debug-product-fixed.log`.
- 결과: `out/BuildPipeline/runs/20260905T064850387Z-debug-product.json`.
- `git diff --check`: 통과. C4819/C4828 코드 페이지와 외부 library PDB 관련 기존 경고는 남는다.
- 변경 JSON/XML 없음. publisher, 광역 하네스, Client/UI 실행과 입력·저장·화면 검증은 수행하지 않았다.

이 수정은 컴파일 복구이며 사각 선택의 최종 화면 판정을 대신하지 않는다. 사용자는
`Server + Client` profile을 Ctrl+F5로 실행한 뒤 F1 → Action Workbench → KoukuSaydon →
Sequencer에서 사각 선택을 확인한다. 기존 미커밋 조사 문서와 SHIELD_STAGGER 구현 계획서는
이 수정 커밋에 포함하지 않는다.

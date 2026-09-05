# 2026-09-05 KoukuSaydon Composition Resources 카테고리 탭과 Logic 카탈로그 결과

> 대응 계획: [Lane/Box Composition Workbench 구현 계획서](2026-09-05_KOUKU_SAYDON_LANE_BOX_COMPOSITION_WORKBENCH_IMPLEMENTATION_PLAN.md) §2.3, G02의 부분 구현
>
> 구현과 최소 자동 검증 완료. 사용자 화면 확인은 미실행.

## G01. 실제 구현

사용자 요청 범위만 구현했다. Stage/Animation 편집 경로와 Product 투영은 바꾸지 않았다.

| 파일 | 변경 |
|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h` | `KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{logicId, displayName, logicType}`, `KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE{occurrenceId, logicId, startMs, durationMs}`, `KOUKU_SAYDON_LOGIC_TYPES = {DURATION, TRIGGER, RESULT}`. 문서에 `iNextLogicOrdinal`, `Logics`; pattern에 `iNextLogicOccurrenceOrdinal`, `LogicOccurrences` |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | root/pattern 속성 검사를 `Has_Properties(required, optional)`로 바꿔 `nextLogicOrdinal`, `logics`, `nextLogicOccurrenceOrdinal`, `logicOccurrences`를 읽기 선택·쓰기 필수로 처리. `formatVersion`은 2 유지. Serialize는 `nextPatternOrdinal` 뒤 `nextLogicOrdinal`, `playAllPatternIds` 뒤 `logics`, `nextAnimationOrdinal` 뒤 `nextLogicOccurrenceOrdinal`, `stages` 뒤 `logicOccurrences`를 쓴다. Validate_Shape는 `kakulsaydon.g1.logic.<n>` ordinal, 이름, type, box의 `<patternId>.logic.<n>` ordinal, 참조 존재, `startMs+durationMs <= Stage clock 합`을 검사하고 PRODUCT pattern의 Logic box를 거부한다 |
| `Client/Public/KoukuSaydonActionWorkbench.h` | `Create_Logic`, `Delete_Logic`, `Append_LogicBox`, `Set_LogicBoxWindow`, `Delete_LogicBox`, `Render_AnimationResources`, `Render_LogicResources`, `Render_LogicBoxDetails`와 세션 상태(선택 탭, 새 Logic type/이름, 선택 Logic, 선택 Logic box, Box Detail 입력) |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | Composition Resources 상단 탭 바 7개(`Animation`, `Logic`, `Effect`, `Collider`, `Scene Profile`, `Sound`, `Camera`). Animation 탭은 기존 `Render_ResourceTree` 본문을 `Render_AnimationResources`로 이름만 바꿔 그대로 사용. Logic 탭은 type별 목록, type 라디오 + 이름 + `Create Logic`, 선택 Logic의 `Append Logic at Cursor`(선택 Pattern의 cursor 위치, 기본 1000ms)와 참조 0개일 때만 켜지는 `Delete Logic`. Sequencer는 Animation row 아래 `Logic` lane 한 줄에 box를 그리고 클릭 선택, 드래그 이동, 양끝 트림, Delete 키 삭제. Box Detail은 Logic box 선택 시 `Logic Box` 단락(이름/type, Start ms, Lifetime ms, `Apply Window`, `Delete Logic Box`)만 표시. `Full lifetime` 최소값에 Logic box 끝을 포함. 나머지 탭은 소비자가 없다는 한 줄 안내 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | `_keys(required, optional)` 도입. root optional `nextLogicOrdinal`/`logics`, pattern optional `nextLogicOccurrenceOrdinal`/`logicOccurrences` 검증. PRODUCT pattern에 Logic box가 있으면 거부. 투영 결과는 변경 없음 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | `test_logic_catalog_is_optional_on_read_and_never_reaches_product` 추가 |

같은 worktree의 다른 세션(branch `codex/kouku-shield-stagger`)이 같은 API 이름으로 `Count_LogicReferences`에 격리 Pattern의
보존 JSON 참조 검사를 더했고(`logicOccurrences`를 읽을 수 없으면 어떤 Logic도 삭제 불가), harness
`BossCompositionDocumentContractTests.cpp`에 `VerifyKoukuLogicControls`를 추가했다. 병합 상태에서 두 TU를 다시 컴파일해 exit 0을 확인했다.

새 C++ 파일, `.vcxproj`/`.filters`, JSON 데이터 파일, Server, Shared는 변경하지 않았다. Logic 정의는 이름과 type만 가지며 판정 값(각도, 요구량)과 성공/실패 분기, Server 소비자는 계획서 G03/G04 범위다.

## G02. 실행한 자동 검증

| 검사 | 결과 |
|---|---|
| `KoukuSaydonCompositionDocument.cpp` Debug x64 `ClCompile` | exit 0, error 0 (`out/PR318/compile/client-KoukuSaydonCompositionDocument.log`) |
| `KoukuSaydonActionWorkbench.cpp` Debug x64 `ClCompile` | exit 0, error 0 (`out/PR318/compile/client-KoukuSaydonActionWorkbench.log`) |
| `Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition` | 28 tests, 새 테스트 통과. 실패 4건(`test_seed…`, `test_rejects_duplicate…`, `test_draft_may_be_empty…`, `test_tracked_products_are_current`)은 변경 전 기준선과 동일하며 다른 세션이 저장한 revision 5 composition과 미publish Product 때문이다 |
| `Tools.KoukuSaydonPipeline.test_kouku_saydon_client_product_level_contract` | 17 tests OK (변경 전과 동일) |
| projector `--mode validate` | 소스 검증 통과 뒤 `projected Product is stale` (변경 전과 동일한 다른 세션의 미publish 상태) |
| `git diff --check` (변경 5파일) | 공백 오류 없음 |

실행한 컴파일 인자:

```text
MSBuild.exe Client/Default/Client.vcxproj -t:ClCompile -p:Configuration=Debug -p:Platform=x64 -p:SelectedFiles=..\Private\<file>.cpp -p:BuildProjectReferences=false -m:1 -nr:false -v:minimal
```

Client 링크와 Product 전체 빌드는 실행하지 않았다. `KoukuSaydonActionWorkbench.cpp`에는 다른 세션의 미커밋 hunk(`Render_Timeline` 11줄)가 함께 있으며 그대로 보존했다.

## G03. 사용자 화면 확인 절차

1. F1 → Action Workbench → KoukuSaydon → Composition Resources 상단 탭 7개 확인. `Animation` 탭이 기존 화면과 같은지 확인.
2. `Logic` 탭 → type `DURATION` 선택 → 이름 `방패 무력화` → `Create Logic` → 목록의 DURATION 아래에 표시.
3. Patterns에서 `KAKULSAYDON_G1_PATTERN_1`(세이튼 무력화 시작) 선택 → ruler를 `att_battle_6_02` 반복 시작(5267 ms)에 두고 `Append Logic at Cursor`.
4. Sequencer `Logic` lane의 box 오른쪽 가장자리를 `6_02` 반복 끝(15935 ms)까지 드래그하거나 Box Detail에서 `Lifetime ms` 10668 → `Apply Window`.
5. Save → Reload → box와 Logic이 그대로인지 확인. JSON에 `logics`, `logicOccurrences`가 쓰였는지 확인.
6. 같은 Pattern을 `PRODUCT`로 바꾸면 "Server consumer" 거부 메시지가 나오고 DRAFT가 유지되는지 확인.

입력·화면 결과의 최종 확인은 이 절차를 사용자가 실행한 뒤 기록한다.


## G04. Codex 추가 검토와 보완

2026-09-05 Claude 구현 diff와 현재 소비 경로를 검토했다. 이번 결과는 Logic의 이름/type/배치/시간 저장이라는 G02 부분 범위에 부합한다. 90도 반사, 요구량, 성공/시간 초과 분기, 전멸 실행, Server 소비자는 구현되지 않았다.

- `KoukuSaydonActionWorkbench.cpp`: 오류로 격리된 Pattern은 typed `LogicOccurrences`가 비워지고 원문만 보존되어, 기존 참조 수 계산으로는 사용하는 Logic 정의를 삭제할 수 있었다. 이제 원문의 `logicOccurrences[].logicId`도 센다. 참조 구조를 해석할 수 없는 경우에는 정의 삭제만 거부하고 다른 Logic 생성과 Save는 허용한다. 참조 없는 다른 Logic의 삭제도 정상적으로 유지한다.
- 같은 파일: Logic 박스만 선택해도 Stage/Animation 전용 Duplicate 버튼이 활성화되던 문제를 수정했다. Logic 박스 복제 기능을 추가한 것은 아니다.
- `project_kouku_saydon_composition.py`: Logic 박스 ID 중복, 다음 ordinal과의 관계, 박스 끝 시각과 Pattern 전체 길이 및 600초 한계, Pattern당 1024개 제한을 C++ 검증과 일치시켰다.
- 기존 Python 테스트에 3개 회귀 검사를 추가했다. 기존 `BossCompositionDocumentContractTests.cpp`의 Kouku editor 경로에는 Logic 생성/Append/창 수정/삭제/Save/Reload와 격리 Pattern의 참조 보존 검사를 추가했다. 새 하네스나 project 등록은 없다.

## G05. 추가 자동 검증

| 검사 | 실제 결과 |
|---|---|
| 최초 현재 소스 `KoukuSaydonActionWorkbench.cpp` Debug x64 ClCompile | exit 0. 보고된 `Logics`/불완전한 형식 오류 재현 안 됨. `out/LogicResourceReview/debug-workbench-compile.log` |
| 보완 전 Debug Product Engine → Shared → Server → Client 컴파일·링크·배포 | PASS, 45.144초. `out/BuildPipeline/runs/20260905T083335662Z-debug-product.json` |
| 보완 후 Client Debug Build | C++ 컴파일 오류 없음. 실행 중인 사용자 Client가 EXE를 점유해 링크는 LNK1104. 종료 통보 후 링크 재실행 예정. `out/LogicResourceReview/debug-client-final.log` |
| 기존 editor contract 빌드 | Debug x64 exit 0. `out/LogicResourceReview/editor-contract-build.log` |
| `ValtanPatternAuditionServiceHarness.exe --kouku-composition-editor-contract` | PASS. Logic 생성·배치·창 조정·Save/Reload, 잘못된 창 거부, 격리된 Pattern의 참조 Logic 삭제 거부, 비참조 Logic 삭제, 불명확한 참조가 있어도 다른 편집·Save 유지, 기존 애니메이션·원문 보존, stale Save 거부 확인. `out/LogicResourceReview/editor-contract-run.log` |
| projector 31 tests + product-level contract 17 tests | 48개 중 44개 통과, 기존 실패 4개(2 failure/2 error). Logic 관련 4개 및 product-level 17개 통과. `out/LogicResourceReview/python-tests.log` |
| 변경 전 기준선 비교 | HEAD projector/27 tests와 보완 후 projector/31 tests를 같은 revision 5 입력에 비교하여 실패 이름·최종 메시지 4개가 일치함을 확인 |
| projector `--mode validate` | 기존 미publish 상태로 `projected Product is stale`. 사용자 Data나 Product를 자동 publish하지 않음. `out/LogicResourceReview/projector-validate.log` |
| `git diff --check` | PASS |

## G06. 사용자 확인과 저장 데이터 보존

- 사용자가 EXE 검증 완료 및 해당 오류가 IntelliSense 오류였다고 확인했다. 에이전트는 Client/UI를 실행하거나 조작하지 않았다.
- 저장 Pattern이 사라진 것처럼 보인 현상은 사용자가 다른 모델을 선택한 것이 원인이라고 확인했다.
- 실제 JSON revision 5에서 `KAKULSAYDON_G1_PATTERN_1`의 `idle 3000 → 6_04 2267 → 6_02 2667 × 4 → 6_03 2200`, 합계 18135ms와 `PATTERN_4`의 무력화 성공 clip 3개, 합계 4167ms를 확인했다.
- 확인 당시 revision 5 원문을 `out/LogicResourceReview/composition-preserved-20260905-174144.json`에 보존했다. 이후 사용자가 저장한 revision 8과 비교해 기존 Pattern ID 누락 0개, `stages` 변경 0개, Logic 정의 1개/박스 1개가 있음을 확인했다.
- 회귀 검사는 시스템 임시 폴더의 Data 복사본에서만 수행했다. 저장소의 실제 composition JSON은 에이전트가 수정·복원·publish하지 않았다.
- 보완된 UI 버튼과 최종 EXE의 화면 결과는 아직 사용자 확인 전이다. branch는 `codex/kouku-shield-stagger`이며 이번 변경의 commit/push/PR/merge는 실행하지 않았다.

## G07. Duration box의 성공/실패 outcome 연결 (Claude, 2026-09-05 추가)

사용자 결정: Result box를 timeline에 따로 놓지 않고, DURATION box의 Box Detail에서 성공/실패 두 줄로 기존 RESULT Logic을 고른다.

| 파일 | 변경 |
|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h` | `KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE`에 `strOnSuccessLogicId`, `strOnTimeoutLogicId` |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | box의 `onSuccessLogicId`/`onTimeoutLogicId`를 읽기 선택·쓰기 필수 문자열로 parse/serialize. Validate: 비어 있거나 RESULT type Logic을 가리켜야 하고, outcome이 있으면 box의 Logic은 DURATION이어야 한다 |
| `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp` | `Set_LogicBoxOutcome(patternId, occurrenceId, success, resultLogicId)`; Box Detail의 DURATION box에 `Outcomes` 단락과 `Success`/`Timeout` 콤보(RESULT Logic만 나열, `(none)`으로 해제); 타임라인 라벨에 `ok>이름 fail>이름` 표시; `Count_LogicReferences`가 typed/보존 JSON 양쪽에서 outcome 참조도 세어 참조된 RESULT 삭제를 막는다 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | box optional key 2개와 같은 규칙 검증 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | `test_logic_box_outcomes_name_result_logics_on_duration_boxes_only` 추가 |

검증: 두 TU Debug x64 ClCompile exit 0(error 0, `out/PR318/compile/client-*-outcomes.log`), projector 32 tests 중 새 테스트 통과(기존 실패 4건 동일), product-level contract 17 OK, `git diff --check` 통과. Client 링크와 화면 확인은 미실행.

사용자 확인 절차: Client 종료 후 Product 빌드 → Workbench에서 `KAKULSAYDON_G1_PATTERN_1`의 `방패무력화` box 클릭 → Box Detail `Outcomes`의 `Success`에 그로기 RESULT, `Timeout`에 전원 전멸 RESULT 선택 → Save → Reload 뒤 JSON box에 두 `logicId`가 남는지 확인. RESULT Logic이 참조되는 동안 `Delete Logic`이 꺼지는지 확인.

사용자가 Client를 종료한 뒤 정본 runner `Invoke-BuildAndRegression.ps1 -Configuration Debug`를 실행해 Engine → Shared → Server → Client 컴파일·링크·배포 PASS를 확인했다
(`out/BuildPipeline/runs/20260905T091455384Z-debug-product.json`, `out/PR318/compile/product-debug-outcomes.log`; `Client.exe` 18:14:55, `Server.exe` 18:14:48). 경고는 기존 PhysX/DirectXTK PDB 누락 LNK4099뿐이다. 화면 확인은 사용자가 수행한다.

## G08. Summon 카탈로그와 Summon lane (Claude, 2026-09-05 추가)

사용자 결정: Summon은 Logic과 같은 방식의 Resources 카테고리이며 type 없이 이름만으로 정의한다. 진짜 세이튼 찾기에서는 Summon box 길이가 패턴 전체 시간이고 수명이 끝나면 소환물이 사라진다.

| 파일 | 변경 |
|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h` | `KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION{summonId, displayName}`, `KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE{occurrenceId, summonId, startMs, durationMs}`; 문서 `iNextSummonOrdinal`/`Summons`, pattern `iNextSummonOccurrenceOrdinal`/`SummonOccurrences` |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | root `nextSummonOrdinal`/`summons`, pattern `nextSummonOccurrenceOrdinal`/`summonOccurrences`를 읽기 선택·쓰기 필수로 parse/serialize. Validate: `kakulsaydon.g1.summon.<n>` ordinal·이름, box `<patternId>.summon.<n>` ordinal·참조·`startMs+durationMs <= Stage clock 합`, PRODUCT pattern의 Summon box 거부 |
| `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp` | Resources 탭에 `Summon`(Logic 다음) 추가, 탭 분기를 이름 기준으로 변경. `Create_Summon`, `Delete_Summon`(typed/보존 JSON 참조 검사), `Append_SummonBox`(기본 `Until Pattern end` 체크로 cursor부터 패턴 끝까지, 해제 시 `Box ms`), `Set_SummonBoxWindow`, `Delete_SummonBox`. Sequencer의 Logic lane 아래 `Summon` lane(초록)과 클릭/드래그/트림/Delete 키. Box Detail `Summon Box`(Spawn ms, Lifetime ms, `Apply Window`, `To Pattern End`, `Delete Summon Box`). `Full lifetime` 최소값에 Summon box 끝 포함. 선택은 Stage/Animation/Logic/Summon 중 하나만 유지 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 같은 규칙의 optional key 검증, PRODUCT Summon box 거부. 투영 결과 변경 없음 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | `test_summon_catalog_and_boxes_follow_the_logic_rules` 추가. 라이브 문서가 이미 Logic outcome을 참조하므로 Logic/Summon 테스트가 픽스처를 만들기 전에 기존 box를 비우는 `without_catalog_boxes` 도입 |

검증: 두 TU Debug x64 ClCompile exit 0(error 0, `out/PR318/compile/client-*-summon.log`), projector 33 tests 중 새 테스트 통과(기존 실패 4건 동일), product-level contract 17 OK, `git diff --check` 통과, projector `--mode validate`는 라이브 revision 17 소스 검증 통과 뒤 기존 미publish Product의 `stale`. Client 링크는 사용자 Client가 실행 중이라 이번에는 실행하지 않았다.

사용자 확인 절차: Client 종료 후 Product 빌드 → Resources `Summon` 탭에서 이름 입력 → `Create Summon` → 진짜 세이튼 찾기 Pattern 선택, cursor 0 → `Until Pattern end` 체크 상태로 `Append Summon at Cursor` → Summon lane의 box가 패턴 끝까지 차는지, Box Detail `Lifetime ms`와 `To Pattern End`가 동작하는지, Save → Reload 뒤 JSON `summons`/`summonOccurrences`가 남는지 확인.

## G09. 쿠크/세이튼 모델 배율 2배 (Claude, 2026-09-05 추가)

사용자 요청: PR 전에 쿠크와 세이튼만 2배 크게. 배율 소유자는 두 곳이며 둘 다 0.01 → 0.02로 바꿨다. 다른 모델과 구조체 기본값은 그대로다.

| 파일 | 변경 |
|---|---|
| `Client/Public/AnimationPreviewAssets.h` | 로컬 preview(Animation Tool·Workbench) 배율. KoukuSaton 4개 항목(`MN_RPCT_00`, `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCZ_00`)의 `fPreviewScale` 0.01f → 0.02f. "대형" 이름 action의 100x preview 배율은 그 위에 곱해지므로 그대로 |
| `Data/Actors/BossCatalog.json` | Server 아레나 본체 배율. `BOSS_KAKULSAYDON_G1_KOUKU`의 `bodyModelPreScale` 0.01 → 0.02 (`CKoukuSaydonPresentationAssetService::Ensure_Prototypes`가 소비) |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` 1364행 | 쿠크 body-only presentation admission이 고정하던 `bodyModelPreScale -ne 0.01`을 0.02로 |
| `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | 쿠크 catalog 행 기대값 0.02 |

Server 판정용 collider radius는 바꾸지 않았다(Debug wire 표시만 있고 damage 권위와 무관). 검증: BossCatalog JSON parse OK, publisher 스크립트 PowerShell parse 오류 0, `test_kouku_saydon_runtime_inputs` + product-level contract 20 tests 중 쿠크 catalog 검사 통과. 실패 1건 `test_project_tuned_world_anchor_preserves_physical_alias`는 `Gameplay.world.json` revision(기대 1788, 현재 1867) 불일치로 이 변경과 무관하다. `git diff --check` 통과.

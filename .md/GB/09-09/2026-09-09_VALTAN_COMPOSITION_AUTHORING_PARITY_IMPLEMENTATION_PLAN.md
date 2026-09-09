# 발탄 Composition 저작 흐름 정리 구현 계획서

작성일: 2026-09-09

상태: 구현 전. 이 변경은 계획서 작성만 포함한다.

기준: `591012db`와 2026-09-09 현재 작업 트리. 쿠크와 이펙트 관련 동시 변경은 보존한다.

이 계획의 목표는 발탄에서 `Composition Patterns → Resources Append → row/box 편집 → Box Detail → local Play → Save → 명시적 Publish → Server Play`를 각각의 책임에 맞게 연결하는 것이다. **Save는 미완성 저작 원본을 안전하게 저장하고, Publish는 실행 가능한 발탄 Product를 만드는 경계로 분리한다.** 기존 발탄 Server 전투와 V1/V2 재생기를 확장하며, 별도 발탄 런타임이나 새로운 manifest 체계는 만들지 않는다.

문서는 개인 계획서 규칙의 구현 계획서 형식을 따른다. G별 파일, 상태, 호출 흐름, 종료 검증을 정하며 H/CPP 전문은 포함하지 않는다. 아래에서 `추가`라고 명시한 함수·필드만 구현 예정 계약이다. 나머지 함수명은 현재 파일에서 확인한 기준점이다.

연결 문서:

- [기존 발탄 V2 binding 구현 계획](C:/Users/user/Desktop/LostArk/.md/GB/09-03/2026-09-03_VALTAN_COMPOSITION_EFFECT_BINDING_IMPLEMENTATION_PLAN.md): V2 저작 연결의 선행 범위. 이 문서는 Save, Draft Pattern, row, Product 소비까지 확장한 별도 범위를 소유한다.
- [발탄 Save pipeline 조사 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-03/2026-09-03_VALTAN_LARGE_DONUT_AND_SAVE_PIPELINE_RESULT.md), [발탄 V2 timeline/Save 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-04/2026-09-04_VALTAN_WARP_PORTAL_V2_TIMELINE_AND_SAVE_PIPELINE_RESULT.md): 과거 상태와 현재 코드의 차이를 확인하는 근거다.
- [쿠크 Composition Workbench 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-05/2026-09-05_KOUKU_SAYDON_ACTION_COMPOSITION_WORKBENCH_AND_BOSS_TOOL_IMPLEMENTATION_RESULT.md), [쿠크 append 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-05/2026-09-05_KOUKU_MODEL_PATTERN_APPEND_SEQUENCE_RESULT.md), [쿠크 pattern/bundle 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md): 사용자 흐름과 기존 저장·readiness 처리의 재사용 기준이다.
- [Effect Composition 계획](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_EFFECT_COMPOSITION_WORKBENCH_AND_PATTERN_CAMERA_IMPLEMENTATION_PLAN.md), [결과](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_EFFECT_COMPOSITION_WORKBENCH_AND_PATTERN_CAMERA_RESULT.md): 공통 Composition shell, Resources와 leaf owner 경계를 재사용한다.
- [Effect Tool 북극성 가이드](C:/Users/user/Desktop/LostArk/.md/GB/08-05/2026-08-05_EFFECT_TOOL_G06_SHADER_REMAINING_GUIDE.md), [렌더링·이펙트 복원 계약](C:/Users/user/Desktop/LostArk/.md/GB/렌더링이펙트복원V2.md), [팀 인터페이스](C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md).

## G00. 현재 저장 경로와 변경 경계 고정

### 목표와 종료 증거

현재 Save가 실패하는 정확한 위치를 보존하면서, 저작 저장에 Product 전체 완성을 요구하는 연결을 제거할 준비를 한다. “70개 검증 때문에 현재 모든 Save가 불가능하다”를 전제로 코드를 바꾸지 않는다.

2026-09-09 읽기 전용 검증에서 `valtan_tuning_pipeline.py validate`는 PASS였다. managed pattern 42개, legacy pattern 25개, combat object 9개, world member 97개, projected artifact 9개였다. 이어 실제 Save가 호출하는 `validate_and_project`, Pattern Sound candidate dependency 검사, V2 candidate binding 검사를 현재 데이터로 실행해 모두 PASS를 확인했다. V2 binding 문서는 formatVersion 2, binding 102개다. 이는 현재 source 검증 결과이며, Workbench 버튼을 누른 Save나 Client 화면·재생을 검증한 결과가 아니다.

현재 Save의 조건부 Product 산출물은 8개다. 기본 Source·descriptor 5개와 합하면 13개 target 후보이며, dirty Sound/V2 owner가 함께 있으면 최대 15개다. 실제 쓰기는 바이트가 바뀐 target만 수행한다. 이 숫자를 “70개 저장 대상”이나 “70개 필수 하네스”로 설명하지 않는다.

### 수정 파일과 현재 함수

| 파일·현재 기준점 | 현재 책임과 이번 변경에서 유지할 것 |
|---|---|
| `Client/Private/ValtanActionWorkbench.cpp:4831` `Save_Reload` | dirty Pattern/Sound/V2 owner를 모아 비동기 Save job을 요청한다. UI 프레임에서 파일을 직접 쓰지 않는 구조를 유지한다. |
| `Client/Private/BalanceTool.cpp:5390` `Begin_ValtanCompositionSave`, `:5413` `Begin_ValtanSaveJob`, `:5518` `Launch_ValtanSaveCommand` | Save job 상태와 staging 파일을 소유한다. `-CommitOnly`도 이미 존재하지만 현재는 source만 쓰는 의미가 아니다. |
| `Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1` | canonical commit 뒤 SourceManifest를 다시 생성한다. 선택적 Publish와 durable commit 완료를 별도 단계로 기록하는 기존 receipt를 유지한다. |
| `Tools/ValtanPipeline/promote_valtan_animation_chains.py:3320` `commit_typed_authoring_patch` | writer lock, baseline/CAS, journal, atomic commit을 소유한다. `:3480` 전체 `validate_and_project`가 V2-only Save에도 실행된다. |
| 같은 파일 `:1796` `validate_and_project`, `:2997` `_atomic_commit_locked` | 전자는 Product 준비로 이동할 검사 묶음이고, 후자는 Source Save에서도 유지할 저장 안전장치다. |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py:7591` `source_manifest` | 14개 Source·관련 문서를 하나의 revision으로 묶어 strict join한다. 이 전체 revision은 editor의 열기·일반 Save 허용 조건에서 분리한다. |
| `Client/Private/ValtanActionWorkbench.cpp:1801` `Reload_Canonical`, `:5107` `Reload_AfterPendingSave`, `:5178` `Update_SaveState` | 전체 Product reopen 실패가 저작 화면과 Save 성공 표시까지 결합되는 부분을 바꾼다. |

`Save_Reload`는 이미 dirty owner별 local 검사와 job receipt를 사용한다. `Animation_Tool.cpp:2021`의 `Validate_ValtanCompositionAnimationGraphMutations`도 변경되지 않은 animation signature는 건너뛴다. 이 개선을 유지하고 writer와 reopen에 남은 전체 결합을 해소한다.

### 상태와 호출 흐름

현재 흐름은 `Save_Reload → Begin_ValtanCompositionSave → Save job → commit_typed_authoring_patch → 전체 projection 및 Sound/V2 join → atomic commit → SourceManifest strict join → Workbench/Product reopen`이다. `Publish after Save`를 끄더라도 projection과 strict join은 앞 단계에 남는다.

현재 `VALTAN_BIND_SLOT`의 동일 `boss.valtan.shout` 세 binding은 서로 다른 `bindingId`와 `CLIP_OCCURRENCE`의 `clip.02/03/04`를 가진다. 예상 stage 위치는 각각 **1400 / 2300 / 3200 ms**다. 그러나 `BuildEffectV2BindingStableId`(`ValtanActionWorkbench.cpp:262`)는 typed `bindingId` 대신 resource/start/anchor 조합을 만들고, `Build_Timeline`(`:4257`)은 convenience `strStage`가 있으면 clock basis를 보지 않고 stage-local 0 ms로 투영한다. 세 상자가 0 ms와 같은 선택 ID로 충돌할 수 있다. 저장 전체 검증과 별도로 고쳐야 하는 저작 모델 결함이다.

### G00 종료 검증

- 현재 Source의 읽기 전용 validate 결과와 V2 세 occurrence 값을 구현 RESULT에 baseline으로 보존한다. 과거 41개 패턴 집계나 2026-09-04의 `missing V2 read-set` 실패를 현재 실패로 재사용하지 않는다.
- Save의 source commit, local reopen, publish, Server 적용 상태를 분리한 뒤 테스트할 실패 지점을 현재 job receipt에 대응시킨다.
- 이 G는 조사·기준 고정이다. Client 실행·화면 PASS는 부여하지 않는다.

## G01. Source 저작 세션과 immutable Product 소비 분리

### 목표와 종료 증거

저작 원본이 미완성 또는 일부 오류 상태여도 정상 패턴을 열고 수정할 수 있게 한다. 동시에 Source Save가 진행 중인 Server Play의 V2 표현을 바꾸지 못하게 한다. **G02의 source-only Save를 사용자에게 활성화하기 전에 이 runtime 분리를 먼저 닫는다.**

### 수정 파일과 H 계약

| 기존 파일 | 변경할 상태와 역할 |
|---|---|
| `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` | `PATTERN_EDIT`, `PATTERN_STAGE_EDIT`, `VALTAN_SOURCE_JOIN_STATUS`에서 원본 존재·파싱 상태와 Product 준비 상태를 분리한다. Source session은 baseline bytes/revision, editable row, row별 오류, dirty owner를 소유한다. |
| `Client/Public/ValtanPatternTree.h`, `Client/Private/ValtanPatternTree.cpp` | 기존 strict `Load_FromAuthoringPaths`/`Load_WhileAdmitted`를 Product 경계에 남긴다. 추가 `Read_AuthoringInventory`는 문서의 row 목록과 오류를 보존하고, 추가 `Build_AuthoringPatternView`는 선택한 정상 row의 편집 view만 만든다. |
| `Client/Public/ValtanActionWorkbench.h`, `Client/Private/ValtanActionWorkbench.cpp` | source inventory/selection과 마지막 admitted Product view를 별도 멤버로 소유한다. Source error를 Product admission enum 하나로 덮지 않는다. |
| `Client/Public/EffectV2_Document.h`, `Client/Private/EffectV2_Document.cpp`, `Client/Public/EffectV2_Catalog.h`, `Client/Private/EffectV2_Catalog.cpp` | strict runtime parser는 유지한다. 추가 `Parse_BindingsForAuthoring`는 유효 JSON의 각 binding row와 오류를 보존한다. authoring snapshot과 admitted runtime snapshot을 목적이 드러나는 호출로 구분한다. |
| `Client/Public/EffectV2_Runtime.h`, `Client/Private/EffectV2_Runtime.cpp` | 기존 evaluator와 local-preview snapshot overload를 유지한다. live BOSS_VALTAN binding cache가 authoring catalog revision을 따라가던 `Ensure_Bindings`를 admitted Product snapshot 소비로 바꾼다. |
| `Client/Public/ValtanPresentationGenerationAdmission.h`, `Client/Private/ValtanPresentationGenerationAdmission.cpp` | 이미 있는 generation artifact receipt를 통해 Product V2 binding·leaf/group snapshot을 획득한다. Server gameplay revision과 exact network PREPARE 검사는 유지한다. |

Source row에는 stable ID, parsed value, 오류 이유, 보존용 JSON, 수정 여부를 둔다. 문서 전체 syntax 오류와 개별 row의 의미 오류를 구분한다. duplicate ID는 해당 row들을 오류 항목으로 보존하고 정상 row처럼 lookup하지 않는다. 표시용 임시 key는 저장 ID로 쓰지 않는다. `CBalanceTool`의 현재 runtime형 필드에 값 0이나 빈 action을 채워 “완성된 pattern”을 만들지 않는다.

`DATA_JSON_VALUE`는 원본 byte span을 제공하지 않는다. 따라서 UI의 보존 JSON은 오류 표시와 세션 유지에 사용하고, 디스크에서 수정하지 않은 row의 byte 보존은 G02의 Python writer가 baseline/current 원문으로 수행한다. `KoukuSaydonCompositionDocument.cpp`의 `Parse_Text`/`strLoadError`/`strPreservedJson` 방식은 오류 항목 격리 원리로 재사용하며 byte-exact 보존을 이미 보장하는 것으로 오인하지 않는다.

### 함수와 실제 소비 흐름

`Reload_Canonical`을 Source 로드와 Product 로드의 두 결과를 받는 흐름으로 바꾼다. Source 로드 성공이면 inventory와 편집 가능한 row를 stage하고 한 번에 교체한다. Product 로드 실패면 마지막 admitted Product view와 이유를 유지한다. 이 실패가 Source inventory를 비우거나 정상 row의 Save를 막지 않는다. Source 문서 전체가 파싱 불가능하면 해당 owner의 기존 세션과 bytes를 유지하고 그 owner만 쓰기 금지한다.

`Tools/GameplayPipeline/valtan_presentation_generation.py`의 `build_presentation_generation`과 `Tools/ValtanPipeline/valtan_tuning_pipeline.py`의 `_stage_presentation_generation_closure`는 이미 V2 bindings 및 그 leaf/group 문서를 generation artifact로 묶는다. live runtime은 이 **기존 generation의 bytes**에서 만들어진 immutable snapshot을 pin한다. `EffectV2_Runtime.cpp:171`의 `Ensure_Bindings`가 `Get_BossValtanSnapshot()`의 최신 authoring 상태를 자동 반영하지 않게 한다. local preview는 현재 draft로 만든 명시적 snapshot을 기존 `Notify_Stage`/local preview overload에 전달한다. snapshot 선택만 분리하고 renderer·clock evaluator를 복제하지 않는다.

Pattern Sound는 기존 presentation generation의 M lane에 강제로 넣지 않는다. 현재 독립 S receipt와 playback admission을 유지한다. 이미 저장된 Product와 새 source의 sound/effect를 암묵적으로 섞지 않는다.

### G01 종료 검증

- valid JSON 안의 잘못된 pattern/binding 한 row가 있어도 다른 정상 row는 inventory에 남고 선택·편집된다. 오류 row는 이유와 원문을 보존한다.
- source 전체 parse 실패 때 이전 session/preview가 유지된다. 해당 owner가 실패해도 다른 문서를 조용히 지우지 않는다.
- Product generation A 재생 중 authoring V2를 B로 stage/Save해도 A의 binding/leaf bytes가 바뀌지 않는다. local preview에만 명시적으로 B를 넘기면 B를 소비한다.
- 수정한 `BalanceTool`, `ValtanPatternTree`, `ValtanActionWorkbench`, V2와 admission H/CPP만 최소 컴파일한다. 기존 V2 binding/Valtan audition harness에 snapshot 고정·실패 보존 사례를 추가하여 해당 사례만 실행한다.

## G02. Source-only Save와 저장 완료 상태 닫기

### 목표와 종료 증거

패턴, V1 cue, V2 binding, row layout, Sound의 dirty source를 안전하게 저장한다. Save의 성공은 디스크 Source commit과 Source reopen으로 판정하고, local Preview 준비와 Product Publish 결과는 별도로 표시한다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.h`의 post-save 상태와 `BalanceTool.h`의 job result에 `sourceCommitted`, `sourceReopened`, `previewReady`, `publishAttempted`, `publishSucceeded`의 의미를 분리한다. 기존 job/receipt를 확장하며 새 영속 manifest를 만들지 않는다. source commit 후 Preview가 준비되지 않아도 dirty 표시를 정확히 해제하고 `저장됨 / 미리보기 준비 안 됨: 해당 이유`를 표현한다. 실패한 save를 성공으로 표시하지 않는다.

`Tools/ValtanPipeline/promote_valtan_animation_chains.py`에 **추가 `commit_source_authoring_patch`**를 둔다. 기존 `commit_typed_authoring_patch`의 writer lock, read-set staging, `_atomic_commit`과 journal 코드를 재사용하고 아래 Source 저장 경계만 분리한다. `valtan_tuning_pipeline.py`의 기존 `commit-canonical` dispatch와 Save job에는 명시적 source 저장 mode를 연결한다. 옵션 이름만 `CommitOnly`로 바꾸고 내부 projection을 남기는 구현은 종료로 인정하지 않는다.

### 저장 schema와 parse 경계

| Source 문서 | 저장 가능 조건 | Save에서 요구하지 않는 조건 |
|---|---|---|
| `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json` | 유효 JSON, 지원하는 source version, 변경 row의 stable ID/field type/수치 범위, paired pattern/stage identity, source syntax 보존 | 전체 42개 pattern의 runtime 완결성, 모든 animation native window, 모든 dependency의 Product join |
| `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | binding row의 typed 구조, ID 유일성, path 안전성, 변경 필드의 단위/범위 | 선택하지 않은 resource의 물리 준비, 모든 binding과 현재 Product stage의 exact join |
| 기존 Pattern Sound owner 문서 | 기존 row ID와 source 구조 및 변경 수치 범위 | 전체 발탄 Product와의 join 및 unrelated Sound/Effect 완성 |

실제 V2 owner 경로는 현재 catalog가 resolve하는 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`을 사용한다. 입력 파일 경로를 UI나 새 workbench 상수에서 추측하지 않는다.

미완성 Source를 저장하려면 strict runtime parser를 느슨하게 만드는 것만으로 끝낼 수 없다. split source formatVersion 2를 도입하고 gameplay pattern의 `authoringStatus`를 `DRAFT | PRODUCT`로 저장한다. `DRAFT`는 최소 `patternId`, `displayName`, `authoringStatus`, `stages`를 가진다. runtime에 필요한 entry/action/eligibility/hit 등이 아직 없으면 **field 부재**로 보존한다. 무피해 action이나 가짜 정상 값으로 채우지 않는다. `PRODUCT` 표시는 저자의 publish 의도를 나타내며 준비 완료는 publisher가 계산한다. 기존 formatVersion 1 row는 메모리에서 기존 Product 의도를 유지하고, 명시적 첫 source Save 때 version 2로 transactionally 이행한다. 열기만으로 파일을 바꾸지 않는다.

presentation의 paired row는 같은 `patternId`, `stages`를 가진다. 비어 있는 stage/animation/effect 목록은 Draft 저장에서 허용한다. source projection 전에 runtime이 요구하는 완전한 구조를 별도로 만든다. 기존 version 1 strict Product loader와 generated Product format은 그대로 유지하고, publisher의 source adapter가 version 2의 준비된 부분만 현행 strict projection에 전달한다.

unknown field는 원본 row에서 보존한다. 수정한 row도 원본 object에 typed field patch를 적용해 만든 뒤 직렬화하며, 편집 view가 아는 field만으로 row 전체를 다시 만들지 않는다. 지원하지 않는 field를 정상 동작하는 편집 필드로 보여 주지는 않는다. 누락 resource나 미해결 reference는 row별 준비 안 됨 사유다. ID 또는 version을 판별할 수 없는 row는 수정하지 않고 보존하며, 명시적 복구·삭제 명령만 허용한다. 새 ID는 정상 항목과 격리된 오류 항목 모두에 충돌하지 않아야 한다. 이미 존재하던 duplicate ID 오류를 정상화하지 않은 채 보존하는 것은 허용하지만, mutation으로 duplicate를 추가하는 것은 거부한다.

### writer 호출 흐름과 CAS

`Save_Reload → Begin_ValtanCompositionSave → Save job source mode → commit_source_authoring_patch → owner baseline 비교 → 변경 row storage 검사 → source bytes stage → _atomic_commit → source reopen receipt → Accept_PendingSaveOwners`로 연결한다.

- CAS 단위는 **이번 Save에서 쓰는 기존 owner 파일**이다. 현재 전체 14개 manifest hash를 editor 저장 허용 조건으로 사용하지 않는다. 같은 owner 파일에 외부 변경이 있으면 그 파일 이름과 충돌 이유를 표시하고 기존 dirty draft를 보존한다. 다른 owner 문서의 변경은 해당 Save를 막지 않는다. 자동 3-way merge framework는 이번 범위에 추가하지 않는다.
- Pattern 변경은 gameplay/presentation paired source를 같은 transaction에 넣는다. V2-only Save는 V2 owner만, Sound-only Save는 Sound owner만 쓴다. row metadata는 presentation source에 함께 저장한다. 여러 owner를 한 번에 저장할 때는 기존 다중 파일 atomic commit과 rollback을 유지한다.
- 기존 `_array_span`, `replace_or_append_rows`, `replace_append_or_remove_rows`, `_assert_unmanaged_raw_rows_preserved`를 재사용해 수정하지 않은 오류 row와 unknown field의 원문을 보존한다. paired source 전체를 runtime view로 재직렬화해 유효하지 않은 row를 탈락시키지 않는다.
- Source 저장은 `validate_and_project`, 전체 native audit, provenance receipt 재생성, global Pattern Sound/V2 Product join, strict `source_manifest`를 호출하지 않는다. 기존 effect resource read-set은 resource 내용 교체를 실제로 소비하는 준비·Publish 경계에서 유지하며, source reference 문자열 저장 때문에 전체 Resources 상태를 요구하지 않는다.
- `Data/Encounters/Valtan/*`, generated animation/V1 cue, rootmotion, balance provenance receipt, `Valtan.bosscomposition.json`은 Source Save target에서 빠진다. 마지막 파일은 현재 SHADOW descriptor이므로 별도 정본으로 승격하지 않고 Publish 때 갱신한다.
- commit 후 Source reopen 실패는 디스크 저장 여부를 receipt 그대로 표시하고 draft를 복구 가능하게 남긴다. 반복 Save로 이미 commit한 source를 덮어써 문제를 숨기지 않는다.

### G02 종료 검증

- 미완성 Draft, 누락 resource를 참조하는 typed row, 정상 row 옆의 오류 row가 저장 후 reopen에서 동일한 상태로 남는다. 수정하지 않은 오류 row의 bytes가 보존된다.
- V2-only/Sound-only/row-only Save의 실제 write set이 해당 source owner로 제한된다. generated Product, descriptor, receipt와 Server revision의 bytes가 변하지 않는다.
- 같은 owner 외부 수정은 CAS 실패하고 disk/current draft가 보존된다. unrelated owner 수정은 Save를 막지 않는다. multi-owner 두 번째 rename 실패는 전체 rollback된다.
- 기존 `test_action_composition_atomic_save_contract.py`, `test_action_composition_dirty_owner_save_contract.py`, `test_valtan_canonical_typed_patch_transaction.py`에 이 경계의 사례를 보강하여 해당 모듈을 실행한다. 단순 소스 문자열 기대값만 바꿔 통과시키지 않는다.
- C++ job/receipt 소비 변경을 최소 컴파일하고 `git diff --check`를 실행한다. 일반 Save 실행에 전체 publisher나 70개 suite를 선행하지 않는다.

## G03. bindingId와 typed clock으로 모든 V2 box 연결

### 목표와 종료 증거

Append한 V2 box의 선택, 이동, 세부 수정, Duplicate/Delete, Save/Reopen, local preview와 Product 재생이 같은 `bindingId`와 clock을 사용하게 한다. `VALTAN_BIND_SLOT` 세 shout box의 정확한 위치와 독립 편집이 이 G의 회귀 기준이다.

### 수정 파일과 H 계약

`EffectV2_Document.h`의 `EFFECT_V2_BINDING::strBindingId`, `eClockBasis`, `strClipOccurrenceId`, `iStartMs`, `eRepeatPolicy`, `strAnchorSlotId`, `eFollowPolicy`, `eRotationBasis`, `LocalTransform`, `eStopPolicy`가 저작 계약이다. convenience `strStage/strClip/strBone` 등은 호환 소비용이며 저장 key나 clock 판정에 쓰지 않는다.

`ValtanActionWorkbench.h`의 `TIMELINE_ITEM`에 V2 원본 `bindingId`와 표시 occurrence identity를 구분한다. EACH_LOOP 표시 box가 여러 개여도 underlying owner는 한 binding이다. 선택 state와 ImGui ID에는 `bindingId + displayed occurrence`를 쓰고, mutation은 typed `bindingId` 하나로 보낸다. 복제만 새 bindingId를 발급한다.

`EffectV2_Catalog.h`의 기존 `EFFECT_V2_STAGE_BINDING_KEY`와 `Stage_AppendBossValtanStageBinding`, `Stage_RemoveBossValtanStageBinding`, `Stage_DuplicateBossValtanStageBinding`, `Stage_UpdateBossValtanStageBindingStart`를 확장한다. **추가 `Stage_ReplaceBossValtanBinding`**은 old bindingId를 대상으로 완전한 typed replacement를 검증하고 한 번에 stage한다. Box Detail에서 anchor/transform/clock/stop을 바꾸는 모든 입력이 이 경계를 호출한다. `UPDATE_START`만 구현한 상태에서 나머지 UI field를 저장 가능하게 보이지 않는다.

### clock 투영과 편집 흐름

`BuildEffectV2BindingStableId`의 합성 key 소비를 제거하고, `Resolve`/`Build_Timeline`/selection/delete/duplicate/detail/preview가 typed binding ID로 찾게 한다.

- STAGE: `stage offset + clock.startMs`가 pattern상의 시작이다.
- CLIP_OCCURRENCE: pattern/stage/action과 정확한 occurrence ID를 resolve하고, 기존 `Resolve_ClipSourceToStageMs`의 source window·playRate 변환으로 `clock.startMs`를 stage 위치에 투영한다. sourceStart 이전·window 바깥·missing occurrence를 0으로 clamp해 정상 box로 만들지 않는다. 오류 표시와 원래 typed 값을 유지한다.
- drag 입력은 화면의 stage ms를 현재 basis의 ms로 역변환한 candidate를 stage한다. basis 변경은 기존 화면상 시작을 유지할 수 있는 정확한 변환에 성공할 때만 commit한다. occurrence 재배치·playRate 변경 후 binding의 occurrence ID와 원본 clock 값은 유지되고 화면 위치만 다시 계산한다.
- STAGE는 현재 계약대로 occurrence null/ONCE만 허용한다. EACH_LOOP는 명시된 occurrence의 반복을 evaluator와 같은 식으로 표시한다. occurrence 삭제는 참조하는 V1/V2/Sound 항목을 보여 주고 기존 typed cascade 명령으로 함께 제거하거나 삭제를 거부한다. 무관한 occurrence로 자동 재연결하지 않는다.

현재 `Resolve_ClipSourceToStageMs`는 missing occurrence에서 0을 반환하고 sourceStart 이전을 clamp한다. 이 함수를 성공 여부와 오류를 반환하는 변환으로 바꾸고 V1/V2 호출자가 실패를 소비하게 한다. runtime의 `Resolve_StageSpawnClock`(`EffectV2_Runtime.cpp:799` 부근)이 사용하는 식은 `occurrence wall start + loop epoch × loop wall duration + (binding source start − occurrence source start) / playRate`다. Python `effect_v2_binding_pipeline.py:1318` 부근의 raw startMs→playMs/stageDuration 직접 비교도 바꾼다. source window `[sourceStart, sourceStart + sourceDuration)`와 투영된 wall time을 각각 검사하여 sourceStart가 0이 아닌 clip과 playRate를 UI·publisher·runtime이 동일하게 취급하게 한다.

끝점은 실제 런타임 의미를 가져야 한다. 현재 V2 `EXPLICIT`은 finite duration field가 없으므로 임의 resize를 그대로 활성화하지 않는다. 이 G에서 **BOSS_VALTAN binding formatVersion 3**를 추가하고 `clock.durationMs`를 nullable 양의 정수로 정의한다. `EXPLICIT`이면 양수 필수이며 같은 clock basis에서 `startMs + durationMs`가 끝이다. 다른 stop policy이면 null이다. version 1/2 읽기는 유지하고 version 2의 기존 EXPLICIT 의미를 임의로 유한 길이로 바꾸지 않는다. version 2 EXPLICIT row는 migration 시 명시 길이가 없다는 사유를 표시하며 저자가 NATURAL/경계 종료 또는 유한 길이를 선택한다. 현재 102개 source binding의 stopPolicy는 NATURAL 100개, STAGE_END 2개이고 EXPLICIT은 0개다. 현재 데이터는 기존 종료 의미를 유지한 채 version 3로 이행할 수 있다.

`EffectV2_Document.cpp`의 Parse/Serialize/Equals, `EffectV2_Catalog.cpp` staging, `EffectV2_Runtime.cpp` pending/group lifetime, Python V2 binding parser/publisher를 같은 G에서 갱신한다. group child의 종료는 자신의 유한 끝과 binding 끝 중 빠른 쪽이며 자연 수명·stage/occurrence 종료·Stop/Seek에서도 같은 규칙을 쓴다. V1 resize는 기존 `cue_end`/source duration 계약을 사용한다. 시각적 box 폭만 바뀌고 runtime은 자연 종료하는 상태를 허용하지 않는다.

### G03 종료 검증

- `VALTAN_BIND_SLOT` shout 3개의 box가 1400/2300/3200 ms이고 bindingId가 모두 다르다. 가운데 box만 이동/수정/삭제해도 다른 둘은 바뀌지 않는다. Save/Reopen에서 ID와 typed clock이 유지된다.
- STAGE, CLIP_OCCURRENCE, sourceStart가 0이 아닌 clip, playRate 변경, EACH_LOOP, missing occurrence, 중복 ID를 각각 검증한다.
- `durationMs`의 Parse→Serialize→Parse, local seek/loop/stop, group 종료와 Product snapshot 소비가 일치한다. 지원 version 외 값·0 duration·overflow는 오류를 보존하며 거부한다.
- 기존 `test_action_composition_effect_v2_clip_projection_contract.py`, `test_action_composition_sequence_identity_contract.py`, `Tools/EffectToolV2/test_effect_v2_binding_pipeline.py` 및 해당 V2 runtime harness 사례를 실행한다. Python 모듈의 실제 파일 경로는 기존 test module을 사용하며 새로운 광역 harness를 만들지 않는다.

## G04. Composition Resources append와 Box Detail의 typed owner 연결

### 목표와 종료 증거

쿠크의 resource 선택→candidate→commit 흐름을 발탄의 기존 owner에 연결한다. 리소스 목록은 준비되지 않은 항목까지 사유와 함께 유지하고, 사용할 수 있는 리소스는 선택한 row/stage에 append할 수 있게 한다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.cpp`의 `Render_ResourcesPane` 영역(`:11900` 부근), `Can_AppendCompositionAnimationResource`, `Append_CompositionAnimationResource`, `Apply_CompositionResourceAppend`, detail 렌더링과 `PENDING_RESOURCE_APPEND`를 확장한다. `ICompositionWorkbenchSession`과 공통 shell의 frame 종료 후 deferred command 처리를 재사용한다.

resource identity는 `ANIMATION_CLIP`, `ANIMATION_SEQUENCE`, `V1_EFFECT`, `V2_EFFECT`, `V2_GROUP`, 기존 Sound/Camera 등 **현재 owner별 typed 종류와 stable resource ID**다. label, filename prefix, pointer, vector index로 종류를 역추론하지 않는다. `IsBossValtanEffectV2Resource`의 `boss.valtan.` prefix만으로 append 지원 여부를 판단하지 않고 catalog의 typed resource 및 선택 대상의 capability로 결정한다. 목록 탐색 때문에 모든 leaf resource를 미리 로드하지 않으며 `Read_Inventory → 선택 resource Load_ResourceSnapshot`을 사용한다.

| 입력 종류 | 실제 append owner와 저장 | Box Detail와 runtime 소비 |
|---|---|---|
| Animation clip/sequence | `CBalanceTool`의 stage/animation slot draft. 새 occurrence ID를 발급하고 순서를 저장한다. | source window, playRate, repeat 및 stage time을 기존 animation graph/local preview에 반영한다. |
| V1 effect | 기존 `Add_ValtanStageEffectCue`로 cue ID와 exact occurrence를 가진 `Valtan.presentation.json` product cue draft를 만든다. | cue timing/transform/follow/stop은 기존 cue 편집 경계에 연결한다. Publish가 patterneffectcues를 만들고 기존 V1 playback이 소비한다. leaf 자체 수정은 Effect Tool owner가 저장한다. |
| V2 effect/group | 기존 V2 Catalog의 typed binding append/replacement. 선택 row의 stage/action/clock을 명시한다. | G03 typed clock/anchor/detail과 같은 immutable V2 snapshot을 local preview·Product runtime이 각각 소비한다. leaf/group 문서 자체 수정은 Effect Tool V2 owner가 저장한다. |
| Sound/Camera | 기존 sound/camera owner의 typed 항목 append 명령을 호출한다. | 각 기존 소비자의 단위와 receipt를 유지한다. Effect용 timing 구조로 강제 변환하지 않는다. |

V1은 현재 source occurrence 기반 계약을 사용하므로 선택 row에 유효 occurrence가 없으면 정확한 사유를 표시한다. 새로운 임의 stage clock을 V1에 조용히 추가하지 않는다. V2 STAGE binding은 animation occurrence 없이 만들 수 있다. append가 지원되지 않는 대상 조합은 행 선택만으로 성공한 것처럼 보이지 않고, 필요한 stage/occurrence를 선택하도록 상태를 표시한다.

### 함수와 commit 흐름

`Resources 선택 → 선택 resource의 저장된 snapshot 확보 → 대상 pattern/row/stage 및 typed capability 확인 → candidate 작성 → owner stage → frame 종료 시 draft commit → Build_Timeline → 선택한 새 box 표시 → 기존 local preview snapshot 갱신` 순서다. 실패하면 기존 document와 preview 및 selection을 유지한다.

Box Detail의 Apply는 session input buffer를 owner mutation으로 바꾸는 순간이다. 작업 중 수치 입력을 원본에 매 프레임 쓰지 않는다. V1/V2 Deep Link는 정확한 저장 resource ID와 source binding/cue ID를 Effect Tool에 전달한다. 현재 Product에 없다는 이유만으로 Draft의 resource 편집 진입을 차단하지 않는다. Workbench Save는 invocation과 배치만 저장하며 leaf 내부 변경을 몰래 함께 저장하지 않는다.

### G04 종료 검증

- Animation clip/sequence, V1 leaf, V2 leaf/group의 append→detail Apply→Duplicate/Delete→Save/Reopen 경로를 각각 확인한다. append 실패 때 문서/선택/preview가 유지된다.
- 저장되지 않은 leaf edit와 저장된 resource snapshot의 차이를 구분한다. source leaf 저장 후 명시적 refresh가 성공하면 새 preview snapshot을 만들며 실패 시 이전 것을 유지한다.
- 기존 `test_action_composition_effect_invocation_contract.py`, `test_action_composition_resource_categories.py`와 수정한 owner의 focused round-trip 검증을 실행한다.
- 사용자 확인 경로는 `F1 → Action Composition Workbench → Valtan → Composition Resources → 대상 row 선택 → Append → Box Detail → Apply → local Play → Save → Reopen`이다. 에이전트는 Client를 대신 실행·조작하지 않는다.

## G05. 지속되는 row와 빈 Draft Pattern 만들기

### 목표와 종료 증거

Composition Patterns에서 원본 animation intake의 Product 승격 transaction을 거치지 않고 빈 Draft Pattern을 만들 수 있게 한다. 사용자가 추가한 빈 row와 box의 row 배치가 Save/Reopen 후 유지된다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.h/.cpp`는 row 선택·Create input buffer·deferred command를 소유하고, `BalanceTool.h/.cpp`는 source draft mutation을 소유한다. 기존 `TIMELINE_ITEM::iSubrow`는 충돌 없는 표시용 packing 값이다. 이를 저장 ID로 사용하지 않고 presentation pattern에 아래 metadata를 추가한다.

| 추가 source field | 책임과 유효 조건 |
|---|---|
| gameplay root `nextPatternOrdinal` | 문서의 단조 증가 pattern ID 발급 counter. 새 ID는 `VALTAN_AUTHORED_000001` 형식으로 전체 source와 retired ID에 충돌하지 않는 다음 값을 원자적으로 사용한다. 삭제된 ID를 재사용하지 않는다. |
| presentation pattern `nextRowOrdinal` | 해당 pattern의 row ID 발급 counter. 삭제나 정렬로 감소하지 않는다. |
| presentation pattern `rows[]` | `rowId`, `lane`, `displayName`, `order`를 가진 명시적 저작 row. 빈 row도 보존한다. row ID는 pattern 범위에서 안정적이다. |
| presentation pattern `rowAssignments[]` | `rowId`, `ownerKind`, `ownerId`로 기존 stage/occurrence/cue/binding/sound/camera 항목의 row를 지정한다. gameplay clock과 resource 내용은 소유하지 않는다. |

metadata는 별도 JSON, `.bosscomposition` descriptor, leaf 문서에 복제하지 않는다. 기존 formatVersion 1은 메모리에서 deterministic 기본 row를 만들고 첫 명시 Save에만 metadata를 기록한다. 원래 row가 없는 기존 파일을 여는 동작은 source dirty를 만들지 않는다. 관리하지 않는 오류 항목의 row assignment도 함께 보존한다.

새 패턴의 최소 paired source는 G02의 Draft schema를 사용한다. **추가 `Create_ValtanDraftPattern`**, `Add_ValtanCompositionRow`, `Move_ValtanCompositionItemToRow`, `Remove_ValtanCompositionRow`는 `CBalanceTool`의 source candidate를 검증·교체하는 mutation이다. 필수 입력은 displayName이며 stable ID는 owner가 발급한다. 생성 직후 stages가 비어 있어도 Save가 된다.

기존 `CAnimation_Tool::Stage_ValtanCompositionIntakeSequence`와 Python `create_pattern_from_request`/`prepare_create_pattern_transaction`은 source reference intake·승격 용도로 남긴다. Composition의 `New Draft Pattern`은 이 전체 promotion pipeline을 호출하지 않는다. Draft에 sequence를 append할 때는 G04의 기존 animation append를 사용하고 출처 reference는 알려진 값만 기록한다.

### row와 stage의 호출 흐름

`New Draft Pattern → paired source candidate → local commit → pattern 선택 → Add Stage/Append Sequence → Add Row → Append/Move box → Source Save`다. 빈 Pattern의 local Play는 “재생 가능한 stage 없음”으로 실패하지만 저장은 된다.

row는 배치 layout이며 stage 순서·duration·action branch를 바꾸지 않는다. 같은 lane의 다른 row로 이동은 metadata만 바꾼다. 다른 stage로 transfer할 때는 기존 animation/effect/sound dependency cascade와 typed clock 변환이 성공해야 한다. row 삭제는 비어 있을 때 즉시 candidate에서 제거하고, 내용이 있으면 항목 이동 또는 명시적 함께 삭제 명령을 실행한다. 숨겨진 항목을 유실시키지 않는다.

새 stage는 owner가 `stageId`와 `actionId`를 안정적으로 발급하고 명시적으로 입력한 양의 duration을 저장한다. animation/hit가 없어도 Draft로 유지한다. category/eligibility/damage/entry 및 graph edge가 준비되지 않았다고 다른 정상 패턴이나 전체 Save를 막지 않는다. 새 pattern을 normal rotation 또는 scripted sequence에 자동 가입시키지 않는다.

### G05 종료 검증

- 빈 Draft 생성→Save→Reopen에서 동일 pattern ID와 빈 stages를 확인한다. 이후 sequence/V1/V2 append와 재저장이 이어진다.
- 빈 row 추가, 이름 변경, 순서 변경, item 이동이 round-trip에서 유지된다. row-only Save는 animation/effect/gameplay timing을 변경하지 않는다.
- duplicate/delete/undo 성격의 세션 되돌리기에서 ID를 재활용하지 않는다. dangling assignment는 오류 사유를 남기고 다른 row를 지우지 않는다.
- 기존 `test_action_composition_manual_stage_topology_contract.py`, sequence identity와 atomic save 테스트에 신규 Draft/row 사례를 추가한다. Product-ready fixture만 만들어 빈 Draft가 저장되는 핵심 경계를 우회하지 않는다.

## G06. local Play와 Product Publish의 준비 검사 연결

### 목표와 종료 증거

저장한 Source의 준비된 부분은 local preview로 즉시 확인하고, 발탄 전투에 적용할 때만 기존 strict Product·Server revision 검사를 통과하게 한다. publish 실패는 기존 해당 Product 단위를 통째로 보존한다.

### 수정 파일과 현재 함수

`ValtanActionWorkbench.cpp`의 `Play_EffectivePreview`, `Seek_EffectivePreview`, `Refresh_PatternLocalPreviewAfterMutation`, V2 preview refresh와 `Animation_Tool.cpp`의 `Play_ValtanCompositionDraftPattern`/`Seek_ValtanCompositionPattern`는 선택한 source draft로 immutable preview를 만드는 경계다. build 실패 시 마지막 성공 preview를 유지하며 “현재 draft가 재생됨”으로 오인되지 않도록 source/preview revision과 실패 이유를 표시한다.

`Tools/ValtanPipeline/valtan_tuning_pipeline.py`, `promote_valtan_animation_chains.py`, `Tools/GameplayPipeline/valtan_presentation_generation.py`가 Publish 준비와 기존 candidate/receipt를 소유한다. `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:1551`의 `prepare_publication`이 이미 사용하는 inventory, dependency closure, per-pattern unavailable reason, 최종 union 검증 원리를 가져온다. 쿠크와 발탄의 source parser를 억지로 합치거나 공용 publisher framework를 새로 만들지 않는다.

### dependency closure와 Product 단위

발탄의 closure에는 선택 pattern의 stage/action graph, referenced pattern, follow-up/branch, scripted sequence, normal selection entry, world event set, combat object, animation/rootmotion, V1/V2 resource, Sound/Camera의 실제 참조를 포함한다. 기존 parser가 판별하는 edge를 사용하고 문자열 prefix나 “현재 화면에 보이는 box”만으로 closure를 만들지 않는다.

**초기 구현의 발탄 raid Publish 단위는 기존 encounter 전체다.** 현재 scripted sequence와 cross-pattern 참조가 엮인 기존 Product를 pattern별 old/new overlay로 섞어 쓰지 않는다. 선택한 준비된 새 독립 Draft를 local preview로 확인할 수 있어도 Server raid에 추가할 때는 저자가 연결한 encounter closure 전체를 다시 검증하고 기존 candidate transaction으로 배포한다. Product에 연결되지 않은 새 Draft는 raid Publish를 막지 않는다. 연결된 항목이 준비되지 않으면 candidate 준비를 실패시키고 마지막 encounter Product, generation receipt, Server gameplay revision을 그대로 보존한다.

Source adapter는 `DRAFT | PRODUCT` 의도와 계산된 준비 상태를 구분한다. Product 의도가 있어도 필수 field/reference가 빠지면 unavailable 사유를 내며, reference target이 DRAFT인 경우에도 closure 내에서 필요한 준비 조건을 모두 검사한다. “READY”를 수동 flag로 저장해 strict 검사를 우회하지 않는다. 부분 성공 publication, 별도 manifest/admission registry, Resource hash pack을 이번 작업의 완료 조건으로 추가하지 않는다.

### 실제 호출 흐름과 Server 권위

`명시적 Publish → 최신 Source snapshot → 기존 edge 기반 encounter closure → strict validate_and_project 및 native/provenance/Sound/V2 dependency 검사 → 기존 candidate staging → generation artifacts/receipt 준비 → 전체 target CAS/atomic publish → 기존 Server 적용 절차`다.

Server gameplay revision과 `CValtanPresentationGenerationReadAdmission::Acquire_ExactReceipt`의 network PREPARE 일치는 유지한다. 제품 transform/action/phase/damage는 계속 Server가 결정한다. source 저장·local Play는 Client `CValtan` local AI를 제품 전투로 승격하지 않는다. protocol에 새 임의 local play action을 추가하지 않는다. 기존 command sink와 server audition 경로가 지원하는 pattern만 Server Play에 노출한다.

기존 Publish 결과가 재시작을 요구하면 `Published / Server restart required`를 유지하고 실제 Server 재시작 전 적용 완료를 표시하지 않는다. Source와 Product의 전체 exact join은 editor gate에서 제거하지만, Server가 승인한 gameplay와 Client presentation generation의 일치는 그대로 검사한다. “Source 저장됨”, “Preview 준비됨”, “Product 배포됨”, “Server 적용됨”을 하나의 성공 표시로 합치지 않는다.

### G06 종료 검증

- 선택 pattern의 누락 resource는 그 preview와 연결된 Product closure에 이유를 표시한다. unrelated Draft는 정상 패턴의 편집·Save·local preview를 막지 않는다.
- scripted sequence 중간 dependency가 실패하면 이전 encounter/generated documents/generation receipt가 모두 그대로 유지된다. old Product와 new Source 조합을 새 revision 전체 적용으로 보고하지 않는다.
- 준비된 current source는 기존 publisher validate/candidate 경로를 통과하고 V1 cue/V2 binding·finite duration/animation·Sound·Camera를 실제 consumer가 읽는다. Source metadata와 미연결 Draft는 generated runtime 문서로 새지 않는다.
- 기존 candidate atomicity, presentation generation admission, Server audition 관련 focused 검사만 실행한다. Server 관련 데이터 계약을 바꾸면 해당 Server loader/command 소비까지 컴파일·검증한다. Server revision 검사를 약화해 테스트를 통과시키지 않는다.

## G07. 기능별 검증과 사용자 재생 인계

### 구현 순서와 변경 단위

G01→G02를 먼저 닫아 source 저장을 정상화한다. G03은 stable identity/typed clock을 고정하고, G04/G05는 그 위에서 resource append/detail와 row/Draft 패턴을 연결한다. G06은 최종 Product 적용을 닫는다. 각 G에서 실제 사용하는 기존 파일만 수정하고 신규 C++ 파일은 계획하지 않는다. 현재 `.vcxproj/.filters`에 등록된 owner/runtime 파일을 재사용하므로 새 등록은 없다. 구현 중 파일 분리가 실제로 필요해지면 그 G 계획에 파일의 소비자·등록·검증을 먼저 반영하고 빈 placeholder를 만들지 않는다.

각 commit은 해당 G의 코드, source grammar/consumer, 필요한 focused test, PLAN/RESULT를 같은 기능 단위로 묶는다. 대규모 dirty worktree의 다른 Effect/쿠크/캐릭터 문서는 자동 stage·commit하지 않는다. `Client/Bin/Resources`에는 이 작업의 자동 변경이나 Git payload를 만들지 않는다.

### 자동 검증 명령과 기대 결과

다음은 구현 때 실행할 명령이다. 이 계획서를 작성한 사실을 실행 증거로 기록하지 않는다.

```powershell
python Tools/ValtanPipeline/valtan_tuning_pipeline.py --repository-root . validate
```

위 명령은 G00 baseline 및 G06 Product 준비 검증에 사용한다. G02의 Draft source Save 허용 조건으로 자동 실행하지 않는다. Draft source 도입 뒤에는 publisher의 준비된 encounter adapter를 통해 같은 Product 검증 계약을 유지한다.

```powershell
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_atomic_save_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_dirty_owner_save_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_valtan_canonical_typed_patch_transaction.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_effect_v2_clip_projection_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_sequence_identity_contract.py
python -m unittest discover -s Tools/EffectToolV2 -p test_effect_v2_binding_pipeline.py
```

각 명령은 연결된 G에서만 실행한다. 다른 G의 미구현 기대값이나 무관한 전체 suite를 Save·컴파일의 필수 gate로 붙이지 않는다. fixture는 임시 디렉터리에서 writer를 실행하며 실제 Source·Products를 테스트 때문에 수정하지 않는다.

MSBuild는 설치된 Visual Studio의 `MSBuild.exe`를 resolve해 사용한다. 변경 CPP를 명시한 `/t:ClCompile /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m:1 /nr:false /v:minimal`로 최소 컴파일하고, 최종 변경 단위에서 `Client/Default/Client.vcxproj` Debug x64 Build로 link를 확인한다. Server consumer를 바꾼 G에서만 Server 프로젝트도 빌드한다. 변경 JSON은 실제 source/runtime parser로, XML을 바꿨을 때만 project/filter XML parse로 검증한다. 마지막으로 `git diff --check`를 실행한다.

### 사용자 입력·저장·재생 확인

| 확인 항목 | 사용자가 직접 수행할 입력 | 종료 관찰 |
|---|---|---|
| 기본 Source Save | 기존 Valtan pattern의 box 하나 수정→Save→Reopen | 선택/ID/수치가 유지되고 Product Publish 요구 없이 저장 완료가 표시된다. |
| V2 clock | `VALTAN_BIND_SLOT` shout 세 box 확인, 가운데만 이동/Detail Apply | 세 시작이 1400/2300/3200 ms에서 독립적으로 동작하고 해당 binding만 바뀐다. |
| V1/V2 append | Resources에서 V1 leaf/V2 leaf/group를 차례로 append | 각 box의 Detail·Play·Save가 같은 invocation을 소비한다. |
| row/Draft | 빈 Draft 생성→빈 row 추가→Save/Reopen→animation/effect append | 미완성 단계부터 저장되고 row·ID가 지속된다. |
| local Play | Play/Seek/Loop/Stop, typed clock·finite end 변경 | 이전 effect 잔류·중복 실행 없이 현재 preview revision을 소비한다. |
| 실패 보존 | 누락 resource 또는 잘못된 row가 있는 패턴과 정상 패턴을 함께 열기 | 오류 항목과 정상 항목이 모두 남고 정상 Source Save가 된다. |
| Server Product | 명시 Publish 성공 후 요구된 Server 적용 절차→기존 Valtan Server Play | gameplay/presentation revision이 맞고 기존 Server 권위 경로에서 실행된다. 실패하면 이전 Product가 유지된다. |

현재 LAN 스크립트는 이 PC를 `server-host`로 판정했고 `Server + Client` profile을 준비했다. 조사 당시 endpoint는 not-listening이었다. 실제 실행 확인 시에는 그 시점의 Server CMD·Client 상태를 다시 보고하고 사용자가 `Ctrl+F5`로 실행한다. 에이전트는 Client/UI를 자율 실행·조작·캡처하지 않는다. visual fidelity와 실제 아레나 재생 PASS는 사용자의 서면 관찰 이후에만 RESULT에 기록한다.

### 현재 문서 작성의 완료 상태

| 구분 | 2026-09-09 상태 |
|---|---|
| 구현 계획서 | 이 문서로 작성. G01~G07 구현은 아직 하지 않았다. |
| 현재 Source 구조 검증 | 읽기 전용 validate와 실제 Save 하위 projection/Sound/V2 candidate 검사 PASS. |
| 신규 schema·source-only Save·typed V2 clock·row/Draft | 계획이며 미구현이다. |
| Client 최소 컴파일·최종 link | 이 문서 변경에서는 실행하지 않았다. |
| Workbench 실제 입력·Save/Reopen·local/Server Play | 이번 조사에서 실행하지 않았다. |
| Client visual fidelity | 사용자 확인 전이다. |

구현 후에는 해당 RESULT에 실행한 검증과 미확인 화면 경계를 기록하고, 실제 public 계약이 바뀐 부분만 `CLAUDE.md`와 팀 사용서에 반영한다. 현재 계획서 작성 단계에서 공유 문서를 먼저 바꿔 구현 완료처럼 만들지 않는다.

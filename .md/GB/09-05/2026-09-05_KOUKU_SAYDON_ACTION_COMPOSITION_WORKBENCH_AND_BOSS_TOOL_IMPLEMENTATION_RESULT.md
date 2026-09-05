# 2026-09-05 KoukuSaydon Sequencer·Boss Tool 최소 수직 슬라이스 구현 결과

> 현재 상태는 아래 **8절 G06/G07 공용 Workbench·전체 Animation 연결**을 따른다. 1~7절은 이전 시점의 기록이다. 이번 C++ 변경 뒤 빌드와 사용자 화면 확인은 아직 실행하지 않았다.


> 문서 종류: 구현 결과서
>
> 상태: 공용 창·Boss session·전체 Animation 연결 코드 반영 / 빌드·사용자 화면 확인 대기
>
> 기준 브랜치: `codex/kouku-workbench-iteration`
>
> 대응 계획: `2026-09-05_KOUKU_SAYDON_ACTION_COMPOSITION_WORKBENCH_AND_BOSS_TOOL_IMPLEMENTATION_PLAN.md`

## G04 검토 당시 기록 — 최신 상태는 8절

### 검증 의무 제거와 공용화 재조사

- AGENTS.md에서 Core/FullDiagnostic 단계 안내와 일괄 하네스 추가·실행 의무를 제거했다. 광역 진단 미실행 자체를 완료·커밋 차단 사유로 삼지 않는다.
- CLAUDE.md와 매 세션 읽는 gotchas.md의 최소 Core, presentation/Server 변경 시 FullDiagnostic 필수, 고정 회귀 명령 묶음도 제거했다. 사용자 실제 아레나 확인과 변경한 기능의 필요한 compile/link를 기준으로 한다.
- 이번 문서 수정에서는 C++를 수정하거나 빌드·진단을 다시 실행하지 않았다. 문서 diff와 잔존 강제 문구만 확인했다.
- 기존 Sequencer의 Boss 선택은 Kouku에서 조기 반환하고, Valtan Workbench의 Reload/Save/Play는 PatternTree·Balance·Sound/Effect admission에 직접 연결되어 있음을 확인했다. Boss 콤보만 연결하면 이 병목이 다시 따라오므로 화면과 보스별 저장·실행 session을 분리해야 한다.
- 기존 발탄에도 모든 family의 Play Family가 완성된 것은 아니다. Animation/Effect transport는 있지만 Sound의 로컬 timeline 재생, Profile/Light/Camera의 일반 row authoring/transport, 임의 Summon 추가·재생은 별도 연결이 필요하다. 상세 재사용 경계는 대응 PLAN의 공용화 절에 기록했다.
- 공용 Sequencer 구현 자체는 아직 미완료다. 문서 규칙 제거와 실제 런타임 의존성 제거를 같은 완료 상태로 보고하지 않는다.

기준 작업 브랜치는 `codex/kouku-workbench-iteration`이다. 큰 기존 dirty worktree를 보존했으며 commit/push하지 않았다.
기존 `CValtanActionWorkbench`의 Sequencer/Resources 코드는 삭제되지 않았다. 쿠크의 별도 최소 편집기에
기존 기능 전체를 연결하지 않은 것이 사용자에게 기능이 없어진 것처럼 보인 원인이다.

### 이번에 소스에 반영한 범위

- 기본 Product 빌드와 VS Client/Server 빌드에서 자동 publisher와 광역 진단 선행조건을 제거했다. 명시 Core/FullDiagnostic 및 데이터 publisher는 남아 있다.
- K Composition 목록과 Save는 action reference 전체 검사·revision join·PRODUCT timing admission을 요구하지 않는다. 손상 패턴은 오류 행과 원문 JSON을 보존하고 다른 패턴의 편집·저장을 계속한다. JSON 전체 구문/정본 헤더 오류에서는 이전 로드 상태를 유지한다.
- 실제 `1관문` 부모 TreeNode, 별도 Composition Resources 창, 실제 `MN_RPCZ_00` 전체 clip 목록·검색·클릭 preview·Append as Stage·Add Animation Row를 연결했다. Action/Stage 참고 트리도 별도로 유지한다.
- Animation occurrence별 24px 행, box 이동·trim·Stage 길이 drag와 Animation Play Family/Play Row를 연결했다. preview는 offset·sourceStart·play rate·loop·빈 구간을 소비하고 잘못된 clip은 해당 행에서 건너뛴다.
- K Boss Tool의 개별 PRODUCT 오류와 presentation binding 오류를 나머지 목록/모델 로드에서 격리했다. 실제 Server 재생의 revision·entity 권한 검사는 유지했다.
- projector는 PRODUCT만 선택하고 malformed DRAFT나 action reference 때문에 배포를 막지 않는다. RAW clip의 추출 Action ID는 강제로 만들어 넣지 않는다. 잘못된 PRODUCT 자체는 명시 publish 시 오류로 남고 이전 생성물을 보존한다.

### 아직 구현하지 않은 범위

- 기존 Valtan의 별도 Sequencer 창과 모든 family 편집을 공용화해 쿠크에도 제공하는 작업.
- Valtan 내부 FULL_JOIN, Sound/Shake admission 등 남은 전체 편집 차단의 제거. 이번 반영을 프로젝트의 모든 병목 제거 완료로 해석하지 않는다.
- Sound·Effect V1/V2·Logic·Collider·Camera 등의 쿠크 편집·저장·재생, playhead/transport 기능 동등성.
- Resource를 실제 Server 쿠크 본체에 바로 재생하는 raw audition. 현재 preview는 로컬 collision-off actor다.
- Server PRODUCT의 loop·다중 Animation·trim/offset timing 확장. 현재 Server v1 제한은 유지한다.

### 이번 검증

- Debug Product: Engine/Shared/Server/Client 컴파일·링크·배포 성공. `out/BuildPipeline/runs/20260905T031416151Z-debug-product.json`.
- K projector focused 검사 19개 PASS. RAW metadata 없는 projection, malformed DRAFT 격리, invalid PRODUCT 실패 후 기존 출력 보존 포함.
- 첫 빌드는 모든 제품 링크 뒤 결과 JSON writer의 module export 누락으로 종료 코드 1이었다. export를 고쳐 재실행한 위 결과는 정상 종료했다.
- FullDiagnostic, Core, oracle 및 Server 실행 harness를 이번에는 돌리지 않았다. 아래 기존 P0-3 PASS는 당시 증거이며 이번 변경의 실행 검증으로 재사용하지 않는다.
- Client/UI를 실행하거나 화면을 판정하지 않았다. 사용자 편집·저장·재로드·시각 확인은 미검증이다.

사용자 확인 경로는 `Server + Client` profile을 `Ctrl+F5`로 실행 → F1 → Level Navigation의
KoukuSaydon → KoukuSaydon Action Workbench다. Resources에서 전체 clip 목록과 Animation 편집을 확인한다.
이 확인으로 공용 Sequencer 완성을 대신하지 않는다.

---

아래는 최초 P0-3 당시의 기록이다.

## 1. 닫힌 범위

### 1.1 K 전용 저작 정본과 패턴 목록

- 직접 저작 정본은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` 하나다.
- `CKoukuSaydonActionWorkbench`는 이 문서의 K 패턴만 나열하며 Valtan PatternTree, Workbench, Boss Tool을
  include하거나 호출하지 않는다. 현재 표시는 `1관문` 실제 부모 node가 있는 중첩 tree가 아니라 다섯 K Pattern의
  평면 목록이며 display name에 `1관문 /`이 들어간다.
- 1관문 시작 패턴 다섯 개를 만들었다.
  - `KAKULSAYDON_G1_SHIELD_STAGGER`: 빈 DRAFT
  - `KAKULSAYDON_G1_FIND_TRUE_KOUKU`: 빈 DRAFT
  - `KAKULSAYDON_G1_DANCE_TIME`: 빈 DRAFT
  - `KAKULSAYDON_G1_PIZZA`: 6-Stage PRODUCT
  - `KAKULSAYDON_G1_CARD_MATCH`: 빈 DRAFT
- Workbench에는 DRAFT와 PRODUCT가 모두 보이지만 projector는 PRODUCT만 generated encounter와 Server
  bootstrap에 투영한다. 따라서 현재 K Boss Tool과 Server 목록에는 Pizza 한 개만 보인다.

### 1.2 Resource preview와 animation authoring

- K Resource 트리는 exact boss profile `MN_RPCZ_00`의 action/stage/animation clip만 보여 준다.
- leaf 선택은 K arena에서 제품 boss를 건드리지 않고 `Layer_AnimationPreview`의 별도 collision-off preview
  body를 재생한다. 따라서 이것은 raw resource를 실제 Server boss에 즉시 재생하는 기능이 아니다.
- `Append as Stage`는 선택 clip을 새 Stage 전체 구간의 ACTIVE animation occurrence로 복사한다.
- Pattern 생성/삭제/이름/상태, Stage 추가/삭제/순서/길이, animation box 이동/trim/복제/삭제/Stage 이동을
  stable ID candidate edit로 지원한다.
- timeline lane은 24px이며 fit/zoom/maximize와 최대 600초 저작 범위를 지원한다.

### 1.3 저장과 Product projection

- Save는 `parse -> validate -> stage -> CAS -> atomic replace -> reopen` 순서를 사용한다.
- exclusive crash-recoverable writer lock으로 동시 저장을 거부하고 실패 시 기존 source를 보존한다.
- `Publish All PRODUCT`는 K source revision을 고정한 뒤 K Product 두 문서와 Gameplay bootstrap을 갱신한다.
- generated Product는 다음 두 파일이다.
  - `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`
  - `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`
- 두 generated 문서 commit 도중 실패하면 둘 다 이전 상태로 rollback하며 rollback 자체가 실패한 backup은
  삭제하지 않고 경로를 보고한다.

### 1.4 Server 재생과 `[Live]`

- `CKoukuSaydonBossTool`은 generated K encounter의 PRODUCT 패턴만 읽는다.
- `Play Selected`와 `Play All`은 K 전용 scoped request로 Server `CKoukuSaydonBrain`에 전달된다.
- protocol은 local generated source revision을 요청에 싣고 Server catalog의 revision과 exact match해야 admission한다.
- Server는 gameplay revision과 K source revision을 action lifetime 동안 pin하고 result/lifecycle에 되돌려 준다.
- Client는 local revision, expected revision, Server pinned revision 및 Pattern/Stage/action ID가 모두 맞을 때만
  `[Live]`를 표시한다. Publish 뒤 Server를 재시작하지 않은 stale 요청은 false live 없이 거절된다.

### 1.5 Valtan 회귀 경계

- Valtan exact oracle은 primary `BOSS_VALTAN + ENCOUNTER_VALTAN` row에만 적용한다.
- K nullable weapon 허용도 exact K encounter/boss tuple에만 적용한다.
- K authoring/publisher module은 Valtan domain module을 import하지 않고 Valtan source는 K authoring을 참조하지 않는다.
- 다만 최종 Gameplay bootstrap writer와 catalog admission은 아직 전역이다. invalid K Product가 전체 bootstrap을
  거절할 가능성을 제거하는 encounter별 admission은 후속 별도 슬라이스다.

## 2. 자동 검증 결과

- KoukuSaydon pipeline: 72 tests PASS
- Build domain pipeline: 18 tests PASS
- Valtan action presentation + Effect regression: 88 tests PASS
- Valtan status + world entity protocol regression: 21 tests PASS
- NetworkProtocolHarness Debug: `failures : 0`
- Server Debug contract: `failures : 0`
- K composition projector validate/publish PASS
- Gameplay balance/world publisher Validate/Publish PASS
- Debug Product build PASS
  - receipt: `out/BuildPipeline/receipts/product.debug.receipt.json`
  - evidence: `out/BuildPipeline/runs/20260904T191854245Z-debug-product-fbe17ad1.json`
- Debug Core PASS
  - Product/Shared/Server/Client compile·link와 compiled-shader closure PASS
  - NetworkProtocolHarness와 Character Select isolation `failures : 0`
  - Valtan suites와 최종 source-identity guard PASS
  - evidence: `out/BuildPipeline/runs/20260904T193548464Z-debug-core-20def282.json`
- K JSON과 변경한 Client/Server project/filter XML parse PASS
- `git diff --check` PASS. 출력의 LF→CRLF 경고는 기존 working-copy line-ending 경고이며 whitespace 오류는 없다.+

## 3. 사용자가 직접 닫을 visual smoke

에이전트는 Client/UI를 실행하거나 시각 판정을 대신하지 않았다. 다음 순서는 사용자가 직접 확인한다.

1. 이 PC에서 Visual Studio의 Debug `Server + Client` profile을 `Ctrl+F5`로 시작한다.
2. KoukuSaydon Arena에 진입하고 F1에서 `KoukuSaydon Action Workbench`를 연다.
3. 다섯 Pattern과 현재 PRODUCT인 Pizza를 확인한다.
4. 빈 DRAFT Pattern을 선택하고 `MN_RPCZ_00` Resource leaf를 눌러 local preview를 본다.
5. non-loop clip을 `Append as Stage`하고 Stage/box를 조정한 뒤 Authoring을 PRODUCT로 바꾸고 Save한다.
6. `Publish All PRODUCT` 성공을 확인한 뒤 Debug Server를 재시작하고 arena에 다시 진입한다.
7. Workbench의 `Play Published Product (Server)` 또는 K Boss Tool의 `Reload Saved Product -> Play Selected`를
   누르고 실제 boss animation과 Server-confirmed `[Live]`를 확인한다.

## 4. 의도적으로 남긴 범위

### 4.1 P0 authoring content

- 방패 무력화, 진짜 쿠크세이튼 찾기, 댄스타임, 카드 맞추기의 실제 clip 선택과 Stage 구성은 사용자의 resource
  육안 확인 뒤 채워야 하므로 빈 DRAFT다.
- Product v1은 Stage마다 exact non-loop animation 한 개, offset/sourceStart 0, `playMs == durationMs`,
  `endPolicy == EXACT`, play rate 0.1~4.0만 Server에 투영한다.
- 전체 reference의 loop slot은 14개이고 그중 HOLDOUT 3개는 UI에서 숨는다. Workbench에 보이는
  REVIEW_CANDIDATE loop slot 11개와 이동/trim/복제/다중 box 결과는 DRAFT에는 저장할 수 있지만 아직 PRODUCT로
  publish할 수 없다. 이 편집을 실제 Server presentation에 반영하려면 timing/presentation executor 확장이
  다음 작업이다.

### 4.2 후속 family

- Sound
- Effect V1/V2
- Camera/World Sequence
- Rendering Workbench의 Scene Profile, directional/point/spot light, MapLightGroup, profile multiplier
- Screen/Post
- Collider, Summon, judgement
- HP mechanic, Logic/Counter/Branch, damage geometry

이 family들은 현재 composition에 빈 placeholder로 넣지 않았다. 각 family의 데이터 owner, validator, preview,
save rollback, Product projection, Server/Client consumer가 한 수직 슬라이스로 생길 때 순서대로 추가한다.

### 4.3 운영과 구조 후속

- runtime hot reload는 없다. Publish 뒤 Server 재시작이 필요하다.
- `1관문` 실제 부모 TreeNode UI와 raw resource 선택을 제품 Server boss에 즉시 audition하는 command는 아직 없다.
  현재 P0의 Server 경로는 PRODUCT Save/Publish/재시작 뒤 `Play Selected/Play All`이다.
- K Server brain은 현재 animation Stage clock만 소유한다. 전투/분기 확장 전에 encounter-neutral Stage executor
  경계를 추출하거나 명시적으로 재사용해야 한다.
- encounter별 bootstrap admission을 추가해 invalid K가 Valtan world admission까지 막지 않게 해야 한다.
- Save는 현재 네 immutable animation reference의 revision을 함께 pin한다. 해당 reference가 mutable family로
  넓어질 때 pattern이 실제 참조한 owner/revision만 CAS하는 구조로 좁혀야 한다.

## 5. 현재 판단

표준 non-loop clip 기준의 `Resource preview -> Append -> Save -> Publish -> Server Play -> [Live]` P0 경로는
코드와 자동 검증에서 닫혔다. 남은 P0 판정은 사용자의 실제 Client 육안 smoke와 네 빈 패턴의 clip 선택이다.
Sound/Effect/Light/Camera/Logic은 이 최소 경로의 미완료가 아니라 다음 family 수직 슬라이스다.

## 6. G05 공용 편집기 1단계 결과 (2026-09-05 13:05 KST)

### 6.1 구현 상태

- 신규 `Client/Public/CompositionResourceTree.h`, `Client/Private/CompositionResourceTree.cpp`: Valtan Workbench 익명
  namespace에 있던 tree 삽입·정렬·렌더 helper를 encounter 중립 공용 부품으로 옮겼다. `Client.vcxproj`와
  `.filters`의 `03. Tools\05. Sequencer`에 등록했다. Valtan Workbench는 struct와 helper 정의를 잃고 같은 이름을
  공용 include로 호출하며 동작은 바뀌지 않았다.
- `CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory`가 Kouku/Large Kouku/Saydon 분류 규칙의 단일
  소유자다. `Animation_Tool`의 기존 해석기는 여기에 위임한다.
- 쿠크 Resources 창(`Composition Resources###KoukuCompositionResources`)은 Reload 직후부터 Category/Profile 트리에
  네 profile의 extracted action을 모두 보여 준다. action 클릭은 Preview Action, source stage 클릭은 stage preview,
  slot 클릭은 single-clip preview다. Selected Action 패널은 `Preview Action`, `Append Action as Stages`,
  `Append Action to Selected Stage`를, Selected Clip 패널은 `Play Preview`, `Append as Stage`, `Add Animation Row`를
  제공한다. `Physical Clips / MN_RPCZ_00` 섹션은 staged 모델의 실제 clip 목록이다. 검색은 대소문자를 무시하고
  action 이름·profile·action ID·clip 이름을 대상으로 한다.
- Append/Bind는 `Is_AppendAdmitted`가 소유한다. 모든 profile을 browse·preview할 수 있지만 bind는
  `BOSS_BODY_PROFILE_ID`(`MN_RPCZ_00`)만 허용하고 사유를 상태에 남긴다.
- Box Detail은 native clip 길이, source window, `Play Rate`, `End Policy`, `Apply Playback`을 추가했다.
  `Set_AnimationPlayback`은 EXACT가 native clip을 넘는 조합을 거부한다. drag/trim/append/bind는 EXACT window가
  native를 넘으면 `HOLD_LAST_POSE`로 전환하고 상태 문자열에 남긴다.
- Timeline은 `Pause/Resume`, `Stop`, ruler 클릭·드래그 seek/scrub, cursor와 playhead를 추가했다. transport는
  `Consume_PreviewTransportRequest` 값 요청으로 MainApp이 Animation Tool의 pause/seek/stop API에 전달하고,
  `Get_KoukuCompositionPreviewState` snapshot을 `Set_PreviewState`로 돌려준다. Workbench는 모델을 들지 않는다.
- Animation Tool preview는 EXACT window가 native clip을 넘는 row를 더 이상 건너뛰지 않고 마지막 pose를 hold한다.
- Pattern 목록은 `[PRODUCT]`와 `[Error]` 표식을 함께 표시한다.

### 6.2 자동 검증

- Debug Product build PASS: `out/KoukuSaydon/g05-product-build.log`, compile result
  `out/BuildPipeline/runs/20260905T040445515Z-debug-product.json`, `Client/Bin/Debug/Client.exe` 13:04:45 KST.
  변경 파일에서 새 error/warning 없음. `MainApp.cpp` C4819는 기존 경고다.
- `Tools/KoukuSaydonPipeline` 77 tests 중 76 PASS. 실패 1건 `test_kouku_saydon_naming_boundary`의
  `test_pipeline_directory_and_modules_use_canonical_authored_spelling`은 G04가 runner에서 뺀 naming-boundary gate를
  기대하는 기존 oracle이며 G05가 만지지 않은 파일이다.
- G05 계약 `KoukuSaydonSharedEditorContractTests` 4건 PASS. 같은 파일의 기존 oracle 두 곳을 현재 코드에 맞췄다.
  Boss Tool encounter 경로 문자열은 합쳐진 한 리터럴이고, 빈 composition 거부는 editor가 아니라 projector 책임이다.
- `Tools/ValtanPipeline` resource categories, workbench regression oracles, action presentation workbench contract
  110 tests OK. Valtan Workbench의 Kouku token 금지 oracle을 포함한다.
- `Tools/Build/test_build_domain_pipeline_receipts.py` 18 tests 중 4건 실패. 모두 G04의 runner·manifest 변경
  (`valtan.product` 등이 Product profile에서 빠짐, `Test-BuildProductReceipt` 미호출)에 대한 기존 oracle이며 G05 변경
  파일과 무관하다.
- `git diff --check` 공백 오류 없음.
- 같은 작업 트리에서 13:01~13:03 KST에 다른 세션이 `BalanceTool.cpp`, `ValtanBossTool.cpp`,
  `EncounterPatternReference.cpp`, `Publish-GameplayBalance.ps1`, `valtan_tuning_pipeline.py` 등을 갱신했다. G05
  변경 파일은 이후에도 토큰과 빌드 결과가 일치함을 확인했다. 해당 파일은 되돌리거나 정리하지 않았다.

### 6.3 사용자가 직접 닫을 화면 확인

1. `Server + Client` profile을 `Ctrl+F5`로 시작하고 KoukuSaydon Arena에 진입한다.
2. F1에서 `KoukuSaydon Action Workbench`를 연다. `Composition Resources` 창이 함께 열리고 Kouku / Large Kouku /
   Saydon 트리가 즉시 보이는지 확인한다.
3. `Refresh Model Clips` 뒤 `Physical Clips / MN_RPCZ_00`에 실제 clip 목록이 나오는지 확인한다.
4. action 하나를 클릭해 preview body가 action 전체를 재생하는지, slot 클릭이 단일 clip을 재생하는지 확인한다.
5. 빈 DRAFT pattern을 선택하고 `Append Action as Stages`와 `Add Animation Row`로 행을 만든 뒤 box 이동·양끝 trim·
   Stage 길이 drag가 되는지 확인한다.
6. `Play Family: Animation`으로 재생하고 `Pause`, `Stop`, ruler 클릭 seek와 playhead를 확인한다. 정지 상태에서 ruler를
   클릭해 cursor를 두고 Play하면 그 위치부터 시작하는지 확인한다.
7. Box Detail에서 `Play Rate`, `End Policy`를 바꿔 `Apply Playback`하고, box 끝을 native clip보다 길게 늘렸을 때
   `HOLD_LAST_POSE`로 바뀌고 preview가 마지막 pose를 유지하는지 확인한다.
8. `Save` 뒤 `Reload`로 같은 시간대가 복원되는지 확인한다.

### 6.4 이번 G에서 남긴 경계

- Valtan Resources의 공용 tree 채택은 완료했다. Valtan `Render_Timeline` lane/drag widget 추출과 공용 Sequencer 연결은 다음 G다.
- Stage family rows(Effect/Sound/Camera/Profile/Light/Summon)와 실제 재생·정리 consumer는 formatVersion 2 슬라이스다.
- 오류 row가 있는 pattern만 이전 Product를 유지하는 projector 보류 규칙은 아직 없다. 현재는 invalid PRODUCT pattern
  하나가 `Publish All PRODUCT` 전체를 exact pattern ID와 함께 거부한다.
- Valtan Save owner 축소는 시작하지 않았다.


## 7. G05 검토·수정 — 빌드 실행 전 상태 (2026-09-05)

### 7.1 판단과 실제 수정

보스별 문서·편집 세션·저장을 유지하고 Resource UI 부품을 공유하는 방향은 코드와 일치한다.
Valtan Animation/Effect/Sound와 쿠크 Resource가 실제 같은 tree helper를 소비하며 신규 H/CPP의 project/filter 등록도 확인했다.
쿠크 Save는 단일 Composition CAS, Action append는 candidate 전체 구성 후 한 번 commit하는 경로다.
이번 수정은 기존 미커밋 변경을 보존하고 아래 G05 동작에 한정했다.

- 공용 category node는 이름 뒤 개수가 바뀌어도 ID가 같도록 수정했다. 쿠크 Action node도 filtered index 대신
  profile/sourceAction ID와 고정 node ID를 사용한다. 검색 결과 변화로 다른 Action의 펼침 상태를 재사용하지 않는다.
- Selected Action의 slot 목록 높이를 제한하고 스크롤을 제공한다. Action 선택 시 이전 Selected Clip을 비우고,
  서로 다른 preview 요청은 마지막 요청 하나로 정리한다. Resource append의 조기 실패 이유도 창에 표시한다.
  Physical Clips는 처음부터 펼쳐 표시한다.
- Play Family는 클릭 당시 cursor를 요청 값에 포함한다. MainApp은 새 preview 시작 → transport → 상태 반환 순서로
  처리한다. 이전 preview가 재생 중일 때 그 상태를 보고 뒤늦게 seek하던 처리를 제거했다.
- preview 상태에 pattern ID와 실행 사유를 포함했다. 선택 Pattern에 해당할 때만 그 timeline의 playhead/seek를
  연결한다. Resource/Row/다른 Pattern 재생은 별도로 표시하며, Pattern 선택이 바뀌면 cursor를 초기화한다.
- preview 시작 전에 모델 이름 동기화를 끝내 첫 preview가 다음 Render의 asset adoption으로 종료되는 경로를 막았다.
- 뒤 clip에서 앞의 빈 구간으로 seek하면 직전 row의 끝 pose를 다시 계산한다. 처음 빈 구간은 preview 시작 시
  확보한 pose를 사용한다. 자연 종료와 끝점 seek는 duration clock과 마지막 pose를 paused 상태로 유지하며,
  끝에서 Resume는 처음부터 재생하고 Stop은 요청과 재생 상태를 정리한다.
- native 1000ms clip을 3000ms HOLD box로 늘린 뒤 왼쪽 1500ms를 trim하는 경우처럼 원본 끝 이후의 HOLD만
  남겨도 row를 건너뛰지 않고 마지막 pose를 표시한다. EXACT/LOOP의 Source Start는 모델 길이를 알 때 native
  범위 안으로 제한하며, Details의 잘못된 변경은 기존 값을 보존한다. drag의 source 시간도 문서 최대 범위로 제한한다.
- G04 이후 낡은 테스트 기대값 5건을 현재 Product/선택 진단 분리 계약으로 교정했다. Product에 검증 gate를
  다시 넣거나 테스트 전체를 삭제하지 않았다.

### 7.2 실행한 확인과 미실행 범위

| 확인 | 결과와 한계 |
|---|---|
| 기존 G05 shared-editor 구조 검사 4개 | PASS. 문자열·호출 구조 검사이며 C++ 실행 증거가 아니다 |
| 기존 Composition projector 동작 검사 19개 | PASS. 임시 파일을 사용하는 Python 투영·저장 실패 동작 검사 |
| naming/build-domain 기존 검사 24개 | PASS. Product/선택 domain selector·명시 publisher·runner 분리 계약 |
| Client project/filter XML | parse PASS, 공용 H/CPP 각각 한 번 등록 |
| 수정 C++ 인코딩 | 기존 UTF-8 및 파일별 줄바꿈 유지 |
| `git diff --check` | PASS. 기존 working-copy 줄바꿈 경고만 있음 |
| 이번 수정 뒤 Debug Product 빌드 | 미실행. 사용자가 빌드 가능 시점 보고를 요청함 |
| Core / FullDiagnostic | 미실행. 이번 편집 수정의 필수 절차가 아님 |
| Client/Arena 화면·pose 확인 | 미실행. 사용자 직접 확인 대상 |

실행 명령:

```powershell
python -B -m unittest Tools.KoukuSaydonPipeline.test_kouku_saydon_client_product_level_contract.KoukuSaydonSharedEditorContractTests Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition
python -B -m unittest Tools.KoukuSaydonPipeline.test_kouku_saydon_naming_boundary Tools.Build.test_build_domain_pipeline_receipts
```

다음 빌드는 아래 Product 명령으로 진행할 수 있다. 이 명령을 이번 검토에서는 실행하지 않았다.
6.2의 13:04:45 Client.exe는 이번 수정 전 바이너리다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

### 7.3 빌드 후 사용자가 확인할 동작

`Server + Client` → KoukuSaydon Arena → F1 → `KoukuSaydon Action Workbench`에서 확인한다.

1. Resources에서 Action을 펼친 뒤 검색어를 바꿨다가 지운다. 선택 Action과 펼침 상태를 확인하고, Action 선택 뒤
   이전 clip의 Append 버튼이 남지 않는지 확인한다.
2. DRAFT에 clip을 Append하고 box/Stage를 조정한다. Save → Reload 후 시간과 policy가 유지되는지 확인한다.
3. 정지 상태에서 ruler의 중간 위치를 선택하고 Play Family한다. 첫 pose가 해당 시점인지 확인한다.
   재생 중 다른 Pattern이나 Resource를 선택해 이전 preview playhead가 새 Pattern에 섞이지 않는지 확인한다.
4. 두 clip 사이에 gap을 만든 뒤 뒤 clip에서 gap으로 역방향 seek한다. 앞 clip 종료 pose인지 확인하고,
   끝점 seek → Resume → Stop도 확인한다.
5. native보다 긴 HOLD box의 왼쪽을 native 끝 이후까지 trim한다. 남은 HOLD tail이 마지막 pose로 재생되는지 확인한다.

### 7.4 아직 완료로 보지 않는 범위

- 공용화된 것은 Resource tree다. Valtan의 별도 Sequencer/timeline lane/Box Detail 전체를 두 보스가 공유하는 단계는 남았다.
- Effect/Sound/Camera/Profile/Light/Summon row 추가·저장·family 실행은 이번 Animation 수정으로 구현된 것이 아니다.
- 현재 Resource/Row/Family preview는 아레나의 별도 로컬 preview 모델이다. 실제 Server 쿠크 본체 즉시 audition은 남았다.
- Product v1은 기존 제한을 유지하므로 HOLD/loop/offset/다중 Animation box를 DRAFT Save했다고 Server에 자동 반영되지 않는다.
- 공용 Animation Tool/PreviewPanel에는 다른 저작 문서의 unsaved 상태를 보호하는 target 전환 lock이 남아 있다.
  쿠크 Composition Save/Append의 Valtan gate는 아니지만, 보스 간 preview 전환까지 완전한 독립 세션이라고 볼 수는 없다.
  이 lock을 단순 제거하여 다른 문서를 잃게 하지 말고, 후속 공용화에서 문서 세션 보존과 preview target 전환을 분리해야 한다.
- Valtan의 다중 Save owner 축소와 오류 Product별 publish 보류는 별도 남은 작업이다.


## 8. G06/G07 공용 Workbench·전체 Animation 연결

사용자가 확정한 이번 범위는 발탄·쿠크 **Animation 전체 목록과 preview, Append·Save 및 공용 Sequencer 구조**다.
Effect/Sound/Camera의 새 쿠크 row 구현은 이번 Animation 연결에 포함하지 않는다.
기존 Valtan 기능을 삭제하지 않고 두 보스의 독립된 session을 하나의 `Action Workbench` 창 구성에 연결했다.
이 절의 코드 반영 상태와 C++ 빌드·사용자 실제 화면 확인 상태를 구분한다.

### 8.1 공용 화면과 보스별 데이터

`MainApp → CSequencerTool → ICompositionWorkbenchSession`이 실제 화면 경로다.
F1 launcher와 focus 목록은 `Action Workbench` 하나이고 Boss에서 Valtan/KoukuSaydon을 선택한다.
두 Workbench의 내부 객체는 각 보스의 문서·draft·선택·저장 adapter로 남으며 별도 최상위 편집기로 Render하지 않는다.
기존 enum의 V/K 진입은 호환용 boss 선택 alias다.

- Sequencer, Patterns, Composition Resources, Box Detail, Preview, Boss Pattern은 기존 Valtan 창 ID·배치·resize를 사용한다.
- 공용 Windows 메뉴로 창 표시, Resources 확대, Sequencer 최대화, 배치 초기화를 제어한다.
- ruler, box drawing, 양끝 trim hit test는 `CompositionTimeline.h`를 두 session이 실제 호출한다.
- 선택 session만 frame 준비/화면 그리기를 수행한다. Valtan의 deferred Save/선택/Append는 pane이 이전 포인터 사용을 끝낸 뒤 실행한다.
- 선택하지 않은 session의 Save/Publish process 관찰은 계속하며 Boss 전환으로 draft를 Reload하거나 지우지 않는다.
- 공용 shell은 `BossCompositionDocument`/manifest admission을 편집 진입에 사용하지 않는다.
- preview backend 생성 시 Balance를 지연 초기화하고, 발탄 정본 초기화는 실제 Valtan/Balance 기능을 사용할 때 수행한다.

쿠크 Pattern·Stage·Animation 편집은 기존 candidate commit과 단일 Composition CAS Save를 그대로 사용한다.
발탄은 기존 family editor와 split source writer를 유지한다. 쿠크의 편집/Save 호출은 발탄 PatternTree, Sound/Shake admission이나 발탄 Save job에 참여하지 않는다.

### 8.2 실제 Animation Resources 범위

공용 Physical Animation browser가 아래 여섯 몸체의 clip을 어느 Boss 선택에서도 보여 준다.
검색·선택·click preview·Play Preview·Pause/Resume·Stop·Append as Stage·Add Animation Row를 연결했다.
선택 body와 저장 대상 boss가 다르면 preview는 가능하고 Append 불가 사유를 표시한다.

| preview target | 실제 파일의 clip 수 | 읽는 입력 |
|---|---:|---|
| Valtan | 173 | 본체 27 + MN_RPBF_01 AnimSet 146 |
| Valtan_Ghost_MN_RPBF_02 | 286 | Ghost 본체 140 + Ghost AnimSet 146 |
| MN_RPCT_00 | 249 | 본체 WModel |
| MN_RPCT_05 | 249 | 본체 WModel |
| MN_RPCT_06 | 34 | 본체 WModel |
| MN_RPCZ_00 | 91 | 본체 WModel |
| 합계 | 1,082 | body마다 독립된 항목 |

같은 clip 이름을 두 rig가 사용해도 합치지 않는다. selection identity는 body/package/clip이다.
MN_RPCT_07 참고 profile은 실제 MN_RPCT_05 preview body를 사용한다. 악세서리·무기 mesh, 플레이어·일반 몬스터까지
모든 Resources 파일을 수집하는 기능이라는 뜻은 아니다. 이번 범위는 위에 실측한 발탄·쿠크 보스 몸체다.

쿠크 session의 기존 Action 참고 트리는 Valtan sequence와 Kouku/Large Kouku/Saydon reference를 유지한다.
Valtan sequence는 265개 sequence stable ID를 개별 보존하고 authored cut 길이를 preview한다.
쿠크 네 profile의 비어 있지 않은 참고 Action은 335개다. 참고 트리와 Physical 목록을 따로 탐색하며
참고 문서에 없는 실제 clip도 Physical 목록에서 선택한다. 발탄 session의 기존 family/reference Resources는 공용 Physical 목록 아래에 유지한다.

`CWModelDecoder::Read_AnimationCatalog`는 WINT/WMOD section table과 WANM header의 name/ticks/tps만 읽는다.
메시·키프레임·texture/model 생성이나 전체 파일 hash를 목록 생성의 선행조건으로 사용하지 않는다.
특정 package 갱신 실패는 그 package의 이전 행과 실패 사유를 보존하며 다른 파일의 정상 목록은 유지한다.
Refresh 후 선택 clip은 새 metadata에 맞추거나 선택을 해제해 오래된 길이로 Append하지 않는다.

일부 파일은 import ticksPerSecond가 30과 다르지만 현재 Engine `CAnimation`은 cooked 재생을 30 ticks/s로 수행한다.
예를 들어 발탄 idle_battle_1은 파일 계산 2,333ms와 실제 재생 2,233ms가 달랐다.
기존 runtime 동작을 바꾸지 않고 `CAnimation::COOKED_TICK_RATE`를 공용 상수로 노출해 Resources·Append·native window
검사가 같은 runtime 길이를 사용하도록 맞췄다. 파일의 import 속도로 길이를 계산해 실제보다 긴 box를 만드는 오류를 제거했다.

### 8.3 Preview Action 오류와 창 열기 수정

`EnsureDebugTool(ANIMATION)`이 자동으로 창 표시/focus까지 수행하던 경로를 `EnsureAnimationPreviewBackend`와 분리했다.
Resources 클릭과 Play Preview는 Animation Tool의 별도 창을 열지 않는다. 명시적 Open Animation 명령은 기존 동작을 유지한다.

기존 preview 실패는 raid에서 `CAnimationTargetService::Resolve_SceneCharacter()`가 복제 플레이어를 제공하지 않는 데서 발생했다.
Valtan은 기존 `Try_Get_AuthoringPreviewPlacement`, 쿠크는 `CLevel_KakulSaydonArena::Try_Get_AuthoringPreviewPlacement`로
해당 Level의 실제 local replicated character 위치를 사용한다. 카메라 오른쪽 3.25m와 지면 샘플을 사용하며
플레이어가 아직 없으면 구체적인 준비 상태를 표시한다. 월드 원점에 다른 캐릭터를 대신 생성하지 않는다.

기존 Prototype → Clone → Layer → CModel preview 경로가 실제 재생을 소유한다.
Resources의 단일 clip과 sequence는 body를 명시한 값 요청으로 전달하며 authored offset/gap/source window/end policy는
기존 composition sampler로 재생한다. 공용 raw transport는 Boss 전환 후에도 같은 preview를 제어한다.
패턴이 비었거나 오류여도 쿠크 Preview pane의 raw Pause/Stop은 유지한다.

이 preview는 아레나의 별도 collision-off 로컬 모델이다. 실제 Server 쿠크 본체에 raw clip을 즉시 실행하는 기능은 아니다.

### 8.4 Append·Save의 실제 경계

쿠크 본체 MN_RPCZ_00 clip은 `Append_CompositionAnimationResource`에서 기존 `Append_AnimationAsStage` 또는
`Bind_Animation`으로 전달한다. 새 stable Stage/occurrence ID를 부여하고, candidate 검사가 성공한 때만 draft를 교체한다.
전체 Pattern의 Stage/box 변경은 Save로 같은 Composition 파일에 저장하고 Reload에서 다시 읽는다.
보스/rig가 다른 clip을 이름만 복사해 쿠크 본체의 clip인 것처럼 저장하지 않는다.

발탄 raw Append는 현재 preview 모델과 무관하게 metadata clip을 선택 Pattern의 사본에 추가한다.
새 Stage와 첫 animation은 한 번에 commit하여 실패 시 빈 Stage만 남기지 않는다.
변경한 Stage의 native window 검사와 Save는 CModel을 강제로 열지 않고 duration ticks와 실제 runtime clock을 사용한다.
발탄 raw Append는 현재 Workbench와 Balance의 source revision 일치를 확인한다. 다른 세대의 draft에 변경을 보내지 않는다.
기존 HOLD/gap/repeat Stage에 중간 hold를 표현할 수 없는 경우에는 Add Animation Row를 거절하고 Append as Stage를 안내한다.
기존 row의 시간이나 Stage 끝을 묵시적으로 줄이지 않는다.
Server runtime publish 기본값은 껐으며 명시 선택만 실행한다.

발탄의 기존 split source join, source revision pin, 다중 owner Save/reopen 구조 전체를 이번에 새 저장 형식으로 이주한 것은 아니다.
발탄이 처음부터 STALE/REJECTED인 경우에는 기존 Save 제한이 남아 있다. 이 제한을 쿠크 session에 전달하지 않는다.
새로 연결한 정상 발탄 session의 raw Append/Save 경로와 모든 기존 발탄 저장 병목 제거를 같은 완료 상태로 보고하지 않는다.

### 8.5 확인 상태

- C++ 코드 연결과 독립 리뷰: 수행. preview target/창 표시, 원자 Append, frame 포인터 수명, selection metadata 갱신 경로를 확인했다.
- 마지막 수정 뒤 기존 focused Python 검사: 29개 PASS(쿠크 편집·preview 연결, descriptor reader, 발탄 native window/지연 초기화, 쿠크 Product projector). C++ 실행 결과를 대신하지 않는다.
- 삭제한 read-only Sequencer facade/Animation 창 자동 열기의 낡은 UI oracle 네 개는 제거했다. 남은 descriptor reader 검사는 유지한다.
- Client project/filter·Engine project XML, 쿠크 Composition/Encounter JSON parse: PASS. 공용 헤더 세 개는 각 project/filter에 한 번 등록돼 있다.
- `git diff --check`: 공백 오류 없음. 기존 working-copy 줄바꿈 경고는 남아 있다.
- 여섯 body/여덟 package header 실측: 1,082개. 결과는 `out/KoukuSaydon/g07-animation-resource-review.json`에 기록했다.
- 이번 C++ 변경 뒤 Product compile/link: 미실행. 사용자가 빌드 가능 시점 보고를 요청한 상태를 유지했다.
- Core/FullDiagnostic 및 광역 검증: 미실행. 편집 진입이나 Save의 필수 단계로 추가하지 않았다.
- Client 실행·UI 조작·화면 capture·최종 pose 확인: 미실행. 사용자 직접 확인 대상이다.
- 기존 dirty worktree를 보존했으며 stage/commit/push하지 않았다.

### 8.6 빌드 후 사용자 확인 경로

다음 빌드는 Engine public header와 Client 변경을 함께 반영해야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

`Server + Client` 실행 → KoukuSaydon Arena → F1 → `Action Workbench` → Boss `KoukuSaydon` 순서다.

1. Composition Resources의 Physical Animation에서 여섯 body와 clip 수를 확인한다. 각 body clip을 클릭하고 Play Preview한다.
   Animation Tool 창이 새로 열리지 않고 플레이어 주변 해당 body가 재생되는지 확인한다. Pause/Resume/Stop도 확인한다.
2. MN_RPCZ_00 clip 선택 → 편집할 DRAFT Pattern 선택 → Append as Stage → Add Animation Row.
   Sequencer에서 Stage 선택·길이 조절·box drag/trim을 하고 Box Detail에서 playback 값을 편집한다.
3. Play Family → ruler seek → Save → Reload하여 전체 Pattern의 Stage/occurrence/길이/offset/policy가 유지되는지 확인한다.
4. Boss를 Valtan으로 바꿨다가 KoukuSaydon으로 돌아와 저장 전 draft와 선택이 보존되는지 확인한다.
5. Valtan의 편집 가능한 Pattern에서 Valtan physical clip을 Append한다. 다른 body를 preview한 뒤 Valtan Save가
   현재 preview 모델을 이유로 막히지 않는지 확인한다. legacy source가 이미 STALE라면 표시된 원인을 따로 기록한다.

쿠크 Effect/Sound/Camera/Profile/Light/Summon의 일반 row 저작·family 재생, Product v1 timing 확장,
Server raw audition/hot reload는 이 Animation 작업으로 완료됐다고 표시하지 않는다.


## 9. G07 Sequencer 컴파일 오류 수정 — 2026-09-05

사용자 빌드 로그에서 `SequencerTool.h`의 `NS_BEGIN(Client)`부터 C4430/C2882가 발생하고,
클래스 선언이 무너져 `SequencerTool.cpp`에 100개 이상의 후속 오류가 발생한 것을 확인했다.
공용 shell로 분리하면서 기존 Workbench include를 제거했으나, 직접 사용하는 `NS_BEGIN`,
`NS_END`, `bool_t`의 정의를 제공하는 `Engine_Defines.h` include가 빠진 것이 원인이었다.
`SequencerTool.h`에 해당 include를 명시했다. 편집·저장·preview 동작은 변경하지 않았다.

- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS, exit code 0.
- Engine → Shared → Server → Client compile/link 및 Engine SDK/runtime 배포: PASS.
- SequencerTool.obj 생성 및 Client.exe 최종 링크: 확인.
- 이전 사용자 로그의 Engine.lib LNK1114 오류: 이번 순차 Product 빌드에서 재발하지 않음.
- 빌드 로그: `out/KoukuSaydon/g07-compile-fix-product.log`.
- 결과 JSON: `out/BuildPipeline/runs/20260905T053309319Z-debug-product.json`.
- 기존 C4819, shader deprecation, 외부 라이브러리 PDB 경고는 남아 있으며 컴파일 오류는 없음.
- Core/FullDiagnostic, data publisher, runtime 진단과 Client/UI 실행: 미실행.
- 위 8.5절의 C++ 빌드 미실행 상태는 당시 기록이며, 현재 compile/link 상태는 이 절을 따른다.


## 10. G08 Lobby 쿠크 입장 버튼과 Raid Entry 영상 시간 수정 — 2026-09-05

### 구현

- `LobbyCommandService.h`: 기존 enum 값은 유지하고 `LOBBY_STAGE::KOUKU_SAYDON` 추가.
- `Level_Lobby.cpp`: Lobby ImGui에 KoukuSaydon 버튼, Resolve_Stage에 기존 K world/level 매핑 추가.
- `CharacterSelectionState.cpp`: K world에도 발탄과 같은 selected class 및 created/audition nickname 규칙 연결.
  기존 Server 승인 payload 검사와 one-shot socket handoff 뒤에만 Level을 전환한다.
- Character Select의 `Enter KoukuSaydon Arena`는 이미 연결되어 있어 중복 버튼을 추가하지 않았다.
  O키 Raid Entry Preview가 열려 있으면 해당 ImGui 창 전체가 숨겨지는 기존 동작을 유지한다.
- `RaidEntryPreviewView.cpp`: portrait 동영상과 탭 easing의 시간 입력을 Timer_Default에서 Timer_60으로 수정.
  전자는 Client.cpp의 idle loop마다 갱신되고, 후자는 실제 MainApp Update에 전달되는 frame seconds다.
  따라서 기존 코드는 여유 프레임에서 극히 짧은 idle delta만 누적해 영상이 느려지거나 부하에 따라 속도가 달라졌다.
  Bern 실제 입장창과 Character Select O키 입장창이 같은 수정 경로를 사용한다.

### 확인 결과

- Client Debug x64 ClCompile와 Build: 각각 exit code 0, 컴파일·링크 오류 0.
- 새 `Client/Bin/Debug/Client.exe`: 2026-09-05 14:44:58 KST.
- 발탄·쿠크 영상의 연속 000~299 DDS: 각각 300개 존재, DDS header 및 800x560 크기 확인.
  기존 동영상은 30fps, 10초 루프의 프레임 재생이며 새 MOV container decoder를 추가한 것은 아니다.
- 로그: `out/KoukuSaydon/g08-entry-movie-compile.log`, `g08-entry-movie-build.log`.
- 파일 확인: `out/KoukuSaydon/g08-entry-movie-check.json`. `git diff --check` PASS.
- Server/Shared, data schema, project/filter 변경 없음. Core/FullDiagnostic·publisher·Client UI 실행은 미실행.
- 기존 인코딩/외부 PDB 경고는 남아 있다. 다른 작업자의 dirty 변경을 보존했고 commit/push하지 않았다.

### 사용자 확인과 남은 경계

1. Server + Client 실행 후 Lobby ImGui의 KoukuSaydon 버튼으로 승인·Arena 진입 확인.
2. Character Select에서는 기존 Enter KoukuSaydon Arena 사용. O키 입장창이 열려 있으면 Esc로 닫은 뒤 버튼 확인.
3. Character Select에서 O키 → 쿠크/발탄 탭을 번갈아 선택해 움직이는 배경과 탭 애니메이션이 정상 속도로 재생되는지 확인.
   Bern의 NPC 입장창도 같은 영상 runtime을 사용한다.
4. 실제 아레나 전체의 프레임 저하를 이 수정으로 해결했다고 판정하지 않는다. 조사한 공용 Arena/Character/Valtan
   갱신은 같은 frame delta를 사용하고 있었다. 지속 FPS 저하는 사용자 재현 위치와 frame-time 측정이 더 필요하다.
   요청한 MOV가 이 입장창 이외의 별도 동영상이면 파일 경로와 재생 화면은 아직 제공되지 않았다.

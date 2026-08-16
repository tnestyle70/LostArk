# 2026-08-14 4캐릭터·발탄 이펙트 복원과 레이드 성능 구현 결과

작업 시작일은 2026-08-14이며 마지막 갱신일은 2026-08-15다. 이 문서는 4캐릭터 Track A 후보,
Effect Tool, ordinary runtime, Valtan Whirlwind 메인과 사용자 육안 검증 상태의 단일 RESULT 정본이다.
자동 검증과 사용자 visual 판정은 항상 분리해서 기록한다.

## 1. 이번 변경의 결론

Artist F의 사용자 visual 승인을 기준으로 Track A를 제품 런타임과 병행하는 두 번째 경로로
확장하지 않고, authored schema v13 문서 하나를 ordinary `Stage_Document`로 재생하는 방향을
유지했다.

이번 변경에서 자동 검증까지 닫힌 범위는 다음과 같다.

- 공용 Track A element import transaction과 rollback 계약
- executable Track A 12 BA stage와 Warlord 17000 BA1 canary를 위한 13-stage batch seam
- 4직업 101개 full-source occurrence의 source-backed v13 `.unified` 후보 materialization
- Effect Tool 사용자 튜닝을 보존하는 field-aware Track A reimport
- All Effects의 101개 후보 Open/Edit/Save 일반화와 admission-aware Play/Visible 잠금
- ordinary `CEffectObject::Stage_Document`를 통한 source recipe 실제 소비와 rollback 검증
- Valtan 420633 Whirlwind active stage의 exact source mapping, authored v13 canary, Model View join
- Valtan animation/effect의 authoritative Server timeline 전달
- Effect Tool과 runtime의 반복 validation, document copy, frame allocation, anchor rebuild 일부 제거
- owner/world aggregate effect budget과 remote cosmetic suppression
- Server ingress/outbound backpressure, slow reader 격리와 reliable terminal flush

4직업 전체 intake는 51 skills/74 stages/113 clip occurrences다. visual target 101개는 authored v13
`.unified` 파일로 존재한다. 과거 Legacy-selected partial surface checkpoint는 폐기했고, 현재 strict
occurrence 출력은 아래 1.1절의 4,777 elements로 transaction write됐다. 기존 Legacy는 후보 생성 source가
아니라 rollback/reference로 유지하고 product mapping은 0으로 유지했다.

Valtan Whirlwind canary는 9개 first-LOD carrier 중 3개만 ordinary authored v13으로 실행 가능하다.
따라서 Model View 검증 대상으로는 준비됐지만 제품 catalog/animevent mapping은 계속 차단했다.

### 1.1 2026-08-15 baseline 보존·G3 통합 전 체크포인트

이 문서의 과거 `101/101`, `1,677 probe PASS`는 selected 2,160 surface 시점의 구조 검증 기록이다.
사용자 화면에서 DimensionMaster A가 `fm_m_trail` 한 종류만 보이고 Sprite family가 잠긴 사실을 확인한 뒤
그 결과를 visual-ready 판정에서 철회했다.

현재 디스크에는 source occurrence, material profile과 별도 AnimationTrail family를 다시 결합한 101문서가
존재한다. 이 문서는 Claude G3 코드 통합 뒤 대표 육안검증에 사용할 **기존 baseline**이며, Codex의
authoring-approximate projection으로 다시 쓰지 않았다.

- source Particle corpus: 4,846
- 현재 101 occurrence strict mapped Particle: 4,687
- typed exclusion: 159
- Particle portable / source-preserved deferred: 4,488 / 199
- portable Particle product-full / fail-closed: 3,481 / 1,007
- Decal: 79 = Base-ready 46 + authoring-incomplete 33
- source-backed AnimationTrail: 8 notify / 11 elements
- placeholder Trail: 0
- `CHARACTER_AFTERIMAGE` receipt ledger: 72 occurrences / 29 target documents
- current output: 101 documents / 4,777 elements
- authored `authoringApproximate`: 0
- DimensionMaster canonical sibling join: 784
- product mapping mutation: 0
- restoration receipt SHA-256:
  `15bc2eb079575fde3bf97091d6a6cb887acfc21460ac5bdbf8fb32ef09e6a7af`

공용 source Material compiler는 material identity와 occurrence parameter를 분리하고 exact physical
package/material full-path join을 사용한다. DimensionMaster canonical sibling material은 stable ID가 일치하는
784개에만 적용했고 Artist/Lance/Warlord에는 추정 복사하지 않았다. 타 직업은 각 source receipt에 실제로
존재하는 parent, texture, scalar/vector/dynamic parameter와 typed evaluator만 사용한다. admitted carrier의
`effect.standard` fallback은 0이다.

Wave 0 parent Material evidence, strict authoring/product admission과 Lance BA4 dynamic-arithmetic 경계를 읽는
현재 **no-write projection**은 product-full 2,793 / authoring-approximate 722 / hard portable fail-closed 973,
preview target 3,515다. portable 4,488 / deferred 199 / output 4,777은 변하지 않는다. 이 수치는 아직
`EXPECTED_*`나 authored 101의 정본이 아니다.

사용자 승인 순서는 `admission/Lance/provenance 소묶음 마감 → 중지 → Claude G3 코드 통합 → Client Debug
빌드 → 기존 baseline 대표 육안검증 → 방향 승인 시 최종 denominator/rewrite/full gate`다. 따라서 이 RESULT는
baseline과 projection을 섞어 완료로 기록하지 않는다.

AnimationTrail 11개는 source event/emitter identity와 exact material/resource를 가진 별도 weapon-bone history
family이며 Cascade `TypeDataRibbon`을 포함하지 않는다. `TrailGhostEffect` 72 occurrence는 29개 문서에
`CHARACTER_AFTERIMAGE` raw payload/lineage ledger로 byte-lossless 보존했지만 body/equipment pose snapshot과
ghost material runtime이 없으므로 generic Trail element로 투영하지 않고 실행 deferred 상태다.

자동 검증 checkpoint는 다음과 같다.

- base source-material contract tests: 32/32 PASS
- four-class source-material compiler tests: focused 39/39 PASS
- four-class source-material compiler `--check`: PASS, seed 50 + compiled 776 + blocked/no-package 1
- physical `.materials.json` input: 22 files, receipt SHA 고정
- portable particle runtime capability tests: 12/12 PASS
- previous baseline candidate write/check: PASS, 101 documents / 4,777 elements
- current no-write projection partition: 2,793 + 722 + 973 = 4,488, preview 3,515
- exact named texture: 6,211, class-local rebase 1,873, manifest-resolved 124, unresolved 1,338
- policy 없는 `EngineMaterials` 15 occurrence: durable fail-closed
- Valtan Python unit tests 15/15, canary builder와 boss-pattern mapping check: PASS
- ClientFrontendHarness x64 Debug build: PASS
- G3 generic authoring ownership: `--effect-g3-authoring-fast` 15/15, Python 3/3,
  `Test-EffectPipeline.ps1`, 대표 authored load 7/7 PASS
- Client x64 Debug full compile/link: PASS

current projection의 denominator rebaseline, 101 rewrite, 전체 native Stage/Draw와 Client Release compile/link는
아직 실행하지 않았다. 사용자는 기존 baseline에서 Artist F와 DimensionMaster A를 승인했지만, 아래 네 스킬
결함과 Valtan UI index 차단을 확인했으므로 전체 visual direction gate는 계속 열려 있다.

## 2. 권위와 마이그레이션 경계

최종 흐름은 다음 하나다.

```text
Legacy JSON / Track A / 원본 추출 evidence
              ↓ 명시적 import·admission 한 번
새 authored v13 .effect.json
              ↓ Effect Tool 편집·저장
ordinary CEffectObject::Stage_Document
              ↓ 사용자 visual 승인
catalog + animation 또는 boss-pattern mapping 전환
```

- 정상 제품 선택지는 새 authored Effect 하나다.
- 이전 Effect, Track A visual program과 source evidence는 Advanced Migration Reference와 rollback
  근거로만 유지한다.
- unresolved material, resource, family, attachment를 generic fallback으로 보이게 하지 않는다.
- Artist F 전용 Core33, shader registry, `modelPreScale=0.01`, fixed-burst 보정은 공용 importer에
  복제하지 않았다.
- product mapping과 candidate materialization은 별도 transaction이다.

## 3. Artist F 승인과 제품 전환 상태

사용자가 이번 대화에서 Artist F visual 검증 완료를 서면으로 확인했다. 이 승인은 visual gate의
완료이며 정량 FPS 수치까지 승인했다는 의미로 확대하지 않는다.

Artist F authored 문서는 schema v13, 33 elements이며 Track A에서 복원한 29 recipes,
350 modules, 564 distributions와 typed material execution을 보존한다. Ribbon, 일부 Decal,
live follow는 원본 완전 재현 범위 밖으로 계속 구분한다.

이전 reconstructed runtime entry는 삭제하지 않고 Advanced/rollback reference로 유지한다.
제품 cue는 direct-authored v13 publisher와 catalog/runtime validator가 모두 통과한 뒤에만
`.unified` asset으로 전환한다.

## 4. 공용 Track A authored import transaction

`CEffectDocumentCodec::Build_GenericAuthoredElementImportStage`가 다음 계약을 소유한다.

1. source/target canonical snapshot을 보존한다.
2. generic starting copy에서 native runtime evidence를 제거한다.
3. cue/emitter timing과 transform을 정확히 한 번 bake한다.
4. recipe가 없는 element 상태로 stable target에 merge한다.
5. merged target에 portable authored particle carrier를 적용한다.
6. caller가 제공한 typed material execution 또는 명시 fail-closed 상태를 적용한다.
7. serialize, parse, drawable validate, canonical equality를 확인한다.
8. 모든 단계가 성공할 때만 output을 commit한다.

direct `Merge_GenericAuthoredElements`에 recipe-enabled incoming element를 넣는 우회는 계속
거부한다. ActionCue parameter binding, unsupported module, timing drift, duplicate stable ID,
material mismatch는 source/target/output 불변 상태로 rollback한다.

`Build_GenericAuthoredElementReimportStage`는 최초 import 뒤 사용자가 Effect Tool에서 저장한 값을
안전하게 유지하는 별도 seam이다. compiler가 `SourceRecipe`, WModel, non-Decal DDS/Material을 갱신하고,
사용자가 저작한 `Detail`, `Visible`, action cue attachment, transform inheritance와 Decal Base DDS/Material은
보존한다. missing/ambiguous/unsafe binding은 canonical round-trip 전 commit하지 않는다. ordinary portable
`SourceRecipe` UI는 read-only evidence로 표시하고 일반 Load/Save의 recipe/resource round-trip은 유지한다.

## 5. 4직업 101개 Track A 후보

첫 C++ batch seam은 다음 13 stage를 대상으로 시작했다.

- Artist 31000 BA1~BA4
- DimensionMaster 2050010 BA1~BA4
- LanceMaster 34010 BA1~BA4
- Warlord 17000 BA1 canary

Artist F 31470은 이미 승인된 별도 vertical slice이므로 이 batch에서 제외한다. batch는 79개의
element plan을 저장하며 Track A selected row 70개, Lance supplemental row 4개, Warlord selected
carrier 5개로 구성한다. 기존 Track A fail-closed/excluded 63개는 분모에서 삭제하지 않는다.

material disposition은 `TYPED_EXECUTION`, `ADMITTED_SOURCE_PROFILE`, `FAIL_CLOSED` 중 정확히
하나다. 초기 13-stage seam 시점에는 admitted source profile 16, fail-closed 63, typed execution 0이었고
Warlord canary 5개도 전부 fail-closed였다. 이 수치는 아래 full-source 101 transaction의 현재 분모가 아닌
역사적 seam 설명이다.

이후 기존 rollout의 101 target을 full-source occurrence 기준으로 다시 materialize했다. 현재 디스크에
보존한 baseline transaction과 receipt 수치는 다음과 같다.

- target documents: 101/101
- source Particle corpus / strict mapped / typed exclusion: 4,846 / 4,687 / 159
- Particle portable / source-preserved deferred: 4,488 / 199
- portable Particle product-full / fail-closed: 3,481 / 1,007
- output resource bindings / exact source bindings / receipt-proven supplemental bindings:
  8,872 / 8,509 / 363
- Mesh Particle carrier: 1,182, missing exact mesh-model blocker: 110
- source Decal joins: 79/79, source resource bindings 149/149
- Base-ready editable/playable Decal: 46
- missing-Base editable/fail-closed Decal: 33
- source AnimationTrail: 8 notify / 11 elements, placeholder 0
- `CHARACTER_AFTERIMAGE`: 72 byte-lossless cues / 29 target documents, receipt-only runtime-deferred
- output: 4,777 elements

모든 source binding은 slot/value가 그대로 출력되며 같은 slot의 기존 starter보다 항상 우선한다. supplemental
363개는 hash-pinned receipt가 같은 source texture의 Base alias를 증명한 경우에만 사용하고 Legacy starter lane은
복사하지 않는다. 모든 non-empty Element resource와 named texture runtime asset은 물리 파일 존재를 확인했다.

모든 target은 기존 제품 ID/path가 아니라 별도 `.unified` candidate ID/path를 사용한다. 물리 파일은
4직업 101개이며 Artist F까지 합치면 102개다. All Effects는 101개 candidate를 모두 stable ID/path로
index하고 Artist 31210 BA1의 두 occurrence 재사용까지 포함해 102개 UI appearance를 만든다. Open/Edit/Save는
101개 모두 가능하다. baseline Particle은 product-full 3,481, portable fail-closed 1,007,
source-preserved deferred 199로
분리되고 Decal은 ready 46 / authoring-incomplete 33이다. AnimationTrail 11은 별도 history family이며
`CHARACTER_AFTERIMAGE`는 Element로 노출하지 않는다. 기존 product document, EffectCatalog, 네 클래스
animevent mutation은 0이다. 따라서 Open/Edit 가능한 문서 수와 각 Element의 의미 있는 Play/Draw 가능 수를
동일시하지 않는다.

## 6. Valtan Whirlwind vertical slice

### 6.1 authoritative identity

- encounter pattern: `VALTAN_WHIRLWIND`
- source action: `420633` / `레이드 발탄_휠윈드`
- Korean name authority: main PR #103 (`28fa75a2`), source commit `72cc7629`,
  `Data/Animation/Reference/Valtan/Valtan.skilltiming`
- branch: stage 001 → 002 → 003
- active action: `valtan.attack.whirlwind.active`
- active clip: `mesh_att_battle_20_03`
- mapping key: `BOSS_VALTAN + VALTAN_WHIRLWIND + actionId`

Server gameplay의 windup, active, recovery, damage와 hit timing은
`Data/Encounters/Valtan/ValtanEncounter.json`에 남겼다. Effect mapping에 gameplay 수치를
복제하지 않았다.

### 6.2 source occurrence와 admission

- notify-004: AnimationTrail, 3 carriers, typed trail runtime 미폐쇄
- notify-005: Dust, 2 carriers, recipe는 portable이나 material/resource closure 미폐쇄
- notify-006: WWind, 3 carriers, Sprite 1 + Mesh 2 visible executable
- notify-009: Light, 1 carrier, external module/point-light runtime 미폐쇄
- notify-001~003 Effect, notify-007 PawnMaterialParam, notify-008 ViewShake는 별도 fail-closed

정량 분모는 first-LOD carrier 9, ordinary enabled recipe/executable 3/9, fail-closed 6/9다.
Trail/Dust/Light의 raw source identity와 recipe graph authority는 evidence로 보존하지만 ordinary v13
실행 recipe는 비활성화한다. source occurrence 4개 중 완전 visible은 notify-006 한 개다.

`B_EffectRoot`는 Valtan WModel의 실제 `b_effectroot`, bone index 83,
bone-name hash `57881f1a4fa28edf`로 exact join했다. silent root fallback은 허용하지 않는다.

현재 `productAdmission`은 `FAIL_CLOSED_NON_PRODUCT_CANARY`이며 EffectCatalog와 animation event
제품 mapping은 둘 다 false다.

### 6.3 Client timeline과 Model View

`ClientReplication`이 `serverTick`, `actionStartTick`, `patternSequence`, `stageIndex`, pattern/action
identity를 `CValtan`에 전달한다. `CValtan`은 wrap-safe 30 Hz age를 계산하고 같은 tuple에서는
시간을 뒤로 돌리지 않으며, 새 tuple에서 clip/cue cursor를 reset하고 과거 tuple을 격리한다.

Effect Tool은 preview-only `Boss_Valtan`을 Model View에서 선택할 수 있다. Whirlwind authored
document를 열면 boss-pattern effect mapping과 patternbindings를 함께 검증하고
`mesh_att_battle_20_03`을 시작한다. action, clip, model bone 중 하나라도 drift하면 현재 preview를
추측값으로 계속하지 않고 구체적 오류와 함께 fail-closed한다.

최신 boss mapping에 source evidence 필드가 추가된 뒤 C++ parser의 exact property set이 구버전으로
남아 있던 문제와, parse 실패 뒤 하네스가 빈 `Bindings.front()`를 접근하던 문제를 수정했다.
또한 headless harness가 `LOSTARK_RESOURCE_ROOT`를 설정하지 않아 존재하는 DDS를 missing으로
오판하던 테스트 환경을 고정했다.

## 7. Effect Tool과 runtime 성능 변경

### 7.1 Tool/UI

- Artist F tree draw에서 source/material 준비를 제거하고 명시 Load/refresh 경계로 이동했다.
- catalog revision별 실패 latch를 두어 같은 실패 preparation을 매 UI frame 재시도하지 않는다.
- numeric drag preview는 60 ms trailing debounce를 사용해 매 tick 전체 document stage를 막는다.
- unified document file poll과 validation은 revision/cache 경계를 사용한다.

정상 tree draw에서 source acquisition, material reconstruction과 전체 renderer preparation을 다시
호출하지 않는 계약을 유지한다.

### 7.2 Playback/renderer

- frame vector는 capacity를 유지하고 element world를 element당 한 번 계산한다.
- prepared lookup은 full JSON serialization 대신 document pointer/revision identity를 사용한다.
- particle/trail dynamic buffer를 prepared/shared pool에서 재사용해 action edge GPU allocation을
  제거한다.
- renderer의 매-frame string-key unordered map과 성공 status string rebuild를 제거했다.
- FOLLOW anchor sync와 normal advance 사이의 frame rebuild 중복을 제거했다.
- anchor map과 transform history의 반복 allocation/linear insertion을 줄였다.

### 7.3 aggregate budget

document validator의 total particle budget에 `SourceRecipe` carrier를 포함했다. scene runtime에는
effect, particle, mesh particle, trail, afterimage, light, post, weighted draw의 owner/world budget을
추가했다. remote cosmetic이 soft limit을 먼저 소비하면 suppress하고 local player와 boss telegraph
reserve를 보존한다.

이 변경은 admission과 CPU/GPU 폭주 방어다. MeshParticle GPU instancing, trail incremental upload,
thumbnail async decode, fixed-step catch-up 정책과 실제 GPU p95/p99 계측은 아직 남아 있다.

## 8. Server backpressure와 안정성

### 8.1 ingress

- room ingress hard cap 1024
- best-effort pressure band 768
- reliable band 960
- per-tick deterministic FIFO drain 256
- 같은 session의 MOVE와 같은 skill aim update는 reliable barrier를 넘지 않는 latest-wins coalescing
- 마지막 64 slots는 leave/rollback reserve

best-effort overload는 해당 row를 drop하고 계측하며, reliable overflow는 queue를 부분 변경하지 않고
해당 session만 fail-closed한다.

### 8.2 outbound

- session당 128 frames / 512 KiB hard cap
- snapshot band 112 frames / 384 KiB
- reliable reserve 16 frames / 128 KiB
- queued world snapshot은 latest-wins
- reliable frame은 FIFO
- room thread는 frame encode/enqueue까지만 수행하고 OS send를 하지 않음
- session sender thread만 blocking send를 수행하며 `SO_SNDTIMEO=250 ms`

ROOM_FULL 같은 terminal reliable frame은 queue drain 뒤 close한다. graceful flush 직후 hard stop이
추월할 때 receive와 sender가 모두 close callback을 놓칠 수 있던 race를 추가로 수정했고,
exactly-once callback 계약으로 고정했다.

현재 transport는 IOCP가 아니다. session마다 receive + sender OS thread를 사용하므로 4클라이언트는
8개의 session I/O thread를 쓴다. 다만 느린 한 client의 blocking send가 room tick이나 다른
session을 막는 구조는 제거했다. IOCP는 같은 bounded queue/session API 뒤의 후속 transport 교체다.

## 9. 자동 검증

실제로 실행한 검증만 기록한다.

현재 디스크의 baseline 101문서/4,777 element를 만들 때 실행한 검증은 다음과 같다.

- base source-material contract tests: 32/32 PASS
- four-class source-material compiler tests: 5/5 PASS
- four-class source-material compiler `--check`: PASS
- portable particle runtime capability tests: 12/12 PASS
- four-class candidate materializer tests: 20/20 PASS
- candidate materializer `--dry-run`: PASS
- candidate transaction write: PASS, 101 documents / 4,777 elements
- write 뒤 candidate materializer `--check`: PASS
- Valtan boss-pattern validator: PASS
- Valtan canary builder `--check`: PASS
- Valtan Python unit tests: 15/15 PASS
- ClientFrontendHarness x64 Debug compile/link: PASS

2026-08-15 admission/Lance/provenance 소묶음 checkpoint에서 추가로 실행한 검증은 다음과 같다.

- repo catalog UModel extraction: 953 requested / 905 resolved / 48 fail-closed / texture 3,571
- source evidence rebuild: 953 / parent props 901 / fail-closed 52 / missing extractor 0
- source-material contract write/check: seed 50 / compiled 776 / admitted 607 / blocked 165 /
  missing package 1, PASS
- Lance dynamic arithmetic + approximate focused Python: 39/39 PASS
- no-write partition: Full 2,793 + Approximate 722 + Hard 973 = portable 4,488, preview 3,515
- 3-way Track A cheap focused native gate: PASS
- portable event focused native gate: 5/5 PASS
- Effect publisher approximate reject/transaction rollback: PASS
- approximate codec canonical roundtrip/invalid flags rollback: 2/2 PASS
- latest ClientFrontendHarness x64 Debug compile/link: PASS

전체 `--effect-authoring-fast`는 공유 WIP의 기존 16항목 때문에 exit 1이고, default candidate pinned suite는
stale `EXPECTED_*` 때문에 의도적으로 실패한다. 위 focused PASS와 분리한다. Debug 빌드도 전체 101문서의
actual Draw 또는 사용자 visual PASS를 뜻하지 않는다.

현재 복원 diff에서 아직 실행하거나 확정하지 않은 항목은 다음과 같다.

- focused 교정이 반영된 DM T, Lance BA1, DimensionMaster BA3, Warlord 17090의 사용자 화면 재검증
- 새 `Boss Patterns > Valtan` row에서 420633을 사용자가 직접 open/play하는 UI 검증
- 방향 승인 뒤 final denominator/rebaseline, 101 rewrite/check와 전체 native Stage/Draw
- ClientFrontendHarness/Client x64 Release와 occurrence별 제품 승인

### 9.1 최종 gate 결과 기록 위치

아래 표는 실제 실행 결과를 얻은 뒤 같은 행에 명령, 분모와 실패 수를 기록한다. 현재 상태를 PASS로
선기록하지 않는다.

| gate | 현재 상태 | 기록할 증거 |
|---|---|---|
| Claude G3 integration | `PASS_AUTOMATED` | family/override schema, reimport ownership, focused 15/15 + Python 3/3 + pipeline + Client Debug |
| baseline representative visual | `PARTIAL_USER_PASS_AND_REJECT` | Artist F와 DM A 승인, DM T/Lance BA1/DM BA3/Warlord reject, Valtan UI blocked |
| final denominator/101 rewrite | `BLOCKED_BY_VISUAL_DIRECTION` | 승인된 Full/Approximate/Hard exact counts와 연속 check |
| 101 native ordinary Stage/Draw | `BLOCKED_BY_REWRITE` | lineage, attempted/submitted/suppressed/failed/committed |
| Client x64 Release | `PENDING` | compile/link exit와 output identity |

다음은 같은 RESULT에 보존하는 별도 Valtan/성능/Server 변경의 기존 자동 증거이며, 현재 101문서의
Stage/Draw 완료 증거로 사용하지 않는다.

- `ClientFrontendHarness --skill-binding-fast` 24/24 PASS 후 latest mapping 교차 계약 추가
- `ClientFrontendHarness --effect-runtime-fast` 24/24 PASS
- Server Debug temp build와 contract test PASS
- Server Release build와 contract test PASS
- graceful flush/hard-stop overtake 포함 Server contract test 10회 연속 failures 0
- Release 4-player Valtan network harness: initial 4/4, fifth ROOM_FULL, replacement reconvergence,
  second generation 4/4 PASS

자동 PASS는 source identity/materialization과 fail-closed 경계에 대한 것이며 화면의 색, 크기, 위치와
최종 visual fidelity를 승인한 것은 아니다.

## 10. 사용자 수동 검증 경로

에이전트는 Client/UI를 실행하거나 visual fidelity를 대신 판정하지 않았다.

Claude G3 코드 통합과 Client Debug 빌드는 완료됐다. 사용자가 **기존 101 baseline 대표 육안검증**을
수행했고 Artist F와 DimensionMaster A를 승인 control로 확정했다. 이후 DM T, Lance BA1,
DimensionMaster BA3, Warlord 17090과 Valtan index의 focused 교정·자동검증·Client Debug 재빌드까지
완료했다. 기존 사용자 REJECTED 관찰은 그대로 보존하며, 같은 occurrence의 사용자 재검증 전에는
어느 행도 visual PASS나 제품 승격으로 바꾸지 않는다.

Valtan 수동 검증의 목표 문서는
`레이드 발탄_휠윈드 | 420633 Active | Portable Canary`
(`effect.valtan.pattern.420633.active`)다. All Effects에 별도 `Boss Patterns > Valtan` staged tree를 연결했고
`420633 | VALTAN_WHIRLWIND | SPIN | mesh_att_battle_20_03` row가 exact ID/path로 한 번만 stage되는 것과
5개 invalid 입력 rollback을 non-UI harness로 고정했다. 이제 Model View의 `Boss_Valtan`,
`mesh_att_battle_20_03`과 notify-006 WWind Sprite 1 + Mesh 2를 사용자가 확인한다. Trail, Dust, Light는
계속 명시 fail-closed다.

4직업 `.unified` 후보는 `F1 -> Effect Tool -> All Effects -> class -> skill/clip -> Candidate -> Open for
Editing`에서 연다. `.unified` sibling은 `Saved Skill Draft (not published)`로 표시되고 기존 Legacy는
Advanced Migration Reference로 격리된다. baseline admitted row에서는 Model View, Play All, Family, Solo와
Transform/Color/DDS Save/Reload를 사용자가 확인한다. baseline Particle product-full 분모는 3,481이고
portable fail-closed 1,007과 source-preserved deferred 199는 켜거나 승인하지 않는다. AnimationTrail 11개는
Cascade Ribbon과 분리된 Trail family에서 history follow와 DDS/width/lifetime을 확인한다. missing-Base Decal
33개는 Base DDS를 먼저 지정하면 실행 잠금이 풀리고 이후 위치/회전/스케일/색을 튜닝한다.
Candidate가 존재한다는 사실은 visual PASS가 아니며 사용자가 확인한 뒤에만 product mapping을 전환한다.

### 10.1 육안 검증 ledger

사용자가 보고한 관찰만 occurrence 단위로 아래 표에 추가한다. 자동 Stage/probe PASS를 육안 승인으로
옮겨 적지 않는다. Effect Tool에서 저장한 변경은 조정 field와 Save/Reload 재확인 결과까지 함께 기록한다.

| 날짜 | 대상 occurrence | 현재 상태 | 사용자 관찰 | 적용한 조정 | Save/Reload 재확인 | 승인 |
|---|---|---|---|---|---|---|
| 2026-08-15 | 4직업 101 candidate / 102 UI occurrence | `STRUCTURAL_RESTORATION_BLOCKED` | DimensionMaster A에서 원본 검격 형상 미복원 확인 | 전수 원인 조사와 full-source 재복원 진행 | 미실행 | `VISUAL_GATE_REOPENED` |
| 2026-08-15 | DimensionMaster A / `effect.dimensionmaster.skill.2050210.unified` | `FAILED_USER_EYE_CHECK` | 24개 중 실제로 보이는 것은 `fm_m_trail` 계열 한 종류뿐이며 Sprite 4개는 Play 잠금 | 미적용 | 미실행 | `REJECTED` |
| 2026-08-15 | Artist F 31470 | `USER_CONTROL_RECHECKED` | 기존 복원 결과가 잘 나옴 | 공용 runtime/G3 변경 회귀 없음 | 해당 검증에서 별도 저장 변경 없음 | `USER_APPROVED` |
| 2026-08-15 | DimensionMaster A / `effect.dimensionmaster.skill.2050210.unified` | `USER_RECHECKED` | 보라색 원형 검격과 Sprite/Mesh 합성이 의미 있게 보임 | 공용 grouped UV·sprite flip·transform 교정 | 해당 검증에서 별도 저장 변경 없음 | `USER_APPROVED_ARTIST_F_LEVEL` |
| 2026-08-15 | DimensionMaster T / `effect.dimensionmaster.skill.2050500.unified` | `AUTOMATED_FIX_RECHECK_PENDING` | 이전 화면에서 summon 형상이 잘리고 방사형으로 찢어져 보임 | exact WModel SHA `f68fb4...1a3b`/30 tps 배포. 0초 isolation은 ModelCue 7,062 px, Elements 0 px로 소유자를 ModelCue로 확정하고 4 section/23-bone finite palette를 검증 | authored 저장 변경 없음; 사용자 재확인 대기 | `RECHECK_PENDING` |
| 2026-08-15 | LanceMaster 34010 BA1 / `ce43b71ae7ca3379dfe8529d` | `AUTOMATED_FIX_RECHECK_PENDING` | 이전 화면에서 exact resource가 있음에도 흰 arc로 포화됨 | legacy Valtan missile과 분리한 exact two-emissive evaluator가 5 lane, ParticleColor, strength/pow/UV 순서를 소비. WARP 9,677 px와 비백색 RGB chroma PASS | 101 저장·admission 변경 없음; 사용자 재확인 대기 | `RECHECK_PENDING` |
| 2026-08-15 | DimensionMaster 2050010 BA3 / `14e8104724f2a67e20f0ad90` | `AUTOMATED_FIX_RECHECK_PENDING` | 이전 화면에서 기대한 보라 glow 대신 검정·청록·녹색 ring | exact `maintex`/`uv_noise`, watertrail reflection/final tint, direct dissolve를 분리. WARP 23,872 px, dissolve=1, `B>R>G` PASS | 101 저장·admission 변경 없음; 사용자 재확인 대기 | `RECHECK_PENDING` |
| 2026-08-15 | Warlord 17090 / `effect.warlord.skill.17090.unified` | `AUTOMATED_BOUNDARY_RECHECK_PENDING` | 이전 화면에서 선택 spark Sprite만 beige spokes로 보이고 chain silhouette는 없음 | Mesh14 rotation strict projection, chain06 8 + chain07 4, same-group Base/FULL 제거. chain 12개는 explicit disabled `AUTHORING_APPROXIMATE`; Mesh witness 484 px PASS | 101 transaction rewrite 없음; 사용자 재확인 대기 | `RECHECK_PENDING` |
| 2026-08-15 | Valtan 420633 notify-006 WWind 3 | `UI_INDEX_FIX_RECHECK_PENDING` | 이전 화면에서는 All Effects에서 선택할 수 없었음 | 별도 `Boss Patterns > Valtan` staged tree, exact 420633 once, existing load/play 재사용, 5개 invalid rollback PASS | UI 실행 안 함; 사용자 최초 확인 대기 | `NOT_VISUALLY_REVIEWED` |
| 2026-08-15 | DimensionMaster T / `effect.dimensionmaster.skill.2050500.unified` | `ALTERNATE_BIND_FIX_RECHECK_PENDING` | 30-tps asset 뒤에도 화면 전체를 덮는 금·회색 동심원/방사형 skeletal 형상이 그대로이며 해결되지 않음 | active Action frame이 WSKL rest로 잘못 bake되어 WMSH inverse bind와 최대 `90.678497` 어긋난 원인을 확정. Action detach + POSE bind snapshot으로 recook한 SHA `87186351...22b6`을 배포하고 normalized bind identity `5.68e-14`, 19 moving clock bones, source/runtime CPU skin bounds를 고정 | authored 저장 변경 없음; 새 WModel 사용자 재확인 대기 | `RECHECK_PENDING_AFTER_ALTERNATE_FIX` |
| 2026-08-15 | LanceMaster 34010 BA1 / `ce43b71ae7ca3379dfe8529d` | `USER_RECHECKED_AFTER_TYPED_CHROMA` | 흰색 포화가 사라지고 의도한 청색/청록 trail 형상이 보임 | strict two-emissive evaluator와 legacy Valtan missile 분리 상태를 보존 | 해당 검증에서 저장 변경 없음 | `USER_APPROVED` |
| 2026-08-15 | DimensionMaster 2050010 BA3 composite | `ALTERNATE_COMPOSITE_FIX_RECHECK_PENDING` | fragment Sprite는 의도한 particle 형태. `002.dds`는 검은 사각형, 다른 Sprite DDS 2개는 미출력이며 Mesh Particle은 원작과 다른 검정·녹색·청색 ring | 단일 water fixture를 폐기하고 WATERTRAIL/LINEARFLOW/MAKEFLOW/RING/PARTICLETRAIL exact family와 hidden DDS를 연결. zero emission/back color의 final-tint 오용을 제거하고 EPAL_Z one-sided 앞면 부호를 교정해 ordinary Mesh4/Sprite4 전부 nonzero RGB 및 composite `B>R>G`를 고정 | 101 저장·admission 변경 없음; 사용자 재확인 대기 | `RECHECK_PENDING_AFTER_ALTERNATE_FIX` |
| 2026-08-15 | Warlord 17090 / `effect.warlord.skill.17090.unified` | `SOURCE_BOUNDARY_RECHECK_PENDING` | 흰색 방사형 선만 보이며 원작의 창끝 4개 + 사슬 4방향 발사·회수 동작과 다름 | `fx_d_grid_016.dds + effect.standard`의 허위 exact 투영을 제거했다. `atypical_028`은 non-exact G3 baseline으로만 유지하며 chain06/07 12개는 `SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE` / Approximate다. GPU witness는 chain06+07 geometry/motion만 검증한다 | Product mapping 제외; 사용자 재승인 전 FULL/exact 주장 금지 | `RECHECK_PENDING_AFTER_SOURCE_BOUNDARY_FIX` |
| 2026-08-15 | Valtan 420633 All Effects owner selection | `OWNER_SELECTOR_FIX_RECHECK_PENDING` | 상단 Character 선택에는 여섯 playable class만 있고 Valtan을 같은 흐름에서 선택해 skill/Open/Play All/Solo로 들어갈 수 없음 | `CHARACTER_CLASS`/PlayerSkills를 바꾸지 않고 상단을 UI-only `Character / Boss` 7-option owner selector로 교체. Valtan 선택 시 exact 420633을 기존 unified tree의 Open/Play All/Family/Solo 흐름으로 직접 노출하고 six-player owner 수와 invalid rollback을 고정 | UI 실행 안 함; 사용자 재확인 대기 | `RECHECK_PENDING_AFTER_UI_FIX` |

사용자가 제공한 원본 스크린샷 5장은 다음 폴더에 byte 그대로 보존했다.

- [`01_dimensionmaster_2050210_a_user_pass.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/01_dimensionmaster_2050210_a_user_pass.png), SHA-256 `a86fac6aaafdff29013f43e87728ac2233c47484de7e9a78e134a17b1c1312dd`
- [`02_dimensionmaster_2050500_t_clipped.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/02_dimensionmaster_2050500_t_clipped.png), SHA-256 `7ba5183e04c474def6558d2c26acf9e2e956a6af2969f2f7e6d3115a6150e0c9`
- [`03_lancemaster_34010_ba1_white_arc.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/03_lancemaster_34010_ba1_white_arc.png), SHA-256 `6a929e142c26e55f04169bb4cc23dda1d1c951bdd7360652b4f72d9ac2b6182b`
- [`04_dimensionmaster_2050010_ba3_color_mismatch.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/04_dimensionmaster_2050010_ba3_color_mismatch.png), SHA-256 `a9037ea9ddaf0155ca3392f4584a68cff0b07838047cdf9294b76ce9f9cb9ef1`
- [`05_warlord_17090_chain_missing.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/05_warlord_17090_chain_missing.png), SHA-256 `c2ef873b086bb7908956d995865bb3b2ed7845a8ca8e8a38270e07808e6abfee`
- [`06_dimensionmaster_2050500_t_still_broken_after_retime.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/06_dimensionmaster_2050500_t_still_broken_after_retime.png), SHA-256 `3cf9c0922a2b5f6afa5901fe4032c5ec2ed86fae8b7b61fa9e2f42a46155bfda`
- [`07_lancemaster_34010_ba1_user_pass_after_typed_chroma.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/07_lancemaster_34010_ba1_user_pass_after_typed_chroma.png), SHA-256 `f4296f365b4b2ae66684fc260b33eca9eb28e62c027d4565bcef85fa7da2eb64`
- [`08_dimensionmaster_2050010_ba3_sprite_mesh_still_broken.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/08_dimensionmaster_2050010_ba3_sprite_mesh_still_broken.png), SHA-256 `1e229a7b7e51c0591078752f968d2e269c8fddd72194025d5d9cc281139a0780`
- [`09_warlord_17090_four_chain_still_missing.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/09_warlord_17090_four_chain_still_missing.png), SHA-256 `fa52de85b941a498bec04f31367bf2a8552e7e183fc76bc40ba7f8c56a64d85a`
- [`10_valtan_420633_all_effects_selector_missing.png`](../08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/10_valtan_420633_all_effects_selector_missing.png), SHA-256 `b97ecf7f9de0b5f2abe41782e37c3aeebd56265866d7aaa7d54981514e5f430c`

다음 항목은 별도 상태로 유지한다.

- baseline Particle source-preserved deferred 199와 portable fail-closed 1,007: 이번 대표 육안 승인 대상 아님
- missing-Base Decal 33: `AUTHORING_INCOMPLETE_LOCKED`, Base DDS 지정 뒤 새 ledger row로 전환
- AnimationTrail 11: exact history family materialization 완료, 전체 native Stage/Draw 뒤 사용자 검증 대기
- `CHARACTER_AFTERIMAGE` 72/29: receipt-only runtime-deferred, generic Trail로 재생하거나 승인하지 않음
- Cascade Ribbon, Light, Camera/ScreenPost, Valtan Dust: 이번 검증에서 보류

### 10.2 2026-08-15 자동 gate 범위와 남은 육안 경계

사용자 화면으로 다음 자동 검증 공백이 확인됐다.

- `Stage_Document` 성공은 각 Element가 실제 픽셀을 제출한다는 증거가 아니다.
- `Query_ParticleRuntimeProbe` 존재는 WModel별 geometry draw, Sprite draw, material opacity를 증명하지 않는다.
- 최초 candidate의 partial source 분모는 폐기하고 full source Particle 4,846을 strict mapped 4,687 + typed
  exclusion 159로 다시 고정했다.
- 공용 module/material compiler와 DimensionMaster-only canonical join을 적용한 baseline의 portable Particle
  fail-closed는 1,007이다. 현재 no-write projection은 이를 Approximate 722와 Hard 973으로 다시 분류하지만
  아직 authored 문서에 반영하지 않았다.
- materialization write/check는 각 Element의 실제 GPU draw submission을 증명하지 않는다.
- AnimationTrail 11은 별도 history family로 materialize했지만 전체 native Stage/Draw와 사용자 trajectory
  확인은 아직 남아 있다.
- DM T의 첫 30-tps asset `f68fb427...f1a3b`은 retime만 정확한 중간 증거였고, 사용자 화면으로 bind-pose
  결함이 남았음을 확인했다. active Action frame을 WSKL rest로 bake한 기존 경로와 PSK-derived WMSH inverse
  bind의 최대 오차 `90.678497`을 fail-closed로 잡고 SHA `87186351...22b6`으로 recook했다. 새 asset의
  normalized bind identity 오차는 `5.68e-14`, 두 clip 30 tps/19 moving clock bones와 source/runtime CPU skin
  bounds는 일치한다. 후반 2.90142초 material-param 미지원은 이 초기 geometry 결함과 계속 분리한다.
- Lance BA1 strict evaluator는 기존 Valtan legacy missile profile을 바꾸지 않고 exact 5-lane material,
  ParticleColor, strength/pow/UV 순서를 소비한다. GPU chroma witness는 흰 DDS/무색 포화 회귀를 막지만
  사용자가 보는 arc의 크기·색·타이밍 승인은 아니다.
- DimensionMaster BA3는 단일 water row가 아니라 실제 8개 occurrence를 ordinary 문서에서 각각 stage/draw한다.
  exact WATERTRAIL/LINEARFLOW/MAKEFLOW/RING/PARTICLETRAIL family와 hidden DDS lane을 연결하고, `trail_002`
  zero emission 및 makeflow back color를 final tint로 쓰던 축약과 EPAL_Z one-sided 앞면 부호를 교정했다.
  최종 GPU 픽셀은 `23872/5070/7016/18407/8946/23840/1307/10`, Mesh4/Sprite4 composite와 typed 7 chroma
  PASS다. 없는 emissive DDS는 추가하지 않았고 101/disk admission은 바꾸지 않았다.
- Warlord 17090은 `fx_d_grid_016.dds`가 parent 9개 reference 중 하나일 뿐 Base/opacity exact input이라는
  근거가 없고 alpha가 전부 255이며 opacityMask/WPO output과 texture parameter mapping이 cooked-null임을
  확인했다. 따라서 `grid_016 + effect.standard`의 허위 exact 투영은 제거했다. `atypical_028`은 명시적
  non-exact G3 baseline으로만 남고 chain06/07 12개 reason은
  `SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE`다. 자동 GPU witness는 geometry/motion을 검증할 뿐 material
  exactness나 Product admission 근거가 아니며 사용자 재승인 전 Product mapping에서 제외한다.
- Valtan authored 문서와 All Effects 소유자 선택을 분리했다. playable enum/count는 그대로 두고 상단
  `Character / Boss` selector에 UI-only Valtan 한 항목을 추가해 exact 420633을 기존 unified tree의
  Open/Play All/Family/Solo 경로로 보낸다. 자동 fixture는 7 owner option, player six 불변, exact row once와
  duplicate/path escape/mismatch/missing/corrupt rollback을 증명하며 실제 UI 조작은 사용자 확인이 필요하다.

최종 자동 checkpoint는 production `Data`/`Resources` root를 명시해 다음과 같이 닫았다.

- materializer strict typed/Warlord/G3 ownership focused `8/8 PASS` (`0.084s`)
- `Test-EffectPipeline.ps1` PASS (`productMutation=false`, sidecar 13 programs/135 rows)
- `--effect-g3-authoring-fast` `15/15 PASS`, `--effect-runtime-fast` `failures: 0`
- `--effect-dm-t-isolation-fast` 새 SHA `87186351...22b6`, `failures: 0`
- `--effect-rejected-slices-fast` DM BA3 8/8 + Lance regression + Warlord four-direction return + Valtan owner/rollback,
  `failures: 0`
- Client x64 Debug full shader compile/link PASS. 첫 시도는 Valtan selector 교체 뒤 legacy source-preset loop가
  삭제된 지역 `Classes`를 참조해 compile FAIL했고, 같은 seven-owner 정본에서 player option만 순회하도록
  교정한 재실행이 `Client/Bin/Debug/Client.exe` link까지 PASS했다. Client/UI는 실행하지 않았다.

따라서 현재 자동 증거는 full-source materialization, exact resource identity, admission 소묶음과 focused
runtime fixture까지다. Element별 draw readiness 또는 원본 fidelity 완료 증거로 확대하지 않는다. 이번 순서는
G3 통합 뒤 baseline 대표 육안검증을 먼저 하고, 그 결과가 방향을 지지할 때 101 rewrite와 full gate를 수행한다.

### 10.3 다음 사용자 수동 재검증 경로

`Framework.sln`의 `Server + Client` profile을 `Ctrl+F5`로 시작한 뒤 Client에서 `F1 > Effect Tool >
All Effects`로 들어간다. 에이전트는 이 UI를 실행하거나 화면을 대신 판정하지 않았다.

1. DM T: `Character / Boss = Dimension Master`, 검색 `2050500`, `T` skill의 Candidate를 열고
   `Open for Editing > Play All`. 이어 `Model / Summon > Solo`와 Elements family를 따로 재생해 새 summon이
   동심원 cage 없이 정상 skeleton/animation으로 보이는지 확인한다.
2. DM BA3: `Character / Boss = Dimension Master`, 검색 `2050010`, LMB의 BA3 Candidate를
   `Open for Editing`. `Sprite Particle (4)`와 `Mesh Particle (4)`를 Family/Solo로 모두 보고 마지막에
   `Play All`. fragment 유지, `trail_002` 검은 사각형 제거, 두 particle trail 출력, 보라·청색 composite를 본다.
3. Warlord: `Character / Boss = Warlord`, 검색 `17090`, `A` Candidate를 `Open for Editing`.
   `Mesh Particle (14)`에서 chain06/07을 Family/Solo로 확인한 뒤 `Play All`. 창끝 4개와 사슬이 네 방향으로
   발사됐다 회수되는지 본다. WPO 원식 미확보 때문에 이 preview는 계속 Approximate/product-blocked다.
4. Valtan: `Character / Boss = Valtan`, 검색 `420633`, `420633 | VALTAN_WHIRLWIND | SPIN |
   mesh_att_battle_20_03` row에서 `Open for Editing > Play All`. Current Effect의 Sprite/Mesh Family와 Solo도
   같은 row를 재사용하는지 확인한다.

Lance 34010 BA1은 사용자가 이미 승인했으므로 다시 요청하지 않고 자동 chroma regression만 유지한다.

## 11. 남은 경계

- 교정 Client에서 새 bind-pose DM T, DimensionMaster BA3 composite, 새 carrier/motion Warlord 17090의
  동일 occurrence 사용자 수동 재검증. Lance BA1은 사용자 승인 상태를 보존하고 자동 회귀만 유지
- 상단 `Character / Boss > Valtan`의 exact 420633 row 노출·Open/Play All/Family/Solo 사용자 확인
- Warlord chain은 typed masked/WPO executor와 사용자 승인이 생기기 전 disabled authoring approximate 유지
- 대표 결과 승인 뒤 final denominator rebaseline, 101 transaction rewrite와 전체 native ordinary Stage/Draw
- ClientFrontendHarness/Client x64 Debug·Release와 publisher rollback 검증
- strict mapped Particle 4,687 중 source-preserved deferred 199의 후속 typed runtime. 이 중 Cascade
  `TypeDataRibbon`은 이번 범위에서 계속 보류한다.
- no-write hard portable fail-closed 973의 missing drawable/WModel, non-compiled/fallback/resource-contract
  사유를 G4 family/evidence/runtime 축으로 계속 분리
- 나머지 fail-closed row의 missing exact mesh/drawable, non-exact texture/material과 resource blocker는 exact
  evidence가 생기기 전 그대로 잠금
- Warlord를 포함해 각 source receipt에 실재하는 parent/texture/scalar/vector/dynamic evidence만 사용하고,
  DimensionMaster canonical profile 784개를 다른 캐릭터에 추정 복사하지 않는 계약 유지
- Decal 79 중 Base-ready 46과 authoring-incomplete 33의 사용자 저장·재import·실행 승격 검증
- AnimationTrail 11의 전체 native Stage/Draw와 사용자 trajectory/width/lifetime 튜닝 검증. Cascade Ribbon은
  이 family와 분리해 계속 보류한다.
- `CHARACTER_AFTERIMAGE` 72/29의 body/equipment pose snapshot과 ghost material runtime
- Valtan AnimationTrail history 연결, Dust material/resource, Light와 unresolved Effect/PawnMaterialParam/ViewShake
- frozen import batch builder의 byte-current `--check`는 candidate 생성 전 baseline/CRLF raw hash를 소유해
  현재 stale다. Effect Tool은 이 파일의 101개 stable target/path index를 사용하되 실제 저장 파일 identity를
  다시 검증한다. active materializer receipt/check와 runtime gate가 현재 후보 내용의 실행 정본이다.
- Valtan product catalog/mapping과 실전 4-client authoritative-age visual 승인
- MeshParticle instancing, trail ring/incremental upload, thumbnail async pipeline
- GPU/CPU p95/p99, frame allocation, draw/upload, UI stat count의 실측 telemetry
- bounded session API 뒤의 IOCP/overlapped transport 교체
- 실행 중인 Debug Server가 이전 binary이면 사용자 재시작 후 새 queue/metrics 정책 적용

따라서 현재 완료로 유지할 수 있는 것은 101개/4,777 elements의 strict source occurrence 재결합,
공용 non-Cascade module/material compiler, exact AnimationTrail 11, Decal/`CHARACTER_AFTERIMAGE` 보존 계약과
product mapping 0/Legacy 유지 경계다. baseline portable fail-closed 1,007과 deferred 199는 현재 디스크의
비교 입력이며, no-write projection의 Approximate 722/Hard 973은 아직 authored 정본이 아니다.
Element별 actual draw와 사용자 visual 승인은 아직 완료가 아니다. 승인 단위 product mapping/Legacy
zero-reference 제거, Valtan Whirlwind 제품 승인과 4-client 60 FPS도 완료로 기록하지 않는다.

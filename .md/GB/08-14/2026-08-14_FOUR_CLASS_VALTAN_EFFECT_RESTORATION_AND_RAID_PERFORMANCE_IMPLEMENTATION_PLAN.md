# 2026-08-14 4캐릭터 101/101·발탄 휠윈드 authored Effect 구현 계획

작업 시작일은 2026-08-14이며 마지막 갱신일은 2026-08-15다. 같은 복원 작업의 범위, gate,
family 경계와 제품 전환 순서는 이 문서를 계속 정본으로 사용한다. 날짜가 바뀌었다는 이유만으로
같은 내용을 복제한 일일 PLAN을 만들지 않는다.

문서 역할은 다음처럼 고정한다.

- 이 PLAN: 아직 남은 구현 범위와 완료 gate의 정본
- 대응 `2026-08-14_FOUR_CLASS_VALTAN_EFFECT_RESTORATION_AND_RAID_PERFORMANCE_RESULT.md`:
  실제 구현 상태, 자동 검증, 사용자 육안 검증 기록의 정본
- `Data/Effects/AuthoredCorrections/Generated/FourClassCombat.track-a-restoration-receipt.json`:
  101 candidate와 recipe/resource 분모의 기계 판독 정본
- `Data/Effects/Authored/*.unified.effect.json`: Effect Tool이 편집하고 ordinary runtime이 소비하는 저작 정본

사용자 눈검증 중 발견되는 위치, 크기, 색, DDS, timing 문제와 재검증 결과는 대응 RESULT의 수동 검증
ledger에 occurrence 단위로 누적한다. 승인 단위 product mapping/Legacy 제거처럼 독립적으로 구현·검증할
새 수직 슬라이스가 시작될 때만 2026-08-15 이후의 별도 PLAN/RESULT를 만든다. AnimationTrail history
runtime은 이번 복원 계획의 필수 family로 포함한다.

## 현재 승인된 실행 순서

2026-08-15 Codex/Claude 인계 뒤의 순서는 다음으로 고정한다.

```text
Codex admission/Lance/provenance 소묶음 마감
  -> 중지 (101 rewrite / EXPECTED rebaseline / full gate 금지)
  -> Claude G3 generic authoring family + authoringOverrides 코드 통합
  -> Client x64 Debug 빌드
  -> 기존 101 baseline 대표 스킬 사용자 육안검증
  -> 방향 승인
  -> final denominator 한 번 재기준화
  -> 101 transaction rewrite/check
  -> FULL+APPROX Stage/Draw, HARD reject/rollback full gate
  -> Debug/Release/publisher와 사용자 occurrence 승인
```

기존 baseline은 Particle portable 4,488 = product-full 3,481 + fail-closed 1,007이며
`authoringApproximate=0`이다. 현재 no-write projection은 product-full 2,793 + authoring-approximate 722 +
hard fail-closed 973 = portable 4,488, preview target 3,515다. 이 projection은 아직 PLAN의 최종 종료 분모가
아니며 디스크 101문서에 쓰지 않는다.

G3 구현 정본은
[`2026-08-15_G3_GENERIC_AUTHORING_FAMILY_AND_OWNERSHIP_PLAN.md`](../08-15/2026-08-15_G3_GENERIC_AUTHORING_FAMILY_AND_OWNERSHIP_PLAN.md),
전체 조사·인계 정본은
[`2026-08-15_AUTHORING_PRODUCT_GATE_SPLIT_CODEX_HANDOFF.md`](../08-15/2026-08-15_AUTHORING_PRODUCT_GATE_SPLIT_CODEX_HANDOFF.md)다.
G3 통합은 완료됐지만 101 rewrite와 최종 denominator 재기준화는 아직 하지 않았다. 과거
selected/provisional admission 수치를 새 baseline으로 해석하지 않는다.

## 0. 2026-08-15 사용자 화면 검증에 따른 gate 교정

DimensionMaster A `effect.dimensionmaster.skill.2050210.unified` 화면 검증에서 후보가 검격 형상을
복원하지 못하고 일부 Mesh 한 종류만 보이며 Sprite family가 전부 잠긴 사실이 확인됐다. 따라서 기존
`101/101 DOCUMENT_READY`와 ordinary Stage/probe PASS를 사용자 눈검증 준비 완료로 해석한 판정을 철회한다.

최초 101 candidate는 과거 Legacy approximation이 선택한 Particle 일부만 승격했다. 이후 source
event/clip을 다시 결합하고 공용 module/material compiler와 exact family materializer를 적용해 현재 디스크의
101문서를 transaction write했다. 현재 출력은 Particle 4,687 + Decal 79 + source-backed AnimationTrail 11,
합계 4,777 elements다. 원본 Particle 4,846 중 4,687개가 현재 101 occurrence에 정확히 귀속되며, 159개는
target 없음·event/clip 범위 밖·no-event로 typed exclusion receipt에 남는다.

현재 Particle 4,687은 portable 4,488 / source-preserved deferred 199로 분리된다. 디스크 baseline의
portable은 product-full 3,481 / fail-closed 1,007이고 `authoringApproximate=0`이다. Wave 0 material evidence와
strict approximate/Lance arithmetic 경계를 읽는 no-write projection은 product-full 2,793 /
authoring-approximate 722 / hard fail-closed 973이다. 어느 쪽도 `effect.standard` white fallback을 제품
admission으로 사용하지 않는다. DimensionMaster canonical sibling material graft 784개만 stable ID로 결합했고
Artist/Lance/Warlord에는 DimensionMaster profile을 추정 복사하지 않았다. 각 직업은 자신의 exact
package/material/texture receipt와 typed evaluator만 사용한다.

Decal 79개는 Base-ready 46개와 사용자가 diffuse를 지정해야 하는 authoring-incomplete 33개로 분리된다.
AnimationTrail은 8개 source notify의 11개 element를 정확한 source event/emitter identity와
`b_weapon_rhand` history 계약으로 복원했으며 Cascade `TypeDataRibbon`은 이 family에 들어오지 않는다.
`TrailGhostEffect` 72 occurrence / 29 target document는 generic Trail로 근사하지 않고
`CHARACTER_AFTERIMAGE` byte-lossless receipt ledger로 보존했으며, body/equipment pose snapshot과 ghost material
runtime이 생길 때까지 실행은 deferred다.

기존 baseline transaction write와 check는 완료했지만 no-write projection은 아직 재기준화·write하지 않았다.
Claude G3 통합과 Client Debug 빌드는 완료됐다. baseline 대표 육안검증에서는 Artist F와 DimensionMaster A만
사용자 승인됐고 나머지 네 스킬과 Valtan index가 재교정 대상으로 확정됐다. 최종 101 native Stage/Draw와
Client Release는 그 교정·사용자 재검증 뒤에 남아 있다.
Hard 973은 안전한 admission 완화가 없는 compiler/evidence blocker의 projection 상태다.

교정된 복원 목표는 다음과 같다.

- 원본 Particle 4,846을 `strict mapped 4,687 + typed exclusion 159`로 분해하고, 4,687개를 stable source
  occurrence identity로 materialize한다.
- Particle은 `portable 4,488 + source-preserved deferred 199`를 유지한다. 최종 Full/Approximate/Hard 분모는
  G3 통합과 baseline 육안검증 뒤 한 번만 고정하고 안전한 typed evaluator/resource 증거가 없는 행을 임의로
  열지 않는다.
- Decal 79개는 Base-ready 46 + authoring-incomplete 33의 저작 계약을 유지한다.
- Light 218과 ScreenPost 89는 사용자 결정대로 product admission을 보류하되 source evidence를 삭제하지 않는다.
- Cascade `TypeDataRibbon`은 보류한다. AnimationTrail은 별도 weapon bone/socket history family로 이번
  작업에서 source notify 8 / element 11로 구현하며 Cascade Ribbon으로 낮추지 않는다.
- `CHARACTER_AFTERIMAGE` 72 occurrence / 29 target은 byte-lossless ledger로 보존하고 character pose ghost
  runtime 전에는 generic Trail element로 투영하지 않는다.
- 도화가 F에서 별도로 닫았던 geometry pre-scale/basis, typed material execution, shader opcode와 실제 draw
  검증을 4직업 공용 compiler/runtime 계약으로 확장한다.
- material compiler는 캐릭터 이름을 기준으로 profile을 복사하지 않는다. material family의 불변 shader
  semantics와 occurrence별 DDS/scalar/vector/dynamic parameter를 분리하고, 각 source receipt의 exact evidence만
  결합한다.
- 완료 gate는 codec load나 Particle probe 존재가 아니라 각 admitted Element의 WModel/Sprite draw submission,
  non-zero visual envelope, material lane과 source occurrence 재생을 포함한다.
- 이 교정과 사용자 재검증 전까지 product mapping은 0, Legacy는 제품 정본으로 유지한다.

## 1. 이번 작업의 고정 목표

Artist F에서 사용자가 승인한 단일 authored Effect 흐름을 Artist, DimensionMaster,
LanceMaster, Warlord의 현재 combat intake 전체로 확장한다.

이번 작업의 첫 번째 물리 완료 분모는 다음과 같다.

- 4직업 combat intake: `51 skills / 74 stages / 113 clip occurrences`
- visual occurrence: `102`
- intentional silent 또는 no-carrier occurrence: `11`
- 4직업 고유 `.unified` authored 후보: `101/101`
- 별도 승인·제품 연결된 Artist F `.unified`: `1`
- 작업 종료 시 물리 `.unified` 목표: `102`

작업 시작 시 4직업 후보는 `14/101`이었다. Track A seam 13개는 v13이고 기존
`effect.dimensionmaster.skill.2050500.unified` 한 개는 v12였다. 기존 combat source intake의
원본 emitter graph를 101개 clip occurrence로 다시 materialize하여 `SourceRecipe`와 실제 DDS/WModel
binding을 보존한 v13 후보 `101/101`을 만든다. Legacy 평탄화 문서를 새 후보의 source로 사용하지 않는다.

발탄은 `VALTAN_WHIRLWIND` action `420633` / `레이드 발탄_휠윈드`의 active stage를 같은 authored v13,
ordinary `CEffectObject::Stage_Document` 경로로 구현한다. 이번 휠윈드의 실행 완료 분모는
메인 WWind 3 carriers다. Dust 2, Light 1과 ScreenPost는 보류한다. Cascade Ribbon과
AnimationTrail을 같은 family로 취급하지 않는다. Cascade Ribbon compiler는 보류하지만,
AnimationTrail은 4캐릭터 육안 검증 전에 닫을 이번 작업의 필수 family로 유지한다.

## 2. 사용자가 확정한 제품 방향

```text
Legacy JSON / Track A / 원본 추출 evidence
              ↓ 한 번 import 또는 의미 보존 migration
새 authored v13 .effect.json
              ↓ Effect Tool 편집·저장
ordinary CEffectObject::Stage_Document
              ↓ 사용자 visual 승인
catalog + animation/boss-pattern mapping 전환
```

- 정상 UI의 편집·선택 대상은 `.unified` authored Effect다.
- Legacy와 Track A는 Advanced Migration Reference와 rollback 근거다.
- 동일 Effect에 두 renderer나 두 runtime authority를 운영하지 않는다.
- 101개 모든 row는 기존 source intake에서 원본 `SourceRecipe`, DDS, WModel identity를 authored 데이터로
  이식한다. 지원하지 않는 module/material/family는 다른 모양으로 근사하지 않고 data를 보존한 채 fail-closed 한다.
- resource merge는 source slot/value가 항상 우선한다. source에 해당 slot이 없는 경우에만 기존
  standalone starter를 supplemental binding으로 유지하여 사용자가 diffuse를 교체할 수 있게 한다.
- Effect Tool에서 사용자가 조정하는 `Visible`, `Detail.Transform`, 색/알파와 Decal diffuse는 reimport 때
  덮어쓰지 않는다. compiler가 갱신하는 필드는 `SourceRecipe`, source-selected WModel과 non-Decal DDS/material이다.
- Legacy는 후보 생성의 입력이 아니라 제품 전환 전 rollback/reference로만 유지한다.
- 제품 mapping은 candidate 생성과 분리한다. 사용자가 승인한 occurrence만 전환한다.
- canonical/SHA/receipt/harness 확대는 기능 목표가 아니다. 기존 사용자 파일 보호, 원자 rollback,
  필수 build처럼 저장소 계약에 필요한 최소 검증만 유지한다.

## 3. 이번 범위와 제외 범위

### 3.1 이번 구현 범위

- 현재 combat intake 51 skills/74 stages의 101개 `.unified` 후보
- 기존 executable Track A 12 BA stage의 authored materialization
- Warlord 17000 BA1의 editable/fail-closed canary 보존
- 기존 4직업 source stage manifest와 imported document를 이용한 101개 exact source materialization
- Artist F에서 증명한 ordinary authored Particle/Mesh/Sprite 재생 경로 재사용
- source Decal의 exact recipe/resource와 DDS, Visible, Transform, Detail 편집 경로 보존. Base가 없는
  Decal은 editable fail-closed draft로 만들고 사용자가 Base diffuse를 지정한 뒤에만 실행 admission
- source-backed AnimationTrail identity, weapon bone/socket history sampling, authored width/lifetime/DDS와
  Effect Tool 편집·저장·개별 재생
- `TrailGhostEffect` raw notify payload와 source identity를 `CHARACTER_AFTERIMAGE` receipt ledger로
  byte-lossless 보존하고 generic Trail 투영을 차단하는 경계
- Valtan Whirlwind 메인 WWind 3 carriers
- 101개 후보를 All Effects의 class/skill/clip과 연결하는 Open/Edit/Save 및 Model View 동기화
- visual 승인 뒤 사용할 catalog+animevent 원자 승격 경계

### 3.2 이번 구현에서 제외

- Cascade Ribbon source graph portable compiler
- Cascade Ribbon의 native-exact 복원 및 제품 완료 판정
- Light 15, Camera/ScreenPost 2와 Valtan Light 1
- PawnMaterialParam, ViewShake와 unresolved generic Effect notify
- `CHARACTER_AFTERIMAGE` body/equipment pose snapshot과 ghost material 실행 runtime
- PlayerSkills 전체 67개 중 현재 combat intake 밖의 이동기·스탠스 16개
- MeshParticle GPU instancing, Trail incremental upload, thumbnail background decode 등 새 성능 작업
- IOCP transport 교체

Ribbon/Trail schema와 기존 slot/data는 제거하지 않는다. Cascade `TypeDataRibbon` source graph는
fail-closed data로 보존한다. AnimationTrail은 무기 bone/socket history를 소비하는 별도 family/계약으로
이번 작업에서 복원하며 Cascade carrier로 낮추지 않는다. Light도 source evidence와 fail-closed 상태를 보존하지만 이번
candidate/product 완료를 막는 필수 family로 계산하지 않는다.

## 4. 완료 상태의 다섯 gate

`복원`이라는 한 단어로 다음 상태를 합치지 않는다.

1. **DOCUMENT_READY**: v13 `.unified` 문서가 존재하고 ordinary codec/Stage_Document가 읽는다.
2. **SOURCE_ADMITTED**: Track A source recipe/material/family/attachment 의미가 실행 가능하다.
3. **VISUAL_APPROVED**: 사용자가 실제 clip occurrence를 확인해 서면 승인했다.
4. **PRODUCT_MAPPED**: catalog와 animevent 또는 boss-pattern mapping이 새 asset을 가리킨다.
5. **PERFORMANCE_APPROVED**: 정한 4클라이언트 환경에서 Client 60 FPS와 Server 30 Hz를 확인했다.

101/101은 첫 번째 `DOCUMENT_READY` 목표다. 모든 후보가 source identity를 가져야 하지만, 현재 runtime이
해석하지 못하는 module/material/Cascade family는 개별 carrier를 fail-closed 한다. 따라서 candidate 파일
존재와 exact source binding만으로 `SOURCE_ADMITTED` 또는 visual fidelity를 주장하지 않는다.

## 5. G01 101/101 candidate materialization

### 대상 파일

- `Tools/EffectPipeline/build_track_a_authored_import_batch.py`
- `Tools/EffectPipeline/Schemas/lostark.effect-authored-import-batch.schema.json`
- `Data/Effects/AuthoredCorrections/Generated/FourClassCombat.track-a-authored-import-batch.json`
- `Tools/ClientFrontendHarness/Private/FourClassTrackAAuthoredMaterializer.h`
- `Tools/ClientFrontendHarness/Private/FourClassTrackAAuthoredMaterializer.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`

### 구현 계약

- 입력 batch는 101개 target과 각 target의 source skill/stage/clip identity를 정확히 한 번 소유한다.
- 모든 target은 기존 제품 ID가 아니라 정규화된 `.unified` ID/path를 사용한다.
- 최초 생성은 source clip에 속한 모든 carrier의 `SourceRecipe`, exact DDS/WModel, material provenance를
  deterministic stable ID로 이식한다.
- 모든 imported source resource binding을 exact superset으로 보존하고, source와 충돌하는 기존 DDS는
  source 값으로 교체한다. source-missing supplemental slot은 receipt에서 별도 분모로 고정한다.
- 재생성은 stable source identity로 field-aware join하고 사용자 소유 `Visible`, `Detail.Transform`, 색/알파,
  Decal diffuse를 보존한다. whole-file hash 차이를 이유로 사용자 튜닝을 덮어쓰지 않는다.
- unsupported source carrier는 evidence와 resource binding을 보존하되 `visible=false`/fail-closed로 둔다.
- 101개 전체를 memory stage한 뒤에만 candidate 파일을 한 transaction으로 commit한다.
- 실패하면 기존 14개와 Legacy product 문서를 유지하고 새 파일의 부분 생성물을 남기지 않는다.
- EffectCatalog와 animevent mutation은 이 G에서 0이다.

### 현재 중단 지점과 재개 순서

현재 디스크에는 공용 compiler와 family materializer를 적용한 baseline 101개/4,777 elements가 transaction
write됐다.
공용 compiler는 material family identity와 occurrence parameter를 분리하고 exact physical package/material
full-path join을 사용한다. suffix/object-name 추측, 타 캐릭터 donor texture, 명시 정책 없는
`EngineMaterials` admission은 거부한다. DimensionMaster 784개 canonical sibling join만 해당 source에
한정하며 다른 직업에는 추정 확장하지 않는다.

현재 baseline 분모는 Particle 4,687 중 portable 4,488 / deferred 199, portable 중 product-full 3,481 /
fail-closed 1,007이다. Decal은 46 ready + 33 authoring-incomplete, AnimationTrail은 source notify 8 /
element 11이고 placeholder는 0이다. `CHARACTER_AFTERIMAGE`는 72 occurrence / 29 target의 receipt-only
ledger다. 현재 authored 101의 `authoringApproximate`는 0이다.

Wave 0 material evidence와 Codex admission/Lance 경계의 no-write projection은 product-full 2,793 /
authoring-approximate 722 / hard 973, preview target 3,515다. Hard는 safe drawable/profile/module evidence가
없는 compiler/evidence blocker다. 이 수치는 G3 통합과 baseline 육안검증 전에는 고정하거나 write하지 않는다.

baseline 생성 당시 base/compiler/portable/candidate/Valtan tests와 write/check는 PASS했다. 이후 Codex
소묶음은 material contract check, Lance/approximate Python 39/39, 3-way cheap native gate, portable event 5/5,
publisher rollback, ClientFrontendHarness Debug build를 PASS했다. Claude G3의 generic authoring family와
resource/scalar/color override ownership도 공용 schema/Codec/Renderer/Effect Tool/publisher에 통합했고,
`--effect-g3-authoring-fast` 15/15, Python ownership 3/3, `Test-EffectPipeline.ps1`, Client x64 Debug
compile/link와 대표 7문서 ordinary load 7/7을 PASS했다. 이 자동 증거는 사용자 화면의 색·실루엣을
승인한 것이 아니다.

재개 순서는 다음으로 고정한다.

1. material family invariant와 occurrence parameter 분리, exact source contract join — 완료
2. non-Cascade Particle module/runtime와 Full/Approximate/Hard admission 소묶음 — 완료
3. Decal 79, AnimationTrail 11, `CHARACTER_AFTERIMAGE` 72/29 stable evidence materialization — 완료
4. Claude G3 13 authoring family/override ownership 코드 통합과 Client x64 Debug 빌드 — 완료
5. 기존 101 baseline 대표 사용자 육안검증 — Artist F, DimensionMaster A, Lance BA1 승인. DM T/DM BA3/Warlord/Valtan은 아래 alternate slice로 재개
6. DM T bind-pose, DM BA3 8-element, Warlord exact chain, Valtan 상단 owner selector 교정 — focused 구현·자동검증·Client Debug 완료
7. 네 occurrence 사용자 재검증과 Save/Reload 확인 — 현재 단계. Lance 승인본은 자동 회귀 control로 보존
8. 방향 승인 뒤 final denominator, 101 transaction rewrite와 연속 check — 보류
9. Full+Approximate actual Stage/Draw, Hard rollback, Debug/Release/publisher — rewrite 뒤

### 2026-08-15 대표 육안 실패 뒤 교정 계약

사용자가 기존 101 baseline을 실제 Client에서 확인한 결과 Artist F 31470과 DimensionMaster A 2050210은
승인 control이 됐다. 나머지는 자동 draw 성공을 admission 또는 fidelity로 승격하지 않고 다음 수직 slice를
먼저 닫는다.

| 대상 | 확정된 결함 경계 | 구현 결과와 자동 증거 | Approximate 전환 조건 |
|---|---|---|---|
| DimensionMaster T 2050500 | 첫 30-tps retime 뒤에도 cage가 남았다. active Action frame이 WSKL rest에 bake된 반면 WMSH는 PSK inverse bind를 유지해 normalized bind 오차가 최대 `90.678497`이었다 | Action detach + POSE bind snapshot으로 SHA `87186351...22b6`을 recook. identity `5.68e-14`, 4 section/13,806 verts/20 source bones, 두 30-tps clip과 19 moving clock bones, source/runtime CPU skin bounds PASS. 후반 unsupported notify는 2.90142초로 분리 유지 | ModelCue/runtime/배포 결함이므로 근거 없이 Approximate로 낮추지 않음 |
| LanceMaster 34010 BA1 `ce43...` | DDS/WModel 오연결이 아니라 exact alpha/emissive/dissolve/noise lane을 generic grouped evaluator가 축약하고 `emissive_tex_strength=5000`을 tint/order 없이 포화시켜 흰 arc 생성 | exact two-emissive evaluator가 5 lane, ParticleColor, strength/pow/UV 순서를 소비해 WARP 9,677 px 비백색 chroma를 증명했고 사용자가 승인했다 | disk admission은 그대로 유지하며 승인본은 회귀 control로 보존 |
| DimensionMaster 2050010 BA3 composite | 기존 gate는 8개 중 water 한 개뿐이었다. ring의 zero emission을 final tint로, makeflow back color를 final tint로 오용했고 두 particletrail은 hidden alpha lane과 EPAL_Z one-sided front-normal 역전으로 미출력됐다 | exact WATERTRAIL/LINEARFLOW/MAKEFLOW/RING/PARTICLETRAIL evaluator, hidden DDS staging과 EPAL_Z 부호를 교정했다. ordinary Mesh4/Sprite4 전부 nonzero RGB, typed 7 chroma와 composite `B>R>G` PASS | disk admission은 그대로이며 사용자 승인 전 FULL/Approximate 승격 금지 |
| Warlord 17090 | admission 교정 뒤 WModel material lane이 비어 흰색으로 나왔고 LocationDirect는 미직렬화 ScaleFactor zero 때문에 정지했다 | `fx_d_grid_016.dds`는 parent reference 9개 중 하나일 뿐 Base/opacity input으로 입증되지 않았다. alpha는 전부 255이고 cooked parent의 opacityMask/WPO output과 texture parameter mapping이 없으므로 기존 generic standard auto-binding은 `SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE`로 철회한다 | Track 2가 9-DDS SHA/channel receipt와 false-exact 제거를 소유한다. 이번 Track 1 Product admission에서 제외하고 다른 family에 일반화하지 않는다 |
| Valtan 420633 | authored/binding은 있으나 상단 All Effects owner selector가 PlayerSkills 6 class만 소유해 하단 별도 tree를 사용자가 찾을 수 없었다 | `CHARACTER_CLASS` 불변인 UI-only `Character / Boss` 7-option selector와 Valtan exact 420633 unified tree를 연결했다. player six 불변, exact once, invalid rollback PASS | UI/index 결함이므로 Approximate와 무관 |

이 교정과 사용자의 재검증이 끝나기 전에는 `EXPECTED_*`, 101문서, product mapping을 다시 쓰지 않는다.

## 6. G02 101개 source-backed Track A authored Effect

### Track A가 있는 13 stage

- Artist 31000 BA1~BA4
- DimensionMaster 2050010 BA1~BA4
- LanceMaster 34010 BA1~BA4
- Warlord 17000 BA1 canary

`CEffectDocumentCodec::Build_GenericAuthoredElementImportStage` 하나를 사용한다. F 전용 Core33,
Y -90°, `modelPreScale=0.01`, ArtistVisualV4 opcode를 다른 직업의 기본값으로 복사하지 않는다.

각 carrier는 다음 상태 중 하나를 명시한다.

- `ADMITTED_SOURCE_PROFILE`
- `TYPED_EXECUTION`
- `SOURCE_PRESERVED_DEFERRED`
- `FAIL_CLOSED`

현재 material admitted carrier는 authored `SourceRecipe`와 grouped-translucent material을 유지한다.
나머지는 white/generic material로 가짜 복원하지 않고 exact recipe/resource evidence를 보존한 채
fail-closed 한다.

### 기존 batch packet이 없던 61 stage

이미 확보한 per-skill imported document와 stage manifest를 source truth로 사용한다. stage/clip timeline과
source event identity로 carrier를 분리하고 source emitter를 재사용하거나 Legacy element를 borrow하지 않는다.
리소스가 물리적으로 없거나 runtime capability가 없는 carrier는 누락시키지 않고 blocker와 함께 보존한다.

이 G의 완료는 101개 문서의 source identity/resource binding과 안전한 fail-closed까지다. 사용자의 화면 확인
전에는 `VISUAL_APPROVED` 또는 `PRODUCT_MAPPED`로 승격하지 않는다.

## 7. G03 Effect Tool 편집과 Legacy 격리

### 정상 UI

- 101개 `.unified` 후보를 class → skill → stage/clip 아래 표시하고 모두 Open/Edit/Save를 제공한다.
- candidate를 열면 대응 character model과 실제 clip을 Model View에 연결한다.
- admitted element에는 Play All, Family, Solo, Visible, Transform, DDS slot, Detail, Save/Reload를 제공한다.
- capability-deferred 또는 portable fail-closed Particle과 missing-Base Decal은 편집·저장은 허용하되
  Visible/Play를 잠가 미지원 source가 부분 실행되지 않게 한다. exact AnimationTrail 11개는 별도 history
  family로 편집·재생하며 `CHARACTER_AFTERIMAGE` ledger는 generic Element로 노출하지 않는다.
- multi-clip 후보는 현재 product cue 순서를 유지한다.

### Advanced Migration Reference

- candidate sibling이 생긴 기존 Legacy는 정상 목록에서 제거한다.
- Legacy는 inspect/preview와 Save As만 허용한다.
- Save Changes로 원본 Legacy를 덮어쓰지 못하게 UI와 backend 양쪽에서 차단한다.
- F old reconstructed entry도 Advanced/rollback reference에서 접근 가능한 명시 row로 연결한다.
- Warlord 17820 clip3/4/8 orphan은 자동 product mapping에서 제외한다.

### Ribbon/Trail과 Light

- Ribbon/Trail element와 texture slot type은 삭제하지 않는다.
- 기존 DDS binding과 Detail/Visible/Transform 값은 round-trip에서 보존한다.
- 이번 101개 materialization은 Cascade source Ribbon compiler와 Light runtime admission을 추가하지 않는다.
- AnimationTrail은 Cascade와 분리된 authored contract/history runtime으로 이번 작업에서 구현한다.
- 미지원 family는 다른 family의 Play All을 실패시키지 않고 해당 family만 deferred/suppressed로 표시한다.

## 8. G04 Valtan Whirlwind authored Effect

### 정본

- gameplay: `Data/Encounters/Valtan/ValtanEncounter.json`
- animation: `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- visual mapping: `Data/Animation/Authored/Valtan/Valtan.patterneffects.json`
- effect: `Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json`
- Korean action name authority: main PR #103 (`28fa75a2`), source commit `72cc7629`,
  `Data/Animation/Reference/Valtan/Valtan.skilltiming`

### 이번 실행 분모

- notify-006 WWind: Sprite 1 + Mesh 2, 현재 executable 3 유지
- notify-005 Dust 2: deferred
- notify-004 AnimationTrail 3: 4캐릭터 AnimationTrail history runtime을 재사용해 후속 연결
- notify-009 Light 1: deferred

Dust는 generic 흰 sprite로 낮추지 않고 이번에는 source evidence를 보존한 채 fail-closed 한다.

Model View는 `Boss_Valtan`, action `420633`, active clip `mesh_att_battle_20_03`, 실제
`b_effectroot` bone을 사용한다. Server의 `serverTick/actionStartTick/patternSequence/stageIndex`로 계산한
authoritative age를 animation과 effect가 함께 소비한다.

이번 Valtan 1차 완료 상태는 `WWind 3 executable`, Dust/Light deferred다. AnimationTrail은 4캐릭터
history runtime이 검증된 뒤 같은 typed contract로 연결하고 gameplay damage와 duration은
`ValtanEncounter.json` 권위를 유지한다. 제품 mapping은
사용자 visual 승인 뒤에만 연다.

## 9. G05 사용자 visual 승인과 제품 전환

제품 cue 정본은 clip occurrence다. 따라서 전체 완료 판정은 102 visual occurrences를 기준으로 한다.

각 occurrence에서 사용자가 다음을 확인한다.

1. 대응 character 또는 Valtan model과 clip
2. Play All
3. Family
4. 필요한 occurrence Solo
5. DDS/Visible/Transform/Detail Save와 Reload
6. 실제 gameplay anchor, timing, follow/snapshot 결과

승인 전에는 기존 gameplay mapping을 유지한다. 승인된 occurrence만 다음 transaction으로 전환한다.

```text
candidate catalog row stage
→ animevent/boss-pattern effect asset 교체 stage
→ 모든 reference와 authoring document validate
→ catalog + mapping 단일 commit
→ 실패 시 기존 Legacy product mapping 유지
```

활성 4직업 animevent target은 현재 98개이며 Warlord orphan 3개는 별도 사용자 결정 전 제외한다.
전환 뒤 Legacy는 Advanced read-only rollback reference로 유지한다. 승인된 Track A가 제품 정본이 되고
catalog/animevent 전체 reference가 0임을 검증한 뒤에만 해당 Legacy를 제거한다.

## 10. G06 성능 검증은 구현 뒤 수행

이번 101/101과 Valtan Whirlwind 구현 중에는 새 최적화와 IOCP 교체로 범위를 넓히지 않는다. 이미 들어간
Effect Tool cache/debounce, prepared handle/buffer reuse, aggregate effect budget과 Server bounded
ingress/outbound는 보존한다.

Effect 구현과 사용자 visual 승인이 끝난 뒤 두 topology를 모두 검증한다.

### 로컬 검증

- Server 1 + Client 4를 같은 PC에서 실행
- 해상도 `1280 × 720`
- Client 목표 `60 FPS`
- Server 목표 `30 Hz`

### LAN 플레이 검증

- server-host 한 대와 다른 팀원의 Client 참여
- 같은 gameplay/effect revision
- 4명 동시 스킬과 Valtan Whirlwind
- disconnect/reconnect와 slow reader가 다른 player를 막지 않음

측정 항목은 Client frame p95/p99와 최대 non-load hitch, Server room tick p95/p99, ingress/outbound
depth와 drop/coalesce다. IOCP는 성능 때문에 검토한 후속 구현이며, 위 두 topology의 계측이 현재
thread+bounded queue 구조로 목표를 못 맞출 때 별도 수직 슬라이스로 진행한다.

## 11. 최소 자동 검증과 수동 경계

canonical/SHA receipt 자체를 작업 목표로 확대하지 않는다. 다음은 사용자 파일과 제품 mapping을
안전하게 지키기 위한 최소 검증이다.

- JSON/schema parse
- 101 target count, v13과 effectAssetId/path 일치
- materializer rollback과 부분 파일 없음
- ClientFrontendHarness focused materializer/ordinary Stage_Document 실행
- Client Debug/Release compile/link
- Effect publisher Validate
- `git diff --check`

에이전트는 Client/UI를 자율 실행하거나 visual fidelity를 대신 판정하지 않는다. 102 occurrence와
Valtan Whirlwind의 visual PASS는 사용자의 서면 관찰만 기록한다.

## 12. 완료 정의

이번 작업은 다음 조건이 모두 충족될 때 종료한다.

- 4직업 `.unified` 후보 `101/101`, 전부 authored v13
- 별도 Artist F를 포함한 물리 `.unified` `102`
- 정상 UI는 새 candidate를 표시하고 sibling Legacy는 Advanced read-only로 격리
- Track A admitted, source-preserved deferred, fail-closed 상태를 명확히 구분
- Cascade Ribbon과 Light data/slot을 훼손하지 않고 deferred 상태로 유지
- Valtan Whirlwind 메인 WWind 3 executable; Dust/Light는 deferred
- Cascade Ribbon과 분리된 AnimationTrail history family를 구현·검증
- 사용자가 승인한 occurrence만 product mapping 전환
- 기존 Legacy product와 rollback reference는 승인·zero-reference 전까지 보존
- 기능 구현 뒤 로컬 Server 1 + Client 4와 LAN 플레이를 1280×720, Client 60 FPS,
  Server 30 Hz 목표로 검증
- IOCP는 성능 계측이 요구할 때만 후속 수직 슬라이스로 진행

## 13. 2026-08-15 Track 1 cue 단위 Product admission 변경

### 정책 변경과 불변 경계

`AUTHORING_APPROXIMATE`는 더 이상 전역 Product Hard가 아니다. exact DDS/WModel carrier와 ordinary
runtime validation을 통과한 문서는 사용자가 승인한 exact cue에 한해
`PRODUCT_APPROVED_APPROXIMATE`로 Product 후보가 될 수 있다. 이는 exactness를 `FULL`로 바꾸는
승격이 아니다. receipt와 Effect Tool read-only 상태에는 admission과 observed exactness를 각각 표시한다.

다음 경계는 그대로 유지한다.

- 승인 목록에 없는 Approximate cue를 일괄 publish하지 않는다.
- hard fail-closed carrier는 source evidence로만 남고 실행되지 않는다.
- unsafe/missing resource, unknown shader/profile, cue identity/hash drift는 stage에서 거부한다.
- `PlayerSkills.effectId`는 Product mapping 정본으로 사용하지 않는다.
- Artist 전용 registry/shader와 Warlord의 근거 없는 `fx_d_grid_016.dds` binding을 일반화하지 않는다.
- G3 `authoringOverrides.resources/scalars/colors`와 stable reimport ownership은 admission 변경과 독립적으로
  round-trip 보존한다.

### 첫 canary와 cue별 rollback

첫 승인 집합은 다음 다섯 cue로 고정한다.

| cue | candidate | rollback |
|---|---|---|
| Artist F 31470 | `effect.artist.skill.31470.unified` | `effect.artist.skill.31470` |
| DimensionMaster A 2050210 | `effect.dimensionmaster.skill.2050210.unified` | `effect.dimensionmaster.skill.2050210.authored-baseline` |
| DimensionMaster LMB BA1 | `effect.dimensionmaster.skill.2050010.ba1.unified` | `effect.dimensionmaster.skill.2050010.ba1` |
| DimensionMaster LMB BA3 | `effect.dimensionmaster.skill.2050010.ba3.unified` | `effect.dimensionmaster.skill.2050010.ba3` |
| LanceMaster LMB BA1 | `effect.lancemaster.skill.34010.ba1.unified` | `effect.lancemaster.skill.34010.ba1` |

source policy는 animation asset, class, skill/stage, clip, start ms, candidate/rollback ID, candidate SHA와
사용자 결정 provenance를 cue마다 저장한다. publisher는 policy와 catalog/animevent/skillbindings를 함께
검증하고 `EffectProductCueAdmissions.runtime.json` receipt를 catalog hash에 묶는다. runtime catalog load는
catalog와 receipt를 함께 stage/commit/rollback한다. animevent loader는 approval-managed target에 대해
`(animationAssetId, clipName, startMs, effectAssetId)` exact match를 요구하고, staged cue target 전체의
prewarm이 성공한 뒤에만 Character document를 교체한다. 실패하면 기존 committed cue document를 유지한다.
source 전환 도구는 한 번에 cue 하나만 candidate 또는 rollback target으로 원자 교체하므로 rollback 단위도 cue다.

publisher는 현재 문서에서 `FULL`, `AUTHORING_APPROXIMATE`, hard-suppressed count를 다시 계산한다.
Approximate가 0이더라도 사용자 opt-in provenance는 보존하지만 observed exactness를 거짓으로 바꾸지 않는다.
후속 재import로 Approximate가 생기면 exact cue 승인과 pinned document SHA를 함께 갱신하지 않는 한 publish가
실패한다.

### Track 1/Track 2와 여섯 playable 경계

Track 1은 위 다섯 canary와 공용 publisher/runtime admission만 수정한다. DimensionMaster T 2050500,
Warlord A 17090, Valtan 420633 직접 시각 변경과 Effect Tool Resources compact UI는 Track 2 소유다.
Gunslinger와 Slayer는 다른 class authored 문서를 복사하지 않는다. 두 class는 source `.animevents` EFFECT
inventory와 package/object intake receipt부터 만들고, 필요한 exact WModel/DDS는 package/object/소비
occurrence/SHA·format 요구를 Track 2에 전달한 뒤 materialization한다.

### focused 검증

- policy schema/duplicate/unknown cue, wrong class/skill/stage/clip/start ms, SHA drift 거부
- unapproved Approximate 거부, exact approved Approximate 허용, hard/unsafe 계속 거부
- catalog + admission sidecar publish fault rollback
- candidate 전환과 cue 하나 rollback에서 다른 cue byte 불변
- runtime catalog receipt hash와 exact cue admission, catalog snapshot restore
- 기존 Artist F와 legacy/full cue 회귀, G3 override round-trip
- Effect publisher Validate, focused Python/C++ harness, Client x64 Debug build, `git diff --check`

# 2026-08-11 Artist 31470 F High-Ceiling Restoration Final Implementation Plan

기준일: 2026-08-11

참고 materializer checkpoint: `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` — BLOCK, 재사용하지 않고 final Source/Material schema 위에서 다시 작성

계획 갱신 branch: `codex/artist-f-high-ceiling-final-plan-v2`

최종 통합 branch: `codex/artist-f-reconstructed-integration-v1`

최종 통합 시작 commit: `38ebe7cf7dceb5054bde93812907173cc0f98c67`

현재 구현 기준 commit: `18d2b48920b2a327ac59b572960325d352e77a6f`

2026-08-10 사용자 승인: source-era 동일성을 주장하지 않는 `RECONSTRUCTED_APPROVED_V1` 실행 경로를
진행한다. 이 승인은 Source 29행과 Material 255행의 historical evidence blocker를 삭제하거나
`SOURCE_EXACT`로 승격하는 허가가 아니다. 각 행을 stable policy/capability ID, explicit typed value,
fixed-input/output oracle, finite tolerance와 implementation identity에 결합한 뒤 execution blocker만
별도로 해소하는 허가다. R2 immutable candidate/parser/catalog transport는 완료됐고, 선택 Mesh/Sprite
production evaluator checkpoint `18d2b489`는 독립 review 중이다. GPU sink, actual Artist F, Product admission은
아직 열리지 않았으며 `Execute=false, Submit=false, Render=false, Product=false`를 유지한다.

문서 역할: 이 파일은 전체 범위와 G 순서, worktree 소유권, admission predicate를 소유하는 최종
구현 계획서다. 각 mutation lane은 기존 Source/Geometry/Material/Compiler PLAN을 갱신하거나 해당 G의
전용 구현 PLAN을 먼저 만들고, public H/CPP의 세부 선언·전체 코드는 그 lane 문서에서만 소유한다.
같은 코드를 이 master 문서에 복제하지 않는다.

## 2026-08-11 고점 복원 최종 결정 정본

이 절은 현재 구현 상태, Effect Tool 정책, 세션 운영과 M0~M3 순서의 최상위 정본이다. 아래 historical
R0~G11 기록과 기존 M0 문구가 이 절과 충돌하면 이 절을 우선한다. 기존 evidence와 frozen SHA는 삭제하지
않되, 과거 상태 문구인 `R2 IN_PROGRESS`, `exact10 only`, `production evaluator NOT IMPLEMENTED`,
`R3-R8 GATED`, 상시 3세션 병렬 구현은 현재 판정으로 사용하지 않는다.

### 바뀌지 않는 최종 목표

- 수동 근사치로 적당히 보이게 만드는 데서 멈추지 않고, 동일 generic pipeline으로 도화가 F 35/35를
  실제 Client에서 실행하고 occurrence별로 눈 검증·수치 환류한 뒤 4개 class와 Valtan까지 확장한다.
- historical source-era 동일성은 주장하지 않는다. 결과는 끝까지 versioned reconstructed restoration이며
  `sourceExactAdmission=false`를 보존한다.
- 사람이 보는 결과는 필수 종료 증거지만 유일한 admission 근거는 아니다. 자동 numeric/structural gate를
  먼저 통과하고, 수동 관찰을 stable occurrence fixture와 tuning overlay로 환류한 뒤 다시 자동 gate를
  통과해야 한다.
- 첫 픽셀, 35/35 nonProduct, 실제 F Debug, Product D/R을 서로 다른 milestone로 판정한다. 앞 단계 화면이
  보였다는 이유로 뒤 단계 admission을 열지 않는다.

### 현재 exact 기준선

| 영역 | exact 상태 | 현재 판정 |
|---|---|---|
| integration | `18d2b48920b2a327ac59b572960325d352e77a6f`, clean/pushed | 이 계획과 fresh 구현 세션의 기준 |
| LF exact13 producer/parser/catalog | `ab559784a7fa0c5d19ccedf600d08f8a9ee50a25` | PASS. fresh-LF exact13 `27,065,827 / ea3afd4e...`, old10 `26,255,930 / b5086d14...` |
| immutable runtime program | 7 schedule / 35 emitter / 399 module / 629 distribution | candidate/parser/catalog/CPU plan compile 완료, 모든 admission false |
| selected evaluator | `18d2b489`, Mesh 1 + Sprite 1 / module-handler consumption24 / unique route15 | 코드와 public API는 존재하나 독립 review 미종료, production caller 0이므로 admission 0 |
| offline resource evidence | WModel 7/7, DDS 48/48, binding 72/72 | bytes와 offline authority만 PASS. immutable GPU composite와 renderer readiness는 0 |
| renderer sinks | Mesh/Sprite/Decal/Ribbon/Light/Post | production typed sink 0/6, first pixel 0/1 |
| Effect Tool typed path | exact CatalogEntry -> evaluator -> GPU composite | 미연결. 기존 legacy Product mode만 존재 |
| actual Artist F | `PlayerSkills 31470 effectId=""`, original animevent refs | Debug diagnostic와 Product 모두 미연결, 0/35 |
| admissions | Execute / Submit / Render / Product | 모두 false. 현재 상태에서 하나라도 일반 경로를 열면 empty/no-pixel success가 될 수 있으므로 금지 |
| expansion raw inventory | `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53` | extraction/inspection 완료, Artist F GPU schema 동결 전 runtime binding으로 승격 금지 |

`18d2b489`의 최대 인정 범위는 evaluator review가 PASS한 뒤에도 Mesh/Sprite packet 2/35와
module-handler consumption 24/399, unique route 15다.
GPU draw, Tool caller, actual F, Product 완료를 뜻하지 않는다. 현재 main worktree의 Manual Restoration
Workbench dirty WIP도 별도 보존 대상이며 이 integration 기준선이나 runtime authority가 아니다.

### Effect Tool과 기존 effect 정책

Effect Tool은 새 도구를 만들지 않고 기존 `CEffect_Tool` 하나를 두 mode로 확장한다.

| mode | 입력 권위 | 허용 작업 | 금지 |
|---|---|---|---|
| Legacy Product | 기존 v13 Authored/Assembly와 legacy prepared document | 기존 effect의 inspect/play/edit 흐름 유지 | typed Artist F로 자동 변환, 기존 cue 일괄 교체 |
| Typed Restoration | 같은 immutable CatalogEntry pointer의 compiled program/resource authority | complete/family/occurrence solo, seek, read-only Source/IR inspection, tuning overlay 편집, validation 실행 | Source/IR Save·Apply, disk reread, 두 번째 renderer/runtime, legacy fallback |

기존 effect는 Artist F 개발 때문에 바꾸거나 재publish하지 않는다. 101은 4-class corpus의 기존 Product
cue 분모이며 전체 Catalog effect 수와 같은 뜻이 아니다. typed migration은 Artist F가 M3를 통과한
뒤 `Artist 9-skill canary -> DimensionMaster -> Warlord -> LanceMaster` 순서로 cue 하나씩 atomic하게 수행한다.
한 cue의 typed stage/prewarm/validation이 실패하면 그 cue의 legacy Product를 그대로 보존한다. 101 cue를
한 번에 변환하지 않는다.

fresh 구현 세션의 step 0에서 어떤 M0 코드도 수정하기 전에 plan parent `18d2b489`의 exact runtime
Catalog로 `legacy-product-cue-projection-v1`을 만들고 독립 review로 동결한다. 이후 새 tree에서 projection을
다시 만들어 baseline을 바꾸지 않는다. 4-class 기존 Product cue
101개 각각의 cue ID, effect asset ID, payload kind, canonical Catalog entry SHA, Authored/Assembly identity,
prepared mode를 기록하고 전체 non-target Catalog entry set도 canonical projection으로 고정한다. Artist F
publication 전후 old101은 ID set과 row SHA가 101/101 exact-equal이어야 하고, 그 밖의 non-target entry도
set/row projection delta 0이어야 한다. M0~M3의 각 frozen checkpoint에서 이 baseline과 delta를 검사한다.
Debug/Release에서 Legacy mode의 101/101 load/prepare와 대표 actual
playback, Legacy↔Typed mode 전환 실패 rollback을 검증한다. 새 Artist F entry의 stage 또는 mode switch가
실패하면 기존 Catalog snapshot, legacy prepared pointer와 Tool selection을 모두 보존한다.

Source Contract와 immutable IR은 read-only다. 사람이 조정할 수 있는 것은 stable occurrence ID를 key로 한
별도 versioned tuning overlay뿐이다. overlay는 숫자/curve/visibility timing 같은 승인된 조정 field만
소유하고 asset ID, resource SHA, renderer family, handler/opcode, owner, source identity를 바꾸지 못한다.
main worktree의 미커밋 Workbench는 삭제하지 않되 M0 first pixel 뒤 UI/queue 구현으로만 재감사하며,
runtime authority나 Artist F 구현 branch에 그대로 cherry-pick하지 않는다.

overlay authoring path는
`Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31470.tuning-overlay.json` 하나다. 저장 계약은
`lostark.effect-restoration-tuning-overlay` formatVersion 1로 동결한다. root는 정확히
`schema, formatVersion, effectAssetId, baseProgramId, baseProgramVersion, baseProgramSha256, entries,
canonicalProjectionSha256, overlaySha256`를 가지며, entry는 exact stable occurrence ID, approved parameter ID,
typed finite value/curve, rationale와 user decision만 가진다. unknown key, bool-as-number, nonfinite, duplicate
occurrence/parameter, base Program mismatch를 거부한다.

Tool의 Save는 active runtime을 직접 바꾸지 않는다. temp overlay를 `parse -> validate -> canonical seal ->
offline compile -> new Program/receipt -> Catalog stage -> resource prewarm -> equivalence -> atomic commit`한 뒤에만
새 revision을 선택한다. 실패하면 기존 overlay file, compiled Program, Catalog revision, prepared pointer와
GPU composite를 모두 보존한다. overlay SHA는 compiler input receipt와 compiled Program canonical projection에
들어가며 exact13이 pin한 Program SHA를 통해 publisher/runtime identity에 결합한다. action-time/per-frame
overlay file read, runtime patch, Program SHA를 유지한 in-place tuning은 0이어야 한다.

### 사람 눈 검증과 occurrence 튜닝 계약

사용자가 최종 visual reviewer다. 외부 reference root는
`C:/Users/user/Desktop/로스트아크이펙트이미지`이며 차원술사 기준 PNG는
`C:/Users/user/Desktop/로스트아크이펙트이미지/차원술사`에 있다. 현재 이 폴더의 PNG는 68개이며,
`차원술사_복원`의 기존 비교 자료와 함께 사람이 형태·타이밍·색·밀도·궤적을 판단하는 참고 자료로만
사용한다. Git 정본, source-exact 증거, 자동 screenshot oracle로 승격하지 않는다.

수동 관찰 한 건은 최소 다음 key로 기록한다.

```text
catalogRevision / compiledProgramSha256
skillId / cueId / occurrenceId / rendererFamily
fixed sample time 또는 seek time / camera·attachment 조건
referenceImagePath 또는 사용자의 known-form 설명
defect category: transform, scale, timing, spawn, color, alpha, UV, material, trail,
                 decal, light, post, missing, extra 중 하나
expected observation / actual observation / approved tuning field
numeric or structural fixture added / Debug recheck / Release recheck
userDecision: APPROVED, RETUNE, BLOCKED
```

35 occurrence는 각각 사용자가 `APPROVED`해야 한다. 전역 scale·brightness·speed로 여러 occurrence를
한꺼번에 덮지 않고 complete -> family -> occurrence solo와 seek로 원인을 좁힌다. 조정 뒤에는 해당
occurrence fixture와 전체 35/399/629 회귀를 모두 다시 실행한다.

### fresh 세션 운영 결정

Artist F M0부터 M3까지 shared C++ 구현은 **fresh 구현 세션 하나**가 직렬 소유한다. Catalog, evaluator,
Object, Renderer, Shader, Effect Tool, Presentation을 여러 구현 세션에 나누지 않는다. 별도 reviewer는
구현 중 live dirty diff를 따라가지 않고 commit/push된 frozen SHA가 생겼을 때만 read-only로 P0/P1을
판정한다. raw corpus specialist는 Artist F interface가 M3에서 동결된 뒤 R8에서만 다시 병렬화한다.

세션 간 협업은 main-local ignored 규칙
`C:/Users/user/Desktop/LostArk/.md/GB/gotchas.local.md`를 따른다. 이 파일은 새 worktree에 자동 checkout되지
않으므로 fresh task prompt에도 동일 규칙을 전문이 아닌 실행 가능한 요약으로 반드시 넣는다. 다른 task에 메시지를 보내기 전에
대상 task의 최신 exact SHA, dirty ownership, 현재 단계와 blocker를 먼저 읽고 `ADD`, `CORRECT`, `STOP`,
`REVIEW_EXACT_SHA` 중 하나로 분류한다. 상대 진행을 읽지 않은 일반 조언, live WIP 재구현, reviewer의
직접 수정은 금지한다.

fresh 구현 세션의 규칙:

1. 이 plan-only commit을 parent로 clean worktree를 만들고, 코드 수정 전에 `18d2b489` old101/non-target
   Catalog projection을 생성·독립 동결한다.
2. 한 번에 milestone의 검증 가능한 한 checkpoint만 구현하고 commit/push한다.
3. 보고는 `base SHA / exact commit / owned files / actual tests / blocker delta / next entry`만 보낸다.
4. reviewer PASS 전 다음 admission을 열지 않는다. P1은 같은 branch의 corrective commit으로 닫고 새 SHA를 재감사한다.
5. acceptance와 직접 연결되지 않은 새 receipt, wrapper, audit framework를 추가하지 않는다.
6. 모든 fresh checkout에서 candidate/catalog LF byte identity를 재검증한다. 오래된 CRLF-smudged worktree는
   증거로 사용하지 않는다.
7. dirty main worktree의 source/data/build output은 읽지 않는다. 유일한 예외는 AGENTS 계약상 팀장이 직접
   관리하는 physical runtime input `C:/Users/user/Desktop/LostArk/Client/Bin/Resources`다. 이 root는 M0~M3에서
   read-only로만 사용하고 selected asset의 Resources-relative path/size/SHA를 compiled authority와 대조한다.
   배포·교체·정리는 별도 explicit transaction 없이는 금지한다.

### 최종 milestone과 순서

| milestone | 구현·검증 범위 | 종료 상태 |
|---|---|---|
| M0-A | `18d2b489` evaluator 독립 판정과 필요 corrective, harness-only request 482줄을 Program/sidecar-derived generic production diagnostic factory로 factor | same CatalogEntry, Mesh/Sprite packet 2/2, module-handler consumption24/unique route15, empty/failure rollback PASS; 일반 admission false |
| M0-B | 1 WModel/preScale, 4 DDS same-byte load, 2 material packet, exact sampler Set/Apply/readback/Undo, Mesh/Sprite typed sink | actual GPU draw 2/2, pipeline stats nonzero, D3D ERROR/CORRUPTION 0, sink 2/6; 일반 admission false |
| M0-C | 기존 Effect Tool의 Typed Restoration diagnostic/solo에서 actual Debug Client first pixel | 사용자가 실제 HWND를 확인하고 occurrence 026/027 결과를 기록; Product false |
| M1 | production evaluator를 7/35/399/629로 일반화, Geometry 7/7, Material 23 family/27 recipe/34 occurrence, DDS48/sampler72/state-input46 exhaustive disposition, output visual-composite33, six sink, provider-local Light+Post atomic submit | full 35 nonProduct standalone, sink 6/6, state46/46·visual33/33, unknown/silent fallback 0; Execute/Submit/Render seam은 각 proof만 별도 개방, Product false |
| M2 | actual Server command -> snapshot -> Artist 31470 animation action의 Debug-only nonProduct typed route, 같은 CatalogEntry pointer, Tool complete/family/occurrence solo·seek, 관찰→overlay/fixture→재compile 반복 | 실제 F Debug에서 35 occurrence 모두 사용자 `APPROVED`; `RETUNE/BLOCKED`가 하나라도 있으면 M2 안에서 반복. Product Spawn과 Release Product route는 false |
| M3 | M2 approved overlay/program을 변경 없이 Debug/Release full regression하고 Product cue/effectId/animevent/format3 publication과 rollback을 한 transaction으로 commit | Source 35/399/629, Geometry 7/7, Material 27/34, renderer 13/16/3/1/1/1, old101/non-target delta0, Product 35/35, 실제 F D/R eye smoke. 새 visual defect는 M2로 되돌림 |
| R8 | frozen generic interface에 4-class와 Valtan corpus 적용 | class/count production 분기 0, 새 의미가 나오면 근사하지 않고 shared capability gate로 복귀 |

진행률은 코드 줄 수가 아니라 아래 exact denominator로만 계산한다.

| 시점 | occurrence/module | material/geometry/resource | renderer/visual |
|---|---|---|---|
| 현재 `18d2` | admitted 0; review 후보 packet2/35·module-handler consumption24/399·unique route15 | recipe2/27·material occurrence2/34·carrier1/7·DDS4/48 후보 | sink0/6, first pixel0 |
| M0 PASS 뒤 잔여 | occurrence33·module375. distribution/property는 slice 차감 근거가 없어 629/1,434/1,572 전체 gate 유지 | recipe25·material occurrence32·carrier6·DDS44·sampler68 | Mesh12·Sprite15·Decal3·Ribbon1·Light1·Post1, sink4 |
| M1 PASS | 35/399/629와 property/leaf 전체 consumed 또는 verified-irrelevant | Material27/34·Geometry7·DDS48·sampler72·state input46/46 exhaustive disposition | sink6/6·visual occurrence composite33/33 |
| M3 PASS | actual cue7/7·occurrence35/35 | 같은 frozen tree에서 resource/overlay/publish identity 일치 | renderer13/16/3/1/1/1·Product35/35 |

현재 evaluator 뒤 M0~M3는 “몇 함수만 연결”하는 규모가 아니다. 중복을 제외해도 약 36개의
production/data/pipeline path, harness/audit/project/result를 포함하면 현실적으로 40~46 tracked file이
영향받을 수 있다. 이 수치는 scope를 늘리는 목표가 아니라 작은 검증 단위로 나눌 현실적 하한이며,
milestone denominator가 닫히지 않으면 파일 수나 경과 시간으로 완료를 주장하지 않는다.

M2의 actual F는 Debug-only nonProduct diagnostic route다. 현재 Product `Spawn`을 거짓으로 열지 않고 실제
Server/animation timing을 사용하되 Release에는 포함하지 않는다. `PlayerSkills.effectId`와 generated Product
`effectref=asset`의 최종 commit은 M3 atomic publication에서만 수행한다.

M2는 한 번 재생하고 종료하는 단계가 아니다. `RETUNE`이면 승인된 overlay field만 수정해 새 Program SHA를
만들고 자동 gate와 실제 F Debug를 다시 실행한다. 35개가 모두 `APPROVED`가 될 때까지 M2를 벗어나지
않는다. M3는 approved Program의 Release/Product 재현 단계이며 M3에서 값이나 overlay를 즉석 수정하지
않는다. 새 차이가 나오면 publication을 rollback하고 M2로 되돌아간다.

Light/Post atomicity는 한 Artist provider가 만든 mixed batch 내부에서만 보장한다. 전체
`Presentation_Manager::Submit_FrameProviders`의 다른 provider까지 rollback하는 global Option-B 변경은 이
작업 범위가 아니다. provider failure 시 whole-frame을 비울지 다른 provider를 보존할지는 별도 Engine
계약으로 남기며 Artist F completion claim에 섞지 않는다.

sampler/state는 이름 기반으로 기존 FX11 backing을 신뢰하지 않는다. offline policy의
`W=WRAP, MaxAnisotropy=0, MinLOD=0`과 현재 Effects11 backing의
`W=CLAMP, MaxAnisotropy=16, MinLOD=-FLT_MAX` 불일치를 보존한다. M0/M1은 approved D3D sampler를
`SetSampler -> Apply -> effect/PS readback -> draw -> UndoSetSampler`로 scope하고 모든 실패 경로에서
원상복구한다. `single mip`을 full descriptor equality로 사용하지 않는다. render state는
`family + blend + two-sided + disable-depth + pass`의 visual occurrence composite로 결정하며 ScreenPost를
material D3D state denominator에 섞지 않는다.

### R8 raw corpus 재사용 계약

raw extraction은 다시 하지 않는다. latest frozen inventory `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53`의
4-class `51 skill / 74 stage / 113 clip / 5,232 occurrence`, Valtan typed target `8,753`, combined unique raw
request `1,212`를 재사용한다. inspected `1,208`, TGA blocker 4와 provenance/corpus blocker 18의 합계 22를
R8 진입 전에 독립 review하고 닫는다. Artist F의 sampler/state/material/geometry schema가 M3에서 동결된 뒤
한 번만 binding/materializer를 재생성한다. R8 순서는 `Artist 9-skill -> DimensionMaster -> Warlord ->
LanceMaster`, 이어서 `Valtan Particle -> Decal -> Trail -> Material -> Camera`다. 공통 handler가 없으면
값을 근사하거나 class switch를 넣지 않고 해당 capability를 generic pipeline에 추가한 뒤 다시 진행한다.

## 2026-08-11 Integration/M0 Captain 재개 계약

이번 재개는 최종 통합 branch `codex/artist-f-reconstructed-integration-v1`의 exact frozen HEAD
`18d2b48920b2a327ac59b572960325d352e77a6f`에서 시작한다. 아래 M0는 기존 R2~R8 전체 복원
계획을 대체하거나 Product admission을 앞당기는 단계가 아니다. 같은 immutable CatalogEntry에서
선택한 Mesh 1행과 Sprite 1행만 production evaluator와 새 typed Mesh/Sprite sink에 연결해
실제 Debug 창의 첫 픽셀을 확인하는 좁은 nonProduct checkpoint다.

### 재개 기준선

| 항목 | frozen 상태 |
|---|---|
| R2 fresh-LF candidate/parser | PASS. candidate 7/35/399/629와 LF identity 재사용 |
| Catalog | strict historical old10/current exact13, immutable CatalogEntry Program/sidecar pointer/revision PASS |
| CPU | typed execution Plan compile/semantic projection PASS |
| production evaluator | `18d2b489` 선택 Mesh/Sprite 구현, 독립 review 중, production caller 0 |
| GPU sink | 0/6 |
| actual Artist F action | NOT REACHABLE |
| 일반 admission | Execute=false, Submit=false, Render=false, Product=false |

### M0/M1/M2/M3

| milestone | 범위 | 종료 조건 | fresh captain sequence |
|---|---|---|---|
| M0 | exact CatalogEntry의 선택 Mesh/Sprite 2행, production packet, production GPU sink, Debug diagnostic/solo first pixel | 아래 M0 acceptance와 자동 gate 및 사람의 실제 창 확인 | **CURRENT** |
| M1 | Mesh/Sprite 외 Decal/Ribbon/Light/ScreenPost를 포함한 production evaluator와 GPU sink 6/6, sampler72/state46/visual33 closure | 여섯 family packet/sink와 전체 sampler/state rollback | NEXT, same captain |
| M2 | 7 cue/35 occurrence actual Artist F Debug-only nonProduct route, provider-local Light/Post atomic batch와 사용자 튜닝 반복 | actual F Debug 35/35, 같은 CatalogEntry, occurrence별 user APPROVED | NEXT, same captain |
| M3 | approved Program의 Debug/Release eye smoke, full regression, Product Catalog/admission | 기존 최종 `35/35, atomic cue publish`, old101/non-target delta0 | FINAL, same captain |

M0에서 일반 Execute/Submit/Render/Product admission은 계속 false다. M0의 완료 문구는 다음 한 줄만
사용하며 이보다 넓은 복원, renderer, actual F 또는 Product 완료를 주장하지 않는다.

```text
Artist 31470 nonProduct Debug M0 selected Mesh/Sprite 2/2 production packet and manual first-pixel PASS
```

### M0 acceptance 동결

1. publisher source exact4와 runtime exact13 생산자, C++ Catalog old10/new13 strict consumer는
   `ab559784`에서 PASS했다. fresh 구현 세션은 이 identity를 재생성하지 않고 fresh-LF checkout에서
   raw byte/hash와 Debug/Release focused harness를 재검증한다.
2. new13 CatalogEntry는 기존 Program과 새 render-resource sidecar를 같은 immutable entry가 강하게
   소유한다. Preparation, Plan, Object, Renderer가 이 entry pointer 하나를 공유하며 두 번째 catalog,
   loader, runtime을 만들지 않는다.
3. `18d2b489` production evaluator는 `LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS`와
   `CEffectReconstructedCpuInspector` 밖에 구현됐다. 독립 review를 먼저 끝내고, harness에만 있는 exact
   selected request factory를 Program/sidecar-derived generic production diagnostic authority로 factor한다.
   production C++에 Artist/31470/emitter hash/row SHA/count를 복제하지 않고 stable selector는 typed
   diagnostic input으로만 받는다. synthetic legacy
   Document/packet과 action-time candidate/sidecar reread는 계속 금지한다.
4. Debug F1 Effect Tool의 기존 world-preview Object가 diagnostic/solo orchestrator가 된다. 이 좁은
   M0 경로만 production evaluator와 production Mesh/Sprite sink를 명시적으로 호출한다. Object,
   Playback, Renderer의 일반 admission 함수와 일반 action 경로는 계속 false다.
5. 자동 판정은 nonempty packet 2/2, 실제 draw/pipeline statistics, D3D11 debug
   ERROR/CORRUPTION 0, 지정 rollback만 사용한다. 이미지 비교, 자동 이미지 oracle, screenshot oracle은
   사용하지 않는다. 마지막 판정은 사람이 x64 Debug Client의 실제 HWND를 눈으로 확인해 기록한다.
6. M0 rollback은 Catalog load, evaluator empty/failure, GPU composite stage, revision/device mismatch,
   sampler Undo에만 추가한다. 전역 Presentation transaction, 타 family rollback은 M1/M2로 남긴다.

### exact10/new13 Catalog 계약

source Artist 행은 기존 세 필드 뒤에
`reconstructedRenderResourceAuthorityPath`를 추가한 exact4다. runtime consumer는 outer 전체 key 순서가
exact10 또는 exact13일 때만 분기하며 optional-field 방식으로 받지 않는다.

```text
old outer exact10
  payloadKind, effectAssetId, artifactRevision, compilerRevision,
  sourceExact, runtimeExecutionAdmission, productAdmission,
  publishReceiptSha256, publishReceipt, reconstructedRuntimeProgram

new outer exact13
  old exact10 +
  renderResourcePublishReceiptSha256,
  renderResourcePublishReceipt,
  reconstructedRenderResourceAuthority

new link exact21
  schema, formatVersion, encoding, effectAssetId, programId, programVersion,
  programSha256, sidecarSchema, sidecarFormatVersion, sidecarAuthorityId,
  sidecarDecisionProjectionSha256, sidecarReceiptSha256, sidecarRawSha256,
  sidecarByteCount, sourceExact, runtimeExecutionAdmission, executeAdmission,
  submitAdmission, renderAdmission, productAdmission, sidecarUtf8Json

new receipt exact26
  schema, formatVersion, receiptRole, payloadKind, effectAssetId,
  artifactRevision, compilerRevision, sourceExact, runtimeExecutionAdmission,
  executeAdmission, submitAdmission, renderAdmission, productAdmission,
  programId, programVersion, programSha256, baseRuntimeEntryProjectionSha256,
  reconstructedRuntimeProgramSha256, basePublishReceiptSha256,
  renderResourceAuthorityLinkSha256, sidecarRawSha256, sidecarReceiptSha256,
  sidecarDecisionProjectionSha256, toolDependencies,
  receiptSha256Domain, receiptSha256

tool row exact4
  role, path, hashDomain, sha256
```

old10 첫 열 필드는 그대로 canonical projection해 기존 identity를 다시 검증한다.

```text
old outer10 canonical  e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2
old link16             74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2
old receipt self       5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3
old receipt full       92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94
```

embedded sidecar는 `sidecarUtf8Json` bytes를 한 번만 parse한다. exact root19, owner
`ARTIST/31470/F`, program/publisher tuple, 모든 false admission, `actionTimeIoAllowed=false`, 전체 self
projection과 decision projection을 검증한다.

```text
sidecar authority      ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1
sidecar bytes          746788
sidecar raw SHA        bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff
sidecar self SHA       bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a
sidecar decision SHA   4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412
program SHA            618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b
candidate raw SHA      72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849
```

old10 entry는 sidecar pointer가 null인 inspection-only entry로 계속 load된다. new13만 non-null typed
sidecar를 가진다. 실제 tracked exact13 publish는 producer source/test와 C++ Debug/Release harness가
temporary exact13 output을 함께 통과한 뒤 마지막에 atomic replace한다. 중간 실패는 기존 snapshot의
revision/status/all maps/entry/Program/sidecar pointer를 모두 보존한다.

### M0 immutable composite identity

M0 GPU composite는 다음을 하나의 staged object에 결합하고 전부 맞을 때만 atomic commit한다.

```text
shared_ptr<const CatalogEntry> exact pointer
Catalog revision
Program ID/version/SHA and candidate raw SHA
sidecar authority/raw/self/decision SHA
selected schedule/emitter/occurrence/recipe/texture/geometry/state row ID + row SHA
shader tracked identity and pass ID
exact ID3D11Device COM identity and diagnostic adapter LUID
production evaluated packet and all staged GPU resources
```

draw 직전 현재 Catalog lookup의 entry pointer/revision/hash와 composite의 값, renderer의 D3D device를
다시 비교한다. 하나라도 다르면 draw하지 않고 기존 composite를 보존한다. device context pointer는
저장 identity로 사용하지 않는다.

### M0 stable selection과 sample 동결

선택은 같은 action schedule과 같은 CatalogEntry를 사용한다.

```text
schedule ID       action-schedule-daa8fc7a3723b850ca9579f2
schedule row SHA  9d716d70a77a810f5c72e05084e3b6b9bfa0e0f5a823463ecc8af9a46352ea9f
source cue        skill-31470/clip-000/notify-022
global time       1.4506419897079468
fixed clock       60 Hz
sample step       88
sample time       1.4666666666666666
local time        0.016024676958719786
spawn serial      0
occurrence suffix ::occurrence:0000000000000000
```

Mesh는 emitter order 26이다.

```text
emitter ID        fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_17
emitter row SHA   23a6519e750f5fcdd22bd5e3f8ebd5ea63427f47440c3f2f74d60213d9977ddb
occurrence ID     source-active-026
occurrence SHA    abbd647d1068f2b17a321214c4608fe9ceb56c78aa26170299cdf13ac90190cc
recipe ID         material-recipe-4b4c59364690a66d
recipe row SHA    ef66bad94bd48c14d421ea5ae32e0fbb0dfe8cf84a4efe9aca8eeb28ac670fbf
family/evaluator  material-family-5fc89efe09353236 /
                  reconstructed-evaluator-c3ac12f104b50f06 v1
geometry use      <emitter ID>::geometry-use
geometry use SHA  ecdec710c4ca2ded8253b771936a8c1b123f694613316ad4ae705a81dddd5b71
carrier ID        geometry-carrier-d98b591aa386ac0dd11f
carrier row SHA   5b800463ed5278e9afca3b4867a661281630a131ead6987a0dd0c0baafad8744
model asset       Effect/Artist/Meshes/fm_a_stone_001.wmodel
model raw SHA     eb08b11e4631938f93b896d9ebf9e7f25d22492094dcf69de443080d5c111c54
geometryPreScale 0.01, vertex와 local bounds에 exactly once
```

Mesh texture/state 선택은 다음과 같다.

| 역할 | stable 선택 | asset / raw SHA |
|---|---|---|
| recipe texture decision | `recipe-texture-binding-06` | row `b2604680e40023ff1ef5efcbaad9e2e6a193fed6b5a70c133c31eaa87f960393` |
| texture0/base | `renderer-material-input-binding-048`, `render-binding-14`, sampler `material-reconstructed-policy-515a17c3340198bdcf21` | `Effect/Artist/Textures/fx_a_environ_002.dds` / `cff398ace89a994c044fcce3736beaa3215cb54b99b1acc105c0c2304ce55962` |
| texture1/normal | recipe가 명시 선택한 `render-binding-15`, sampler `material-reconstructed-policy-ae06d5776b669f2578ec` | `Effect/Artist/Textures/fx_a_environ_002_n.dds` / `62f18a7c49165a62a04525f5954b9c5f494a48ae68b6ce0b9ecc57803ebe63c6` |
| blend | `render-state-descriptor-12` | `BS_EffectOpaque` |
| raster | `material-reconstructed-policy-4ce70dabfda6cdc8633f` | `RS_Default` |
| depth | `material-reconstructed-policy-5c910249033a5d8fcd03` | `DSS_Default` |
| shader/pass | `Shader_VtxEffectMeshPreview.hlsl` | pass 0 `OpaqueBackDepthWrite` |

Sprite는 emitter order 27이다.

```text
emitter ID        fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_0
emitter row SHA   57228087ab6d3ffb84fe634eab1b5406666536bd140400963be873ba56e8249b
occurrence ID     source-active-027
occurrence SHA    6eac180a4d907b9bd4510161d4e200f2c9bc81280ca618f086321fcaa461fe92
recipe ID         material-recipe-2073fb45e643d1d5
recipe row SHA    f210da08033a522e3ab3e581a5535df5629a15411a6c46f5052b6c73d03202a1
family/evaluator  material-family-ee42f716afdf6145 /
                  reconstructed-evaluator-b64318cb50070e35 v1
size policy       UE3_LENGTH_XZY_0P01, world-size에 exactly once
```

Sprite texture/state 선택은 다음과 같다.

| 역할 | stable 선택 | asset / raw SHA |
|---|---|---|
| recipe texture decision | `recipe-texture-binding-01` | row `238f88b150d88389e897d27c946c63b38f512f7c91a24a81ea77c7786e72e1a3` |
| texture0/base | `renderer-material-input-binding-050`, `render-binding-03`, sampler `material-reconstructed-policy-478283291730ec2c2599` | `Effect/Artist/Textures/fx_a_decal_013.dds` / `c37194e45c9dea1b1f897c150ae0fae113431c90cd8161494f716bc705ac368e` |
| texture1/mask | `renderer-material-input-binding-049`, `render-binding-02`, sampler `material-reconstructed-policy-300f64f5c91f35575f26` | `Effect/Artist/Textures/fx_e_fluid_021.dds` / `1cf86038645760963d6a6db584283795f09fc5078b94ffe8e204ca44ed8bdc75` |
| blend | `render-state-descriptor-02` | `BS_EffectAlpha` |
| raster | `render-state-descriptor-03` | `RS_Cull_None` |
| shader/pass | `Shader_VtxEffectParticle.hlsl` | pass 1 `AlphaTwoSidedDepthRead`, 실제 `DSS_ReadOnly` context descriptor 검증 |

Sprite의 standalone `bdisabledepthtest=false` policy oracle를 `DSS_ReadOnly`와 같다고 주장하지 않는다.
translucent production pass 1을 Apply한 뒤 `OMGetDepthStencilState/GetDesc`로 실제 read-only state를
검증한다. 세 candidateCount=2 renderer decision
`renderer-material-input-binding-025/044/051`은 M0에서 전부 거부한다.

선택한 네 texture의 approved policy descriptor는 모두
`MIN_MAG_MIP_LINEAR(21), WRAP U/V/W(1), bias 0, anisotropy 0, NEVER, minLOD 0,
maxLOD FLT_MAX`다. 그러나 현재 Effects11 `LinearSampler` backing은 실제 reflection에서
`W=CLAMP, MaxAnisotropy=16, MinLOD=-FLT_MAX`이므로 이름이 같다는 이유로 안전하다고 보지 않는다.
approved state를 별도 생성해 exact sampler variable에 scope한 뒤 `GetSampler/GetDesc`와
`PSGetSamplers`를 모두 확인하고 draw 뒤 `UndoSetSampler`한다. SRGB/LINEAR은 sampler 추측이 아니라
각 DDS SRV descriptor로 별도 검증한다. clamp/wrap이 섞인 행은 M0에 넣지 않는다.

### production evaluator와 sink 구현 경계

`Effect_ReconstructedExecution.h/.cpp`의 macro 밖에 generic selected evaluator와 immutable evaluated
frame/packet을 둔다. 입력은 Plan과 stable-ID selection이며 Artist ID는 evaluator 내부에 하드코딩하지
않는다. 기존 production `Evaluate_Distribution`과 refactor한 occurrence RNG/fixed-step core를 사용한다.
packet은 raw `EFFECT_ELEMENT_DESC*`를 갖지 않고 Plan/Preparation/CatalogEntry를 강하게 보존한다.

Mesh의 ordered module 000~014와 Sprite의 000~008은 모두 stable module/handler ID로 소비한다. step 88에서
age가 0이라 update delta가 0인 module도 consumed-handler 집합에서 빠지면 실패한다. 알 수 없는 class를
정상값으로 처리하거나 rendererRuntimeConfig를 presentation fallback으로 쓰지 않는다. 결과 frame은
정확히 Mesh 1개와 Sprite 1개가 nonempty이고 같은 entry pointer를 가질 때만 commit한다.

Mesh module 009의 선택 계약도 구현 전에 다음처럼 고정한다.

```text
module ID          <Mesh emitter ID>::module:009
module row SHA     cd94729dd24af5799a9f97a48a1b27908df6e7f720dda2ea728554ed7c53b856
handler registry   handler-a90067d9043e62f6e79a5cc4
handler row SHA    4691180c80cda74a93ea57d68fe2d54d00842562c364a41192b4aeb0fe9f0a40
implementation     source.module.exact.particlemodulelocationprimitivecylinder.v1
implementation SHA dbbdcb1c7b4b28726062cc3846d89b38073fd15dd2cf2cf21869c109c757d5d7
height axis        property/literal/default row가 없는 이 선택 행은 exact handler의 Z-axis default
```

`heightaxis`가 없는 행을 매번 class 문자열로 추측하지 않는다. production dispatch table이 위 handler
registry/implementation identity와 module row를 모두 확인한 뒤 **absent exactly Z**를 적용한다. 다른
handler, 같은 module의 새 `heightaxis`, 또는 X/Y/unknown 값은 이 M0 선택 계약이 아니므로 fail closed 한다.
이는 기존 production cylinder handler의 empty-value Z 동작을 stable handler identity에 결합한 것이며,
새 implicit-default 데이터 행을 발명하지 않는다.

selected evaluator의 distribution RNG는 `artist-f.selected-occurrence-xorshift32.v1`로 고정한다. timing
core의 emitter stream과 lifetime stream은 그대로 두고, packet evaluator는 선택 occurrence의
`iOccurrenceRandomValue`를 별도 local state seed로 복사한다. 각 draw 직전에 `x ^= x << 13`,
`x ^= x >> 17`, `x ^= x << 5`, zero이면 1로 치환하고 `x / UINT32_MAX`를 random unit으로 사용한다.
ordered module 순서와 module의 `distributionIds` 순서를 따른다. operation 2는 component 0..N-1마다
정확히 한 unit을 소비해 component별 lerp를 수행하고, operation 3은 `RandomUnits[0]`용 unit 하나만 소비해
전체 minimum 또는 maximum vector를 선택한다. operation 1과 FLOAT_PARAMETER는 unit을 소비하지 않는다.
operation 2의 random-lock axis는 필요한 component draw를 모두 소비한 뒤 적용한다. cylinder handler는
radius, height, angle, surfaceOnly=false일 때만 radial,
height offset, startLocation, velocityScale 순서다. 선택 Mesh는 surfaceOnly=true이므로 radial draw가 없다.
required/lifetime/spawn module과 그 timing distribution은 fixed-step timing core가 이미 occurrence 생성,
`iLifetimeRandomValue`, lifetime, spawn step에 반영한 authority다. selected packet evaluator는 이 세 handler를
consumed 집합에 포함하고 timing packet과 identity를 검증하지만 별도 local xorshift stream에서 다시 평가하거나
unit을 소비하지 않는다. 따라서 local stream의 첫 draw는 ordered module 순서의 첫 non-timing visual distribution이다.
required module의 fixed literal도 draw를 추가하지 않는다. 선택 행의 exact local draw sequence는 다음과 같다.

```text
Mesh026 seed 2215704123
01 3224154448 0.75068195554210848 M002 startsize op2 c0
02 2530594867 0.58920003184797243 M002 startsize op2 c1
03 3064425754 0.71349222089943765 M002 startsize op2 c2
04  998511512 0.23248407808888799 M003 startvelocity op2 c0
05 2266931971 0.52781123005035591 M003 startvelocity op2 c1
06 1017363772 0.23687346192004008 M003 startvelocity op2 c2
07  730271614 0.17002960996935834 M006 acceleration op2 c0
08 2273174282 0.52926463133871193 M006 acceleration op2 c1
09 1485532676 0.34587752920246628 M006 acceleration op2 c2
10 2620743809 0.61018946804343477 M007 startrotationrate op2 c0
11  881740531 0.20529621541623402 M009 cylinder angle
12 1205524347 0.28068300971777249 M009 cylinder height offset
13 3145893313 0.73246036510273360 M009 velocityscale op2 c0
14 2355157980 0.54835294851762073 M010 startlocation op3 whole-vector selector
final state 2355157980; surfaceOnly=true이므로 cylinder radial draw 없음

Sprite027 seed 244989949
01 1708315311 0.39774815351649845 M002 startsize op3 whole-vector selector
02 3479339244 0.81009679585930350 M004 startvelocity op2 c0
03 1888920883 0.43979866510252436 M004 startvelocity op2 c1
04  572682827 0.13333811125097286 M004 startvelocity op2 c2
05 3775900606 0.87914536867270832 M007 startlocation op2 c0
06 2608091586 0.60724364281800658 M007 startlocation op2 c1
07  953103844 0.22191178151916521 M007 startlocation op2 c2
final state 953103844; procedural draw 없음
```

이 sequence에서 Mesh pre-cue packet의 size XZY는
`[1.6260229333131626,1.5702383313491564,1.3838000477719588]`, acceleration은
`[0,-11.308244941595067,0]`, rotation rate는 `79.33641699127304 deg/s`, local position은
`[1.0635928511667767,0.6978068300971776,-0.28824338214256146]`, velocity는
`[0.18857087353033217,2.231900362239327,-0.653603293505097]`, handler RGBA는 implicit alpha
identity를 적용한 `[0,0,0,1]`이다. Sprite packet의 signed world-size XZY는 `[-0.5,0,0.4]`,
velocity는 `[0.8100967958593035,0.28666618887490275,0]`, local position은
`[3.5165814746908333,0.1,-0.10724364281800661]`, color는
`[0.009999999776482582,0.10000000149011612,0.20000000298023224,1.5]`, dynamic parameter는
`[1,0.5,1,1]`이다.

이 순서와 RNG version은 packet projection에 들어가며 legacy UE LCG나 timing emitter stream을 재사용하지
않는다. step 88 serial 0의 timing identity는 Mesh occurrence/lifetime RNG
`2215704123/2215704123`, lifetime `2.257941908612368`; Sprite는
`244989949/244989949`, lifetime `0.6228164894929623`로 고정한다.

size basis는 source `[x,y,z]`를 client XZY `[x,z,y]`로 한 번만 바꾼다. Mesh packet은 이 값을
dimensionless scale로 보존하고 `.01`을 곱하지 않는다. Sprite packet은 XZY에 `.01`을 한 번 곱한 signed
world-size를 보존하며 production quad의 width/height는 packet X/Z, 즉 source X/Y를 사용한다. 음수 X는
승인된 image flip을 보존하고 절댓값 fallback으로 지우지 않는다. sink는 이 값을 다시 reorder하거나
centimeter 변환하지 않는다.

두 material evaluator의 production common-shader binding도 다음처럼 고정한다.

| 선택 | evaluator/family identity | feature mask | texture lane / shader pass |
|---|---|---:|---|
| Mesh 026 | `reconstructed-evaluator-c3ac12f104b50f06`, family row `b95ca3e38af0bab700b1941c9f34e7c1819fd11eafb8e4ae22bcd0dd374ab43b` | 41 | decision 06의 texture0/1 -> `g_SourceTexture0/1`, Mesh pass 0 |
| Sprite 027 | `reconstructed-evaluator-b64318cb50070e35`, family row `78577bca3d6ff10f53428196606c79548b3b1d52cc0594e469f701a5ced8c568` | 811 | decision 01의 texture0/1 -> `g_SourceTexture0/1`, Particle pass 1 |

packet은 `g_ReconstructedMaterialEvaluatorEnabled`, exact feature mask, `g_ReconstructedUVScale`,
`g_ReconstructedPanRotationAux`, `g_ReconstructedColor`, `g_ReconstructedParams0/1`에 해당하는 typed block을
소유한다. HLSL은 이미 WARP oracle이 검증한
`SECOND_TEXTURE_MULTIPLY -> UV_TRANSFORM_PHASE -> PANNER_PHASE -> COLOR_MULTIPLY -> DESATURATION ->
SIGNED_POWER -> FRESNEL_GAIN -> DISTORTION_OFFSET -> DISSOLVE_ALPHA -> ALPHA_MULTIPLY` 순서만 production
common path에 옮긴다. legacy `g_SourceMaterialProfile`이나 base/noise/mask fallback으로 두 family를 추측하지
않고, evaluator/family/recipe/decision/lane identity가 하나라도 다르면 stage를 rollback한다.

GPU stage는 기존 `CEffectDocumentRenderer`가 소유한다. model은 `CModel` 통합 경로로만 만들고 carrier
preScale을 create/pretransform에서 한 번만 적용한다. Sprite particle은 production particle instance
buffer와 shader를 사용한다. diagnostic placement root만 실제 camera 앞의 visible 위치를 제공하며 packet의
source local transform을 대신 만들지 않는다.

각 DDS는 안전한 Resources-relative path에서 immutable byte vector로 한 번 읽는다. **그 같은 vector**를
SHA-256 검사와 `CreateDDSTextureFromMemoryEx`에 넘기고 SRV format/dimension/mips를 검사한다. stage 뒤에는
DDS, WModel, candidate, sidecar를 reread하지 않는다. M0 smoke는
`LOSTARK_RESOURCE_ROOT=C:\Users\user\Desktop\LostArk\Client\Bin\Resources`를 사용한다. 이 경로는
AGENTS가 팀장 관리 physical runtime input으로 지정한 유일한 dirty-main read-only 예외다. source/code/Data는
이 root에서 읽지 않으며 selected 1 WModel과 4 DDS 이외를 M0 authority로 세지 않는다. M0는 이 root를
수정하지 않는다.

Engine `CShader`에는 exact sampler variable `LinearSampler`의 최소 Set/Undo wrapper만 추가한다.

```text
SetSampler(0, approved state)
-> Begin(exact pass) / FX Apply
-> approved state GetDesc + PSGetSamplers actual COM/descriptor
-> OMGetBlendState + RSGetState + OMGetDepthStencilState actual descriptor
-> production draw
-> UndoSetSampler(0)
-> prior PS sampler restore 또는 original pass re-Apply
```

Set/Apply 뒤, draw 전, draw 실패를 포함한 모든 경로에서 Undo와 이전 context sampler 복원을 수행한다.
Undo 실패면 M0 draw 성공으로 기록하지 않고 composite를 deactivate하며 이전 composite를 보존한다.

### 검증과 checkpoint 순서

1. 이 final plan-only commit을 push하고 reviewer에게 exact SHA와 `PLAN only` scope를 보낸다.
2. exact13 producer/consumer는 `ab559784` PASS를 fresh-LF worktree에서 재검증한다. schema/hash를 바꾸는
   corrective가 필요하지 않으면 새 구현 checkpoint로 다시 만들지 않는다.
3. `18d2b489` selected evaluator를 독립 판정하고, PASS 뒤 harness-only request builder를 하나의 production
   Program/sidecar-derived generic diagnostic factory로 factor한다. M0 selector data는 Tool input일 수 있지만
   production factory의 class/count/hash authority가 될 수 없으며 M1 전에 Artist-specific C++ fixture가 0인지
   audit한다. macro 없는 actual-catalog harness에서 step 88 Mesh/Sprite 2/2,
   module-handler consumption24/unique route15, determinism, empty/failure preservation을 통과시키고 frozen SHA를
   재감사한다.
4. existing renderer/Object/Effect Tool에 selected composite, production Mesh/Sprite sink, sampler scope,
   draw statistics와 D3D debug gate를 연결한다. catalog revision/device mismatch와 GPU stage/Undo failure에서
   이전 composite 보존을 검증한다. commit/push 뒤 exact SHA/scope만 보낸다.
5. `git diff --check`, JSON/XML parse, focused Python/PowerShell audit, ClientFrontendHarness Debug/Release,
   Engine Debug/Release -> UpdateLib Debug/Release -> Client x64 Debug build, ProjectAudit를 실행한다.
6. `Client/Default` cwd와 위 Resources root에서 x64 Debug Client를 실행하고 F1 Effect Tool의
   nonProduct M0 diagnostic/solo를 사용한다. pipeline query는 두 production draw의 IA vertex/primitive,
   VS/PS invocation이 모두 nonzero인지 확인하고, ID3D11InfoQueue의 새 ERROR/CORRUPTION이 0인지 확인한다.
   자동 gate가 모두 PASS한 뒤 사람이 실제 Client 창의 픽셀을 눈으로 확인해 수동 결과를 기록한다.

M0에 직접 필요하지 않은 새 provenance/audit framework는 만들지 않고 M1~M3 backlog에만 기록한다.
Presentation provider-local mixed batch, Light/Post, Decal/Ribbon, sampler72 전체, visual33 전체,
PlayerSkills/animevent actual F, Product, R6/R7, 타 class/Valtan은 M0 구현 파일과 결과에 포함하지 않는다.

## 이번 재개판의 핵심 판단

Artist F M0~M3는 fresh 구현 세션 하나가 shared runtime을 끝까지 직렬 소유한다. 사용자가 여러 작업의
진행률을 해석하거나 서로 다른 implementation branch의 결합 순서를 관리하지 않게 한다. 별도 agent/task는
frozen SHA의 독립 review만 수행하고 코드를 수정하지 않는다.

Source/Material evidence 때 유효했던 상시 specialist 병렬 모델은 여기서 종료한다. Catalog, evaluator,
Object, Renderer, Shader, Effect Tool, Presentation은 같은 immutable pointer와 admission graph를 공유하므로
둘 이상의 writer가 나누면 interface drift와 반복 build가 더 커진다. M3 동결 뒤 R8 raw corpus 적용만
data-only lane으로 병렬화할 수 있으며, generic C++ 변경이 필요하면 즉시 단일 writer로 되돌아간다.

이 결정은 independent review를 줄이는 것이 아니다. reviewer는 각 frozen checkpoint에서 과거처럼
coordinated reseal, identity laundering, rollback, nonfinite/bool, empty-frame/no-pixel success를 공격하되,
live WIP를 계속 따라가거나 구현자의 파일을 고치지 않는다. 구현자는 reviewer의 재현을 같은 branch의
corrective commit으로 닫고 새 exact SHA를 다시 제출한다.

## 목표와 종료 증거

도화가 `31470 / 필법 : 한획긋기 / F`의 7 cue와 35 active occurrence를 하나의
Source Contract에서 immutable typed Cascade IR, geometry와 material binding, 여섯 renderer,
Effect Tool의 동일 prepared revision, Product Catalog까지 연결한다.

이 작업의 종료는 문서 생성이나 scaffold build가 아니다. 다음 조건을 모두 만족해야 한다.

```text
Source denominator        7 cue / 35 occurrence / 399 ordered module reference
Source property           1,434 top-level tagged property / 1,572 primitive leaf / 629 distribution
Renderer denominator      Mesh 13 / Sprite 16 / Decal 3 / Ribbon 1 / Light 1 / ScreenPost 1
Local reference           15 definition / 17 occurrence / PointLight 1 occurrence
Geometry                  7 WModel carrier / 13 Mesh occurrence
Material                  27 unique recipe / 34 rendered material occurrence / builtin Light 1 separate
Runtime authority         immutable compiled IR only
Silent fallback           0
Unknown evaluator run     0
Missing material fallback 0
Double geometry scale     0
Stale generated hash      0
Failed transaction leak   0
Product admission         35/35, atomic cue publish
```

이미지 기반 자동 판정은 종료 증거로 사용하지 않는다. 대신 자동 numeric/structural gate 뒤 사용자의
실제 Client 육안 확인과 occurrence별 `APPROVED`는 필수 종료 증거다. fixed seed, fixed timestep,
명시적인 ActionCue 입력과 sample time, numeric tolerance를 가진 oracle와 컴파일·링크·실행 오류 검증이
수동 판단보다 먼저 PASS해야 하며, 육안에서 발견된 차이는 overlay와 fixture로 환류해 재검증한다.

## 현재 실제 기준점

R0/R1 evidence acquisition 뒤 사용자는 reconstructed high-ceiling 경로를 승인했고, R2 immutable
candidate/parser/catalog와 CPU plan compile은 완료됐다. source-era actual-output/state provider는 여전히
없으므로 historical blocker와 `sourceExactAdmission=false`를 보존한다. `4ffe1102`는 폐기했으며 현재
runtime program은 그 checkpoint를 재사용한 것이 아니다. 선택 production evaluator는 구현됐지만 독립
review 중이고 production caller가 없으므로 GPU/Tool/actual F/Product admission은 모두 0이다.

| 영역 | frozen checkpoint | 독립 판정과 현재 사용 정책 |
|---|---|---|
| Geometry evidence/resource | `0aca792819fdda3f541bb7cec7451c5ed93c6467` | PASS. 7 WModel v1.1 physical deploy와 7/7 decode를 재사용하되 runtime preScale consumer는 아직 0/7 |
| Material evidence/runtime corrective | `cde8f3bddea2f9415f682b387d2705fd25794075` | `627ddc76`, `ab76b7ec`, `d39097c3`의 admission/semantic validation gap을 supersede. evidence integrity PASS, source value는 render 0/89·static 23/94·sampler 0/72, execution readiness 0/255 BLOCK. exact sampler 0, rejected legacy 4, Product false |
| Generic compiler | `c4b00f14b32d27604ac677e9a9ea81b01ecaa551` | non-executable inspection core 범위 PASS, Product false |
| Runtime candidate/parser/catalog | `ab559784a7fa0c5d19ccedf600d08f8a9ee50a25` | fresh-LF exact13 PASS. immutable Program/sidecar pointer와 rollback을 재사용하며 admission false |
| CPU typed plan | `7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2` ancestry | 7/35/399/629 compile와 exact semantic projection PASS. production evaluation proof는 아님 |
| selected production evaluator | `18d2b48920b2a327ac59b572960325d352e77a6f` | Mesh/Sprite 2행, module-handler consumption24/unique route15 구현, 독립 review 중, production caller/GPU sink 없음 |
| offline render-resource authority | integration ancestry의 audited sidecar | DDS48/binding72와 recipe/state evidence만 재사용. current FX11 sampler 불일치 때문에 render authority로 자동 승격 금지 |
| Source execution corrective | `c927e397811d4e5718efd27b187eb59775023685` | evidence integrity PASS, module 370 ready/29 blocked. current-only evidence를 source exact로 승격하지 않음 |
| Source provider acquisition | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | accessible-scope evidence PASS, execution readiness 0/29 BLOCK. 29 occurrence→15 class→7 native family, provider/pilot 0, VSS `PERMISSION_UNCHECKED`, Product false |
| Source runtime materializer | `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` | 폐기한 BLOCK checkpoint. current candidate의 parent나 runtime authority로 사용하지 않음 |

현재 frozen R1 검증의 사실은 다음과 같다.

```text
Source acquisition mutation                21/21 PASS
Source actual-output readiness             0/29 BLOCK
Source ownerless row                       0
Geometry physical resource                 7/7 deployed and decoded
Material source-value acquisition          12/12 PASS
Material runtime mutation                  23/23 PASS
Material WARP oracle plumbing              numeric 200 / state pilot 4 PASS
Material source value                      render 0/89, static 23/94, sampler 0/72
Material execution readiness               0/255 BLOCK
Selected evaluator admitted                0 until frozen review PASS
Production evaluator caller                0
GPU typed sink                             0/6
Actual Artist F                            0/35
Runtime execution admission                false
Product admission                          0/35
```

위 WARP PASS는 evaluator/state consumer plumbing의 재현성만 증명하며 source-era 값을 제공하지 않는다.
Source full audit의 lane 외 14건과 Material acquisition 이전 full audit의 lane 외 12건도 전체 green으로
세탁하지 않는다. 현재 병목은 더 이상 artifact 조사만이 아니라 production caller, immutable GPU composite,
typed sink 6개와 actual action 연결이다. source-era provider 부재를 current default/fallback으로 덮는 것도,
구현되지 않은 seam을 receipt boolean만으로 여는 것도 금지한다.

## 고정 fidelity와 admission 계약

모든 definition, occurrence, resource binding은 다음 네 축과 네 blocker set을 독립적으로 가진다.

```text
sourceFidelity
  SOURCE_EXACT, CURRENT_REVISION_EVIDENCE, RECONSTRUCTED_NUMERICALLY_VERIFIED,
  RECONSTRUCTED_GRAPH, UNRESOLVED를 구분한다.

artifactBindingIntegrity
  compiled expected identity와 실제 payload bytes, package/export/record, model/material hash가
  일치하는지를 판정한다. self-signed container hash만으로 true가 되지 않는다.

executionAdmission
  payload를 읽기 전에 blocker가 0이고 해당 opcode/evaluator/renderer handler와 numeric oracle이
  존재할 때만 true다.

productAdmission
  execution, geometry, material, renderer, Tool/Catalog transaction과 최종 regression이 모두
  통과한 occurrence만 true다. 35개 중 하나라도 false면 cue 전체를 publish하지 않는다.

evidenceBlockers
  historical source fidelity가 닫히지 않은 이유다. verified reconstruction이 성공해도 삭제하지 않는다.

artifactBindingBlockers
  expected package/export/record/model/material identity와 실제 bytes가 다를 때만 유지한다.

executionBlockers
  opcode/evaluator/default/resource/renderer의 실행 의미가 닫히지 않았을 때 유지한다.
  numeric reconstruction proof는 이 집합의 명시된 blocker만 해소할 수 있다.

productBlockers
  execution, prepared transaction, runtime resource, regression이 제품 조건을 충족하지 못할 때 유지한다.
  evidenceBlockers가 남아 있다는 이유만으로 자동 생성하지 않고 G00의 허용 fidelity matrix로 계산한다.
```

`SOURCE_EXACT`는 완화하지 않는다. historical source가 없는 target을 current Constant나 class
이름만으로 exact 또는 executable로 바꾸지 않는다. `RECONSTRUCTED_NUMERICALLY_VERIFIED`는 별도의
evaluator ID/version, oracle provenance, 입력 영역, expected sample, tolerance, 독립 구현 비교를
모두 가진 경우에만 허용한다. `evidenceBlockers`는 이 승격 뒤에도 보존하되 독립적으로 검증된
reconstruction은 대응 `executionBlockers`만 해소한다.

maximum-reconstruction admission branch는 사용자의 별도 명시 승인으로
`RECONSTRUCTED_APPROVED_V1_POLICY_CONSTRUCTION` 상태다. Material의 기존 23 arithmetic evaluator와
WARP replay만으로 source value나 execution row를 자동 승격하지 않는다. Source 29행과 Material 255행은
각각 explicit policy row와 실제 consumer capability가 생긴 경우에만 reconstructed execution candidate에
포함한다. policy/capability가 없는 current default, class 이름, 암묵적 texture/state fallback은 계속
실행 또는 Product admission에 사용할 수 없다.

Product predicate는 마지막에 수동으로 정하지 않는다. 네 축, 네 blocker set, 허용 fidelity matrix로
G00에서 고정하고 G11은 같은 predicate를 평가만 한다. 서로 다른 blocker set을 하나의 union으로
합쳐 source exact가 아닌 행을 영구 Product false로 만드는 방식은 금지한다.

## 세션과 worktree 구조

M0~M3의 active implementation writer는 하나다. 이 plan-only SHA에서 fresh worktree를 만들고
`18d2b489` evaluator 판정/교정 -> production request factory -> immutable GPU composite -> M0 first pixel ->
full 35 evaluator/resource/sink -> actual F Debug -> Product D/R 순서로 같은 branch를 전진시킨다.

reviewer는 frozen commit이 push된 뒤에만 exact tree를 새 worktree에서 읽는다. 구현 세션은 reviewer가
끝날 때까지 다음 admission을 열지 않지만, unrelated 문서/추가 framework를 만들며 시간을 보내지 않는다.
reviewer는 PASS/BLOCK과 재현 입력만 보내고 파일을 수정하지 않는다.

R8에서는 4-class raw/data 적용과 Valtan raw/data 적용을 서로 다른 worktree에서 병렬화할 수 있다.
단 shared compiler/evaluator/renderer/header 수정은 implementation captain 한 명에게 반환한다.

세션 간 메시지는 target task의 최신 상태를 먼저 읽은 뒤 다음 네 종류만 사용한다.

1. `ADD`: 현재 plan/checkpoint에 새로 필요한 입력이나 증거
2. `CORRECT`: exact SHA에서 재현한 P0/P1과 최소 교정 범위
3. `STOP`: admission 또는 destructive action을 즉시 막아야 하는 조건
4. `REVIEW_EXACT_SHA`: parent/SHA/owned files/tests/declared scope가 완결된 frozen review 요청

Checkpoint 보고는 다음 필드를 모두 가진다.

```text
base SHA
commit SHA
owned files
public contract changes
tests actually executed
Product admission
remaining blockers
integration order
```

## 파일과 단일 소유자

아래 표의 과거 absolute worktree prefix는 기록용이며 실행 경로가 아니다. fresh 구현 세션은 자기 clean
repository root를 `$ARTIST_F_WORKTREE`로 두고 표의 repository-relative suffix만 사용한다. 다른 task의
worktree나 main dirty worktree에서 파일을 읽어 build/PASS를 만들지 않는다. 단 앞에서 명시한 team-managed
physical `Client/Bin/Resources` read-only 예외는 허용한다.

| 구분 | 절대 경로 | 단일 소유자와 역할 |
|---|---|---|
| 통합 | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/.md/GB/08-10/2026-08-10_ARTIST_31470_F_COMPLETE_RESTORATION_IMPLEMENTATION_PLAN.md` | Integration Captain의 전체 G와 gate 정본 |
| Source schema | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_AuthoringDocument.h` | G00 Integration만 public transport를 변경하고 이후 additive request를 중재 |
| Source Codec | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentCodec.cpp` | G00 Integration과 Source reviewed bridge만 수정 |
| Source generator | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py` | Source Closure 단일 소유 |
| Source outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Candidates` | Source Closure가 candidate/receipt를 단 한 번 재생성 |
| Source registry | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Contracts/ue3-cascade-source-v1.registry.json` | Source Closure 단일 소유 |
| Geometry evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ModelAssetConverter` | Geometry Evidence lane이 decoder/cooker/golden만 소유 |
| Geometry Engine | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Private/BinaryAsset/Winters` | Geometry Evidence lane이 WModel decode contract를 소유 |
| Material evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py` | Material Evidence lane 단일 소유 |
| Material outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Materials` | Material Evidence lane receipt/recipe 단일 소유 |
| Compiler | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_CascadeCompiler.h` | G04가 inspection core를 만들고 G06이 execution IR shape/admission을 최종 동결 |
| Compiler implementation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_CascadeCompiler.cpp` | G04 inspection adapter 뒤 G06만 execution adapter/opcode/receipt를 최종 수정 |
| Executor | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Playback.cpp` | G07만 finalized execution IR 소비와 Mesh hidden scale 제거를 소유 |
| Prepared render | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | Renderer Foundation이 common dispatch/cache/upload를 소유 |
| Presentation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_PresentationService.cpp` | Prepared/Catalog transaction lane이 exact revision attach를 소유 |
| Effect Tool | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Tool.cpp` | Tool lane이 read-only Source mode와 Draft/Product Play 경계를 소유 |
| Common shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli` | Renderer Foundation 단일 소유; family lane이 수정하지 않음 |
| Light public payload | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Public/Engine_Struct.h` | G06이 radius/falloff/color/brightness typed payload를 소유 |
| Deferred Light shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | G06이 typed falloff 소비와 attenuation contract를 소유 |
| Shared registrations | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Default/Client.vcxproj` | 필요한 lane commit에 additive 등록하고 Integration이 충돌 해결 |
| Audit registration | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | Integration이 gate별 focused audit를 최종 등록 |

Source candidate는 실제로 직접 pin하는 Source/geometry/active-material-closure 입력만 재생성한다.
새 typed MaterialRecipe 전체를 Source candidate에 다시 복제하지 않는다. 최종 compiled receipt가 Source,
Geometry, Material contract hash와 compiler revision을 함께 결합해 불필요한 hash regeneration cycle을
막는다.

## 현재부터 완성까지의 실행 순서

아래 순서는 기존 G 전체를 다시 시작하는 계획이 아니라 R0 evidence PASS 뒤 execution-readiness를 열어
PASS된 Runtime foundation `38ebe7cf` 위에 최종 pipeline을 결합하는 critical path다.

### R0. 35/35 feasibility와 blocker owner 동결 — matrix PASS, execution BLOCK

Playback과 renderer 구현 전에 Source 29개 blocked module과 Material 255개 execution row를 행 단위로
지도화했다. 이 gate는 문서상 owner 이름만 채운 것이 아니라 각 행의 acquisition/oracle 경로와 실패
판정을 frozen receipt로 고정했다.

Source feasibility matrix의 각 행은 다음을 가진다.

```text
moduleOccurrenceId
exactSourceClass
family
requiredRuntimeOutputs
sourceEraPackageOrBinaryIdentity
currentRevisionEvidenceIdentity
nativeEntryOrDispatchIdentity
numericOracleInputDomain
numericOracleExpectedOutput
independentOracleImplementation
oracleProvider
pilotFixtureIds
pilotExpectedMutatedOutputs
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Source safe denominator는 29 blocked module이다. standard seeded 11, EF custom 15, EF multiply
distribution owner 3을 임의로 합치거나 denominator에서 제거하지 않는다. input digest parity는 output
oracle가 아니다. source-era identity 또는 실제 native particle output 비교가 없으면 READY로 승격하지
않고 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`와 blocker를 유지한다. 각 family는 최소 하나의
실제 pilot fixture로 particle/component output 변화와 tolerance를 증명해야 하며 `FEASIBLE`,
`VERIFIED_IRRELEVANT`, `BLOCKED` 중 하나로 판정한다.

Material feasibility matrix의 각 행은 다음을 가진다.

```text
materialRecipeId
materialOccurrenceIds
fieldId
fieldKind
bindingOriginAndOwner
instanceRecordIdentity
parentOrCdoIdentity
staticOrShaderMapIdentity
rendererConsumption
acquisitionPath
oracleProvider
pilotFixtureIds
numericOracleInputDomain
numericOracleExpectedOutput
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Material safe denominator는 27 recipe/34 occurrence, 162 render-state field 중 explicit 73/unresolved 89,
static input 94, strict sampler 72다. 이전 exact sampler 3 instance+1 parent는 texture binding/DDS identity만
exact하며 source-era Texture2D CDO와 TextureGroup filter configuration이 없는 full descriptor는 증명하지
못하므로 strict sampler에 포함한다. render-state 89, static 94, sampler 72를 별도 matrix로 유지하며
validator correction과 혼동하지 않는다.
omitted field는 instance -> Parent Material -> nested default -> class CDO 순서로만 해석한다. D3D
blend/depth/cull state, static permutation, sampler address/filter/sRGB를 offscreen WARP 또는 독립 state
oracle의 작은 pilot로 실제 관측하지 못한 행은 BLOCK으로 유지한다. source-revision ShaderCache 또는
controlled runtime capture가 필요한 행은 acquisition provider가 실제로 동작하기 전까지 FEASIBLE로도
승격하지 않는다.

Frozen 결과는 source value와 execution readiness를 분리한다. Material static 23행은 source MIC의 exact
`bOverride=true` 값을 확보했지만 actual state-output pilot과 final consumer가 없으므로 READY가 아니다.
WARP 200 numeric sample과 state pilot 4건도 source 값 provenance가 아니라 oracle plumbing 검증이다.

```text
Source matrix integrity                    PASS, ownerless 0
Source execution readiness                 0/29 BLOCK
Material matrix integrity                  PASS, ownerless/unknown 0
Material source value                      23/255
Material execution readiness               0/255 BLOCK
VSS                                        PERMISSION_UNCHECKED
NVIDIA DXCache                             548/561 readable, 13 share-locked
global exhaustion claim                    false
```

R0 evidence-integrity 합격 조건:

- Source blocked 29/29, Material render-state 89/89, static 94/94, sampler 72/72가 matrix에 존재한다.
- owner 없는 행 0, silent fallback 0, denominator shrink 0이다.
- 각 행/family는 필요한 expected output과 tolerance 계약을 기록하며 provider, pilot 또는 tolerance가
  없으면 `BLOCKED`로 보존한다.
- READY 후보는 source-era evidence 또는 실제 expected mutated output과 tolerance를 고정한 독립
  actual-output/state oracle pilot을 가진 경우에만 존재할 수 있다.
- current-only/cross-revision 행을 `SOURCE_EXACT`로 승격한 행 0이다.
- acquisition이 불가능한 행은 즉시 명시되어 Artist F 35/35의 hard blocker로 보고된다.
- R0 matrix와 resolver/validator mutation test가 frozen review PASS하기 전 R2 이후 shared runtime 구현을 시작하지 않는다.

위 evidence-integrity 조건은 `7da937ae`와 `cde8f3bd`에서 충족했다. 이는 원본 의미 복원 완료가 아니다.
execution readiness는 Source 0/29, Material 0/255이므로 별도 BLOCK이며 R2 이후를 열지 않는다.

### R1. Source/Material provider acquisition — evidence PASS, execution BLOCK

Source `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3`은 접근 가능한 filesystem/backup/current install/
Git remote·LFS·unreachable object 범위를 조사하고 29 occurrence를 15 exact class와 7 native family로
축약했다. source-era actual-output provider와 standalone pilot은 각각 0이고 blocker delta는 `29 -> 29`다.
current wrapper/dataflow와 fixed input digest는 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`로만 보존한다.

Material `cde8f3bddea2f9415f682b387d2705fd25794075`는 `627ddc76`의 잘못 승인된 exact sampler 4건,
`ab76b7ec`의 row/evaluator validator gap, `d39097c3`의 acquisition/runtime top-level schema gap을
교정하고 typed contract v4와 runtime receipt v3를 재봉인했다. render-state source value는 0/89,
static은 exact override value 23/94, sampler full descriptor는 0/72다. static nonoverride semantics 미확정
43행과 GUID join 없음 28행을 분리하며 execution readiness는 전체 0/255다.

두 lane 모두 VSS는 비관리자 권한으로 `PERMISSION_UNCHECKED`이고 Material GPU cache는 NVIDIA 561개 중
13개가 share-locked다. 접근 가능한 범위는 소진했지만 `globalExhaustionClaim=false`다. safe source-era
standalone runtime/UCC/commandlet/debug capture 경로가 없으며 injection/hooking/anti-cheat 우회는 시도하거나
허용하지 않는다. current capture는 가능해져도 `CURRENT_REVISION_OBSERVED`일 뿐 source-era fidelity를 닫지 못한다.

아래 표는 reconstructed approval 직후의 historical R1 판정이다. 현재 구현 상태는 앞의
`2026-08-11 고점 복원 최종 결정 정본` 표를 사용한다.

```text
Source acquisition evidence                PASS_ACCESSIBLE_SCOPE_EXHAUSTED
Source actual-output execution readiness   0/29 BLOCK
Material evidence integrity                PASS
Material source value                      23/255
Material execution readiness               0/255 BLOCK
ownerless/unknown row                       0
typed materializer                         historical BLOCK at R1
reconstructed branch                       RECONSTRUCTED_APPROVED_V1 / POLICY_CONSTRUCTION
Product admission                          false / 0/35
R2                                          historical IN_PROGRESS, now transport PASS
R3-R8                                       current milestones M0-M3/R8로 superseded
```

source-era exact 경로의 R2 진입 predicate는 Source 29/29, Material render 89/89·static 94/94·sampler
72/72, unresolved execution row 0으로 유지한다. 사용자가 승인한 reconstructed 경로의 R2 진입
predicate는 frozen evidence-integrity PASS, `sourceExactAdmission=false`, 29+255 denominator 보존,
explicit per-row policy/capability/oracle owner, silent fallback 0이다. Corrected materializer는 이 정책을
입력으로 R2에서 새로 작성하고 R2 종료 시 독립 PASS해야 한다. R3 진입은 materializer의 field coverage
100%, unknown/ownerless 0, 29+255 reconstructed capability receipt와 Debug/Release parser mutation PASS를
추가로 요구한다.

### R1 재개 입력 계약

`SOURCE_EXACT` 재개에는 같은 revision의 `EFEngine.dll`, `LOSTARK.exe`, `Engine.u`, `Core.u`, `EFGame.u`,
target UPK, ShaderCache/material map, SystemSettings TextureGroup config와 single-revision identity manifest가
필요하다. 대안은 동일 build identity와 fixed seed/time/world/parameter input, pre/post full numeric state,
expected output과 tolerance를 가진 인증된 source-era capture다.

최대 reconstruction 분기는 사용자 승인으로 시작한다. 행별 explicit project policy와 independent
same-input output/state oracle을 통과한 값만 versioned `RECONSTRUCTED_APPROVED_*` 또는
`RECONSTRUCTED_NUMERICALLY_VERIFIED`로 표시하며, evidence blocker, `sourceExactAdmission=false`,
Product false를 보존한다. 이미지 비교는 단독 numeric admission 입력이 아니지만 사람 눈 검증은 필수다.
수동 smoke에서 발견한 차이는 occurrence별 tuning overlay와 numeric/structural fixture로 환류한다.

이하 historical R2~R8의 denominator와 failure predicate는 유지하되 실제 진행 순서는 앞의 M0~M3/R8을
정본으로 사용한다. R2 transport는 완료됐고 M0 selected evaluator가 독립 review 중이다.

### R2. 승인 checkpoint 통합과 final typed schema 동결 — transport PASS

아래 1~7은 historical construction 기록이다. 현재 fresh-LF candidate/parser/exact13 Catalog transport는
PASS했으며 current implementation parent는 `18d2b489`다. 이 절을 다시 cherry-pick하는 실행 목록으로
사용하지 않는다.

1. clean final-integration worktree를 PASS된 Runtime foundation `38ebe7cf`에서 만든다.
2. Geometry는 조상에 이미 포함된 evidence commit을 재적용하지 않고 `git cherry-pick -x 669acf07`,
   이어서 `git cherry-pick -x 0aca7928`을 각각 실행한다. `669acf07..0aca7928` range는 첫 commit을
   누락하므로 사용하지 않는다.
   Compiler `c4b00f14`와 Publisher `c90c462`는 patch-equivalent 조상에 이미 포함되므로 재적용하지 않는다.
3. Source는 `ae425aaf -> 9b046d61 -> c927e397 -> 7da937ae (evidence PASS, readiness 0/29)
   -> <future 29/29 readiness>` 전체 검증 단위를, Material은 `532d1f52 -> b6757a21 -> e54a5a2a
   -> 627ddc76 (superseded) -> ab76b7ec (validator BLOCK) -> d39097c3 (top-level schema BLOCK)
   -> cde8f3bd (evidence PASS, readiness 0/255)
   -> <future 255/255 readiness>`
   전체 검증 단위를 적용한다. 잘못된 중간 READY 상태에서는 build, publish, admission을 수행하지 않는다.
4. BLOCK materializer `4ffe1102`는 cherry-pick하지 않는다. final Source/Material schema를 받은 combined
   head에서 corrected materializer를 새로 작성한다.
5. Source 35/399/629, Geometry 7, Material 27/34를 결합한 six-hash compiler input을 한 번 생성한다.
6. exact class, opcode, distribution evaluator, renderer, GeometryBinding, MaterialBinding capability ID와
   version을 동결한다.
7. blocked payload는 읽기 전에 거부하고 runtime fallback은 0으로 유지한다.

`native-v14.source-contract-candidate.effect.json`, 그 receipt, `4ffe1102` runtime-program candidate는 현재
R1 readiness를 반영하지 않은 stale 산출물이므로 재사용하지 않는다. 재생성은 Source/Material readiness
receipt -> Source candidate/receipt -> corrected runtime program -> six-hash compiler input -> derived
Assembly/artifact -> Catalog/prewarm 순서로 정확히 한 번 수행한다.

R2 종료 조건은 corrected materializer independent PASS, field loss 0, stale output 0, unknown capability 0,
ownerless blocker 0이며 Product false다.

### R3. Actual typed executor와 resource consumer — M0/M1 remaining

이 구간은 shared C++ dependency가 촘촘하므로 fresh implementation captain 한 명이 다음 순서로 직렬 구현한다.

1. Playback이 immutable compiled program pointer만 소비하게 하고 raw SourceRecipe/module 문자열 scan을 0으로 만든다.
2. standard distribution, fixed seed/time, ActionCue, spawn/lifetime/location/velocity/size/color/SubUV와
   TypeData handler consumption receipt를 연결한다.
3. GeometryBinding expected tuple을 `CModel -> CMesh -> CMaterial`에 연결한다.
   `geometryPreScale=0.01`은 vertex와 bounds에 정확히 한 번 적용하고 Mesh StartSize hidden `x0.01`을 제거한다.
4. MaterialBinding 27 recipe/34 occurrence를 typed evaluator와 HLSL에 연결하고 explicit render state만 소비한다.
5. post-compile raw semantic I/O와 재compile은 0이며 실패 시 이전 prepared pointer/cache를 보존한다.

R3 종료 조건은 399 opcode/629 distribution consumption, Geometry 7/7, Material 34/34, deterministic
fixed seed/time, fallback 0, Debug/Release PASS다. Product는 아직 false다.

### R4. 여섯 renderer family — typed sink 0/6 remaining

공통 render packet, vertex layout, MaterialBinding slot, family dispatch interface를 fresh captain이 먼저
동결하고 같은 captain이 여섯 family를 직렬 연결한다. Artist F M0~M3에서는 family implementation을
병렬화하지 않는다.

1. Mesh + Sprite: 13 + 16
2. Decal + Ribbon: 3 + 1
3. Light + ScreenPost: 1 + 1

각 family는 같은 compiled IR pointer를 소비하고 size/rotation/local-space/timing/material/render-state
numeric packet을 검증한다. raw `eKind`, raw SourceRecipe, legacy heuristic dispatch는 Product path에서 0이다.

### R5. Effect Tool dual mode, actual Artist F와 35/35

1. 기존 `CEffect_Tool` 하나에 Legacy Product와 Typed Restoration 두 mode를 둔다.
2. v14 Source Contract와 compiled IR은 read-only이고 조정은 occurrence-keyed overlay에만 저장한다.
3. Typed Play는 Authored disk reload가 아니라 exact catalog prepared revision을 attach한다.
4. publish -> catalog load -> prewarm -> equivalence -> target commit을 하나의 transaction으로 묶는다.
5. 35 occurrence 각각에 source/compiled identity, opcode, distribution, geometry, material, renderer,
   fixed sample expected/actual/tolerance를 기록한다.
6. `35/35`, Geometry `7/7`, Material `34/34`, fallback/blocker `0`일 때만 cue 전체 Product를 atomic publish한다.

### R6. Runtime 실행과 눈으로 확인

M2에서는 Client를 `Client/Default` working directory에서 Debug로 실행해 actual Server/animation F action을
Debug-only nonProduct typed route로 확인한다. M3 자동 admission과 atomic Product publication 뒤 같은 tree의
Debug와 Release에서 exact published revision을 재생한다. complete/family/occurrence filter가 같은 IR
pointer를 사용하는지, 이펙트가 실제 world/camera에서 누락·폭주·잘못된 scale 없이 재생되는지 수동 smoke한다.

정식 build는 저장소 root에서 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 Release를
직렬 실행한다. 실제 실행은 `Framework.slnLaunch`의 `Server + Client` profile을 사용한다. Lobby에서
Server 승인을 받아 Character Select로 진입한 뒤 Artist thumbnail을 선택하고, Debug는 F1 Effect Tool의 Active Product Cue와
실제 F 입력을 모두 확인한다. Release는 F1 도구가 없으므로 Server command -> snapshot -> animation
event -> exact catalog prepared spawn의 실제 F 경로만 확인한다. `Lobby -> Test`는 캐릭터/네트워크 없는
Map Editor이므로 이 smoke에 사용하지 않는다.

M3 전에 `effect.artist.skill.31470`의 v13 identity carrier, Assembly, compiled artifact/receipt, format-3
Catalog entry, `PlayerSkills.json.effectId`, `sdm_sk_onestroke`의 generated `effectref=asset`이 같은 prepared
identity를 가져야 한다. M2 Debug-only route는 이 Product field를 거짓으로 열지 않는다. 현재 Product chain은
존재하지 않으므로 M3가 atomic하게 생성한다. Geometry WModel 7개와
compiled MaterialBinding에서 재도출한 DDS 전부를 hash-bind하며, exact recovery 4개
`fx_a_noise_011`, `fx_e_ring_001_cl`, `fx_a_decal_014`, `fx_c_atypical_016`도 canonical Resources에
publish됐는지 확인한다.

수동 timeline 기준은 0 ms Ink 4, 1338 ms Weapon 1, 1380 ms Swing 15, 1451 ms Hit 12와
ZoomBlur 1, 1452 ms Distortion 1, 1453 ms Light 1이다. 총 action 2.833초 안에서 종료되어야 하며 Mesh
scale/pivot/basis, Sprite billboard/SubUV, Decal projection/depth, Ribbon continuity/tail, Light radius/lifetime,
Post lifetime, Material fallback/depth/cull/sampler, attachment/world-space를 family와 occurrence filter로 확인한다.

이 단계는 numeric/structural admission을 대체하지 않는다. 외부 reference PNG는 사용자의 눈 검증에
사용하되 이미지 기반 자동 oracle로 만들지 않는다. 수동 관찰에서 이상이 보이면 해당 occurrence ID,
compiled revision, sample/seek time, reference path와 defect category를 기록하고 tuning overlay와 R3~R5의
numeric/structural fixture에 재현을 추가한 뒤 다시 admission한다. 35 occurrence 각각의 사용자
`APPROVED`가 없으면 M3 Product를 열지 않는다.

### R7. Artist F 최종 동결과 회귀

M2 수동 smoke에서 발견된 모든 이상을 occurrence ID와 compiled revision으로 overlay와 numeric/structural
fixture에 환류하고 35개 user APPROVED를 먼저 닫는다. M3/R7에서 새 이상이 나오면 Product publication을
rollback하고 M2로 되돌아가 새 Program SHA를 만든다. approved Program을 변경하지 않은 채 Debug/Release,
focused/deep ProjectAudit, publish/prewarm rollback, no-I/O prepared attach를 다시 통과하고 Source
`35/399/629`, Geometry `7/7`, Material `27/34`, renderer `13/16/3/1/1/1`, Product
`35/35`가 동일 frozen tree에서 유지될 때 Artist F compiler/runtime/renderer interface를 동결한다.
수동으로만 맞춘 값이나 Product path 전용 class/count 분기는 0이어야 한다.

### R8. 4-class와 Valtan 확장

Artist F runtime/compiler/renderer를 M3에서 동결한 뒤 raw/data-only worktree는 4-class corpus와 Valtan
Particle/Decal/Trail/Material/Camera corpus를 병렬 적용할 수 있다. shared C++ writer는 계속 한 명이다.
production code에 class/count switch를 추가하지 않고 fixture denominator만 확장한다. 공통 handler가
부족하면 corpus 값을 근사하지 않고 확장을 멈춘 뒤 shared pipeline gate로 되돌아간다.

latest raw inventory는 `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53`다. 4-class unique request 835와
Valtan unique request 377, 합계 1,212 중 1,208은 structural inspection을 끝냈다. TGA 4와
provenance/corpus 18, 총 blocker 22를 독립 review한 뒤 사용한다. extraction은 반복하지 않고 M3 GPU
schema 위에서 binding/materializer를 한 번만 재생성한다.

4-class source corpus는 Artist F 35개를 포함하지 않는 별도 분모다.

| class | skill/stage/clip | source occurrence | 기존 Product cue |
|---|---:|---:|---:|
| DimensionMaster | 12/15/22 | 872 | 19 |
| LanceMaster | 17/27/45 | 2,199 | 41 |
| Artist (`31470` 제외) | 9/15/16 | 841 | 14 |
| Warlord | 13/17/30 | 1,320 | 27 |
| 합계 | 51/74/113 | 5,232 | 101 |

portable inventory revision을 먼저 고정하고 Artist 9-skill canary -> DimensionMaster -> Warlord ->
LanceMaster 순으로 cue/document atomic admission한다. source 51문서와 기존 Product 101 cue는 서로 다른
분모이므로 합치지 않는다.

Valtan의 정본 분모는 action 170, stage 2,464, clip 2,378, notify 21,931이며 typed target은 Particle
6,159(`PlayParticleEffect` 6,026 + quarantined `DefaultParticle` 133), Decal 536, Trail 430, Material
606, Camera 1,022다. generic `Effect` 3,787은 typed target으로 승격하지 않고 격리한다. 세 Action LOA를
raw SHA로 pin한 뒤 Particle -> Decal -> Trail -> Material -> Camera 순으로 적용한다.

R1 historical exact provider는 없지만 사용자가 reconstructed high-ceiling 정책을 승인해 R2 transport와
M0 구현이 진행됐다. evidence blocker는 보존하며 explicit policy/capability와 실제 consumer proof로만
execution blocker를 해소한다. typed consumer, renderer, actual Client smoke와 final regression 전에는
화면에 보이는 legacy effect나 harness packet을 복원 완료로 판정하지 않는다.

## 기존 G 상세 계약

아래 G00~G11은 원래 계획의 상세 acceptance contract다. 현재 실행 순서는 앞의 R1~R8을 정본으로
사용하며, 이미 PASS한 G를 재구현하지 않는다.

## G00. Known-red base와 public handshake 동결 (historical)

Integration Captain이 `068fcc2` 위에 이 계획과 최소 public handshake를 고정한다.

고정 필드는 다음과 같다.

- `(sourceOccurrenceId, sourceSystemId)` composite emitter identity
- selected LOD node/path/provenance
- ordered module reference index, role, stable ID, exact class/alias lineage
- property `(storage, canonicalPropertyPath, referenceId)`
- distribution definition/occurrence/reference ID, typed payload variant, fidelity, blocker, admission
- PointLight typed component binding과 instance/current inherited default 분리
- GeometryBinding expected tuple
- MaterialRecipe/MaterialOccurrence ID와 contract hash
- renderer family와 source space
- canonical source document hash, compiled IR hash, catalog/compiler revision

G00 종료 조건:

- 모든 Wave 1 branch가 같은 G00 commit에서 시작한다.
- Product 0/35와 known-red baseline 세 항목을 기록한다.
- Source/Compiler가 공통 schema/Codec를 각각 수정하지 않는다.
- 부족한 field는 Integration에 최소 typed request로 제출하며 승인 전 다른 field에 smuggle하지 않는다.

## Wave 1A. Evidence와 generic compiler core 병렬

### G01. Source Evidence Final

Source Closure는 local distribution만이 아니라 selected-LOD 전체 semantic evidence와 reconstruction
oracle을 소유한다.

- 15 distribution target / 17 occurrence와 PointLight 1 occurrence
- 399 ordered module reference, 1,434 top-level property, 1,572 primitive leaf, 629 distribution
- `INSTANCE_EXPLICIT -> NESTED_ARCHETYPE_TEMPLATE -> CLASS_CDO -> PARENT_CDO_HIERARCHY -> EVALUATOR_DEFAULT`
  순서의 field provenance
- target000/001/007/009/014와 custom evaluator의 pre-payload rejection
- raw decoded evidence와 executable typed payload의 분리
- external native-tail 248, seeded module 14, Required local-space default 8, Decal default 3,
  Ribbon default 1, ScreenPost default 1과 selected-LOD/class default의 closure 또는 실행 무관 증명
- standard UE3 module은 tagged instance/archetype/CDO/script/native contract를 결합하고 custom EF class는
  별도 native/differential numeric oracle 없이는 standard handler로 분류하지 않음
- 각 property를 `EXECUTION_CONSUMED`, `VERIFIED_IRRELEVANT`, `UNRESOLVED` 중 하나로 분류하고 근거 receipt 고정
- evidence/artifact/execution/Product blocker를 분리해 definition/property/module/element/receipt/registry/compiler로 전파
- code-only checkpoint와 마지막 bounded source-era artifact 조사

Source-era artifact가 없으면 source fidelity는 `UNRESOLVED` 또는 `CURRENT_REVISION_EVIDENCE`로
동결한다. current package constant를 old child payload로 사용하지 않는다. 실행 의미는 별도의
`RECONSTRUCTED_NUMERICALLY_VERIFIED` receipt가 있는 경우에만 열 수 있으며, standard/native evaluator와
default/seed가 numeric output에 미치는 영향을 독립 sample로 고정한다. Geometry receipt가 확정되기 전
candidate/header를 최종 재생성하지 않는다.

합격:

- 15 definition/17 occurrence와 PointLight active reference all-consumed
- 399 module, 1,434 top-level property, 1,572 leaf, 629 distribution이 consumed 또는 verified-irrelevant이며
  unknown/unconsumed/silent-ignored 행 0
- payload decoded/unresolved, semantic ready/blocked, compiled/admitted 분모를 각각 보고
- target000/001/007/009/014, seed/native-tail/default/Light archetype-CDO mutation이 production resolver에서 실패
- code-only commit은 pure resolver/binder, temp-output generation, mutation test만 PASS로 주장
- checked-in `--check`, deep audit, Codec round-trip은 Gate 1 regeneration commit에서 실행
- Product false

### G02. Geometry Evidence Final — PASS `0aca7928`

Geometry lane은 WModel evidence와 Engine decode contract만 닫는다. `Effect_Playback`과
`Effect_DocumentRenderer`는 수정하지 않는다.

필수 작업:

- strict JSON integer version과 canonical EOL/raw hash domain
- Python/C++ 동일 tangent-W 허용 계약
- writer-independent immutable golden과 actual 7-carrier C++ channel/index/bounds/hash oracle
- actual Resources legacy v1.0 static/skinned/hasBounds/multi-submesh corpus C++ sweep
- full transactional rollback
- self-consistent metadata와 externally bound expected identity 분리
- pivot와 exact UPK→glTF가 없으면 source blocker 유지

합격:

- frozen commit 독립 PASS
- actual 7 carrier C++ Debug/Release 7/7
- legacy corpus 2,586 decode regression
- Engine -> UpdateLib -> Client Debug/Release
- Resources와 Effect runtime diff 0
- Product false

### G03. Material Evidence Final — `cde8f3bd` evidence PASS, execution 0/255 BLOCK

Material lane은 raw evidence와 typed recipe만 닫고 HLSL/runtime을 수정하지 않는다.

필수 작업:

- source-pack manifest를 통한 logical package와 raw export identity 결합
- 342 scalar / 19 vector / 71 texture의 raw array order/name/value/reference/owner 결합
- MIC Parent edge와 selected base graph exact identity
- Texture2D export/serial/tagged sampler와 DDS binding
- parent/default/render/static field owner lineage
- recipe composition digest와 34 occurrence identity 연결
- coordinated closure/receipt reseal, label/parentGraph/inputs/defaults/sampler owner swap 거부
- checked receipt/contract 재생성으로 현재 hash mismatch 제거
- MIC native `FStaticParameterSet`, Engine/EFGame Material CDO, Texture sampler default, installed shader cache와
  surviving expression/native material metadata를 조사해 static/full-render/full-cull/default/sampler/graph별
  reconstruction oracle receipt 생성
- render-state 89, static 94, strict sampler 72, graph 23/502 edge를 각각 행 단위로 보존하며 값이 없는
  행을 공통 fallback으로 채우지 않음

합격:

- recipe row 27/27, occurrence 34/34, unused/unexpected recipe 0
- sampler exact 0, rejected legacy 3 instance + 1 parent, direct unproven 71, strict sampler 72
- graph family 23, null 1,803, unresolved edge 502, Source-exact graph 0
- shallow/deep generator, raw UPK/DDS/manifest audit와 mutation tests
- 23 family의 independent expected sample 출처와 아직 구현되지 않은 evaluator를 별도 기록
- evidence commit에서는 evaluator implemented 0과 Product false를 정직하게 유지하되,
  Gate 1 실행-readiness matrix에서 owner 없는 material blocker 0

### G04. Generic Compiler Core — non-executable checkpoint PASS `c4b00f14`

Compiler lane은 Source 재생성 전 generic typed core만 구현한다.

필수 작업:

- System -> Emitter -> selected LOD -> ordered module -> property/distribution immutable IR
- opcode별 allowed/required schema와 실제 handler consumption receipt
- canonical document identity와 full-field deterministic IR hash
- admission-before-payload, unknown/duplicate/nonfinite/alias/LOD mutation rejection
- generic production invariant와 Artist fixture denominator 분리
- Source generated header read-only 소비

G04에서는 current generated candidate의 최종 adapter PASS를 주장하지 않는다. `7/35/399/629`와
`13/16/3/1/1/1`은 harness fixture에만 둔다. production에 Artist/31470/count hardcode를 넣지 않는다.

합격:

- raw module payload materialization 0
- generic inspection/compiler Debug/Release harness와 static audit
- Product false

## Gate 1. Frozen audit, 통합, Source 단일 재생성 — historical, R2가 supersede

각 G01-G04 lane은 stable commit을 push하고 mutation을 멈춘다. Independent Review는 commit SHA 하나만
검사하고 read-only PASS/BLOCK을 보낸다. 구현 helper와 같은 parser만 사용한 자기확인은 PASS가 아니다.

이 절의 옛 cherry-pick 순서는 사용하지 않는다. 실제 재개 순서는 R2를 정본으로 하며 base
`38ebe7cf`, Geometry `669acf07`과 `0aca7928`의 개별 cherry-pick, compiler/publisher 재-pick 금지,
`4ffe1102` 폐기와 combined head 재작성을 따른다. 이 historical 문구의 통합 금지는
`RECONSTRUCTED_APPROVED_V1` 승인 전 상태를 설명한다. 승인 뒤 실제 통합은 R2 절의 exact 14-commit
순서와 새 policy/materializer gate를 따른다.

Gate 1 종료 조건:

- stale generated output 0
- Source 7/35/399/629와 renderer 분모 보존
- Material 27/34와 Geometry 7 hash를 compiled input receipt가 결합
- actual regenerated candidate의 Codec round-trip
- occurrence별 remaining execution blocker matrix와 owner가 존재
- owner 없는 blocker가 하나라도 있으면 Renderer 단계로 진입하지 않음
- Product false, integration worktree clean

## Gate 1B. Execution-readiness와 runtime resource provisioning

Renderer나 runtime authority를 수정하기 전에 네 병렬 closure job으로 실제 실행 입력을 준비한다.

### G05-S. Source Semantic Reconstruction Readiness — `c927e397 -> 7da937ae`, evidence PASS/readiness 0/29 BLOCK

G01에서 남은 standard/native/default/seed/custom evaluator 행을 actual regenerated candidate 기준으로
다시 계산한다. 각 399 module과 1,434 top-level property는 실행 handler가 소비하거나 독립적으로
실행 무관임을 증명해야 한다. target007/014 custom EF evaluator, external native-tail, seed와 local-space,
Decal/Ribbon/Post/Light default에 evaluator ID와 numeric oracle을 부여한다. current-only evidence는
source exact로 승격하지 않는다.

접근 가능한 provider acquisition은 끝났고 source-era provider/pilot은 0이다. 아래 semantic 작업은 동일
revision artifact 또는 승인된 authenticated capture가 들어와 R1을 다시 열 때만 재개한다.

### G05-G. Geometry Candidate Cook and Resource Provisioning — PASS `0aca7928`

G02의 7개 WModel 1.1 candidate를 deterministic cook하고 expected tuple을 생성한다. candidate는 먼저
임시 staging directory에서 검증한 뒤 팀장 관리 물리 root인
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`의 정확한 7개 target만
backup -> atomic replace한다. 실패하면 기존 7개를 복원한다. Git worktree의 Resources 유무를 제품
payload 존재 증거로 사용하지 않고 test process의 `LOSTARK_RESOURCE_ROOT`를 위 물리 root로 고정한다.

### G05-M. Material Reconstruction Oracle and Texture Provisioning — `cde8f3bd` evidence PASS/readiness 0/255 BLOCK

current Engine/EFGame CDO와 installed cache는 cross-revision 후보일 뿐 source value provider가 아니다.
`cde8f3bd`는 source MIC static override value 23건만 보존했고 render/sampler 및 execution readiness는
열지 않았다. 동일 revision provider 또는 승인된 authenticated capture가 들어올 때만 27 recipe의
static/render/cull/default/sampler와 23 arithmetic family expected numeric sample을 닫는다.
모든 34 occurrence가 참조하는 texture dependency를 열거하고 기존 Resources DDS 또는 새로 검증한 DDS를
명시적인 asset ID/hash로 stage한다. white/black/legacy-default texture를 missing input 대체값으로 쓰지 않는다.
물리 texture 교체가 필요하면 G05-G와 같은 backup -> validate -> atomic replace -> rollback을 사용한다.

### G05-P1. Derived Artifact and Publisher Schema Code

v14 Source Contract 자체는 계속 read-only/non-drawable이다. 이 lane은 v14 Source, Geometry,
Material contract와 semantic reconstruction receipt를 결합할 `lostark.effect-authoring` v13
derived authoring/Assembly identity carrier와 compiled artifact의 schema만 구현한다. v13 carrier는
runtime semantic authority가 아니며 생성물은 다음 identity를 저장한다.

```text
sourceContractHash
sourceSemanticClosureHash
geometryContractHash
materialContractHash
resourceBindingHash
compilerInputHash
```

이 병렬 lane은 generator, serializer, publisher validator, runtime catalog format 3 schema와 synthetic
zero-blocker fixture만 구현한다. G05-S/G/M의 최종 hash를 요구하거나 actual Artist F v13
document/Assembly/compiled artifact를 생성하지 않는다.
`Publish-Effects.ps1`은 verified v13 derived document와 compiled artifact를 함께 검증하고 v14를 직접
publish하지 않는다. runtime catalog format 3은 위 identity와 compiled receipt/revision을 보존한다.
actual generator는 execution blocker가 하나라도 있으면 v13 document/Assembly를 만들지 않고 이전
Product를 유지하며, G10 이후 모든 final hash가 동결된 Gate 10B에서 한 번 실행한다.

Gate 1B 종료 조건:

- unknown opcode/property, unconsumed property, silent ignored native/default field가 각각 0
- Source semantic, Geometry payload, Material recipe/resource, derived Product 입력에 owner 없는 blocker 0
- runtime Resources의 7 WModel과 모든 required DDS가 compiled expected identity와 일치
- G05-P1 synthetic zero-blocker fixture만 v13/schema/publisher round-trip을 통과
- actual Artist v13/compiled artifact는 아직 생성하지 않고 Product bit/publish는 false
- Source exact가 아닌 reconstruction 행의 evidence blocker는 보존

## G06. Runtime Authority Foundation — PASS `38ebe7cf`

G05-S/G/M/P1 frozen PASS 뒤 단일 owner가 중앙 runtime authority와 extension point를 먼저 고정한다.
이 G 뒤에는 family/geometry/material lane이 중앙 파일을 다시 수정하지 않는다.

소유 파일:

- `Effect_CascadeCompiler.h/.cpp`
- `Effect_Playback.h/.cpp`의 prepared input boundary
- `Effect_DocumentRenderer.h/.cpp`의 common cache, upload와 family dispatch boundary
- `Effect_PresentationService.cpp`
- `Effect_Catalog.cpp`
- `Effect_Object.h/.cpp`
- common shader binding layout
- `Publish-Effects.ps1`과 runtime catalog format 3 parser/stager
- `Engine/Public/Engine_Struct.h`
- `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`

필수 계약:

- immutable prepared identity `{catalogRevision, compilerRevision, assetId, sourceHash, compiledHash,
  geometryHash, materialHash, resourceBindingHash}`
- v14 Source/Geometry/Material에서 immutable execution IR을 만드는 최종 adapter, opcode shape와
  execution admission compile을 G06이 소유하고 이후 G07은 compiler 파일을 수정하지 않음
- 각 opcode에 required handler ID/version을 고정하고 G07-G10이 fixed extension point로 제출하는
  capability/consumption receipt가 없으면 execution admission을 열지 않음
- Source/Geometry/Material을 한 번 compile하고 prepared pointer만 consumer에 전달
- Catalog -> Presentation -> Object -> Playback -> Renderer가 같은 compiled document identity를 확인
- central family interface, geometry cache key, material evaluator handle와 shader slot 선할당
- typed Light descriptor에 radius/falloff/color/brightness를 명시하고 `CEffectObject`와 deferred shader가
  고정 linear attenuation으로 세탁하지 않을 public binding 고정
- publish/load/prewarm/equivalence 전체 stage -> validate -> commit과 실패 rollback
- combat Spawn의 compile/model/DDS/vector-field I/O 0
- compile A 뒤 raw B attach, revision/hash/pointer/Product mismatch 거부
- `v14 Source Contract -> immutable compiled execution IR`만 runtime semantics의 단일 권위이며,
  v13 derived document/Assembly는 Catalog identity와 authoring carrier일 뿐 Playback/Renderer가 raw
  semantics를 읽지 않음

G06은 아직 family output을 Product로 승인하지 않는다. 중앙 Renderer의 v14 dispatch는 compiled family
enum만 사용하고 unsupported family는 fail-close한다. Light public payload와 deferred shader 변경은 같은
SHA/worktree에서 `Engine -> UpdateLib -> Client` Debug/Release build lease를 사용한다.

합격:

- actual `Prepare_Catalog -> Find_Prepared -> PresentationService::Spawn -> EffectObject` harness
- compile 1, attach 1, runtime I/O 0
- Catalog/Object/Renderer/Presentation rollback에서 이전 catalog/cache/targets 보존
- 중앙 public API와 common shader 동결
- Product false

## G06B. Source Runtime Program Materializer — `4ffe1102` 폐기, R1 readiness 뒤 R2에서 새로 작성

Source execution receipt의 35 emitter/399 module/629 distribution을 immutable program으로 옮긴다.
`4ffe1102`를 cherry-pick하거나 mutation하지 않는다. R1 readiness가 모두 PASS한 combined head에서
denominator와 fail-closed 요구만 새 구현의 acceptance contract로 재적용하고 generic JSON bag은 runtime
authority로 승격하지 않는다.

추가 합격 조건:

- ParticleParameter 17 occurrence의 name/mode/four-range/fallback/oracle/provenance typed coverage 100%
- PointLight typed component field와 handler receipt coverage 100%
- seed/default closed typed variant coverage 14/14
- READY distribution 629/629 closed evaluator registry join
- exact class -> opcode -> handler -> allowed/required property schema와 consumption receipt
- source record/value, handler SHA, lookup shape, emitter/opcode order, nested key mutation transactional reject
- tracked LF/CRLF에서 동일 canonical program SHA
- Debug/Release actual candidate mutation harness
- runtime execution admission false, Product false

## Wave 2A. Typed executor, Geometry와 Material consumer 병렬

### G07. Typed Cascade Executor — remaining

G06이 최종 compile한 immutable execution IR을 strict consumer로 받아 `CEffectPlayback`의 v14
production simulation이 typed opcode만 소비하도록 바꾼다. G07은 `Effect_CascadeCompiler`를 수정하거나
별도의 IR 승격 경로를 만들지 않는다. G06이 동결한 handler extension point에 Playback capability와
consumption receipt만 등록한다.

필수 opcode/handler 범위:

- Constant, Uniform, ConstantCurve
- standard Float/Vector ParticleParameter의 Direct/Normal/Abs와 네 range
- oracle이 있는 custom EF evaluator의 별도 opcode
- spawn, burst, lifetime, delay, loop, size, rotation, location, velocity, acceleration, color
- local/world/source space와 axis lock
- SubUV, spawn-per-unit, location-on-ground
- TypeData Mesh/Decal/Ribbon/Light
- ActionCue parameter binding과 none binding
- fixed seed, fixed timestep, deterministic sample order
- `Effect_Playback.cpp`의 Mesh StartSize hidden `x0.01`을 제거하고 signed dimensionless axis reorder만 유지

합격:

- 399 ordered reference, 1,434 property, 1,572 leaf, 629 distribution handler receipt
- Playback raw `SourceRecipe.Modules`, 문자열 class/property scan 0
- unresolved/custom payload read 0
- fixed seed/time repeated run deterministic
- failed compile/stage에서 이전 playback/prepared revision 보존
- Debug/Release actual candidate harness
- Product false

### G08. GeometryBinding Runtime Consumer — remaining

G02/G05-G 결과를 기존 `CModel -> CMesh -> CMaterial` 경로와 G06 extension point에 연결한다.

필수 작업:

- expected tuple `{assetId, payloadHash, metadataIdentity, geometryPreScale=0.01,
  DIMENSIONLESS_AXIS_REORDER_ONLY}`와 decoded bytes 대조
- vertex와 embedded/derived bounds에 preScale 정확히 한 번 적용
- prepared model cache identity에 tuple 전체 포함
- G07에서 hidden `x0.01`이 제거됐음을 검증하고 signed dimensionless axis reorder와 carrier preScale이
  정확히 한 번씩만 적용되는지 확인
- Sprite/Decal cm-to-m 경계와 분리
- tangent-W/COLOR sidecar를 Mesh family shader가 소비할 immutable vertex binding까지 전달

합격:

- 7 carrier / 13 Mesh occurrence combined magnitude numeric oracle
- cache collision, wrong hash, double/zero/nonfinite scale, final bounds mutation 거부
- failed load/prewarm에서 prepared cache residue 0
- source fidelity와 external authentication 상태 보존
- Product false

### G09. Material Recipe Compiler and Evaluator — remaining

G03/G05-M의 27 recipe를 G06 extension point가 소비할 immutable MaterialBinding으로 compile한다.

필수 작업:

- exact scalar/vector/texture input과 sampler/render-state payload
- source-exact input과 reconstructed arithmetic family의 분리
- 23 family별 stable evaluator ID/version과 implemented state
- Add/Multiply/Panner를 포함한 실제 관측 연산을 typed evaluator/HLSL로 구현
- static permutation, blend/depth/cull/default/sampler는 source 또는 G05-M oracle이 있는 값만 compile
- strict sampler 72(instance 71 + parent 1)와 omitted default를 공통 fallback으로 채우지 않음

합격:

- 27 recipe / 34 occurrence handler consumption
- per-family independent numeric sample과 mutation test
- unknown expression/input/sampler/render state 실행 0
- static/render/cull/default/sampler execution blocker 0
- reconstructed graph를 `SOURCE_EXACT`로 승격한 행 0
- Product false

## G10. Renderer family와 Effect Tool — historical detail, current single captain

G07-G09 통합·독립 감사 뒤 G06의 중앙 Runtime Authority/Foundation API를 다시 동결한다. 아래 옛
family lane 분할은 파일 경계 참고용이며 현재 M0~M3에서는 fresh captain 한 명이 순서대로 구현한다.

G06 Foundation이 이미 소유하고 family가 변경하지 않는 계약:

- compiled render packet
- common particle attributes와 CPU/GPU upload layout
- immutable geometry/material binding handles
- renderer family dispatch interface
- central dispatcher와 common shader binding slot
- family 구현의 별도 파일/project/filter 경계

중앙 header, dispatcher, common shader는 동결한다. family 구현은 지정된 파일과 shader 경계를 지킨다.

family 검증 순서:

| lane | 분모 | numeric/structural 종료 증거 |
|---|---:|---|
| Mesh + Sprite | 13 + 16 | size/rotation/local-space/lifetime, geometry/material binding, tangent/COLOR input, double-scale mutation |
| Decal + Ribbon | 3 + 1 | projection, signed source space, segment ordering, spawn-per-unit, trail lifetime, invalid topology rollback |
| Light + ScreenPost | 1 + 1 | exact child Brightness/flags, current inherited default 분리, `LIGHT_DESC` radius/falloff/color/brightness 전달, deferred attenuation numeric oracle, post weight/lifetime, unsupported recipe rejection |
| Effect Tool | 35 inspection rows | Source Evidence/IR read-only tree와 Draft/Product Play 경계 |

Effect Tool 계약:

- v14 `SOURCE_CONTRACT`는 read-only이며 Save, Apply, resource replace, execute를 비활성화한다.
- Source tree는 7 System -> 35 Emitter -> 35 selected LOD -> 399 ordered module -> 629 distribution 순서다.
- Legacy mode는 기존 Authored Draft Preview/Save를 유지하고 Typed Restoration mode는 versioned tuning
  overlay만 Save한다.
- Product Play는 Authored 파일을 disk에서 다시 load하지 않고 exact catalog prepared revision을 attach한다.
- Complete/family/occurrence solo는 같은 IR pointer의 transient mask이며 compile/model/texture I/O가 0이다.
- Tool 전용 두 번째 renderer나 Catalog 우회를 만들지 않는다.

fresh captain은 family별 frozen checkpoint를 같은 branch에 순서대로 commit/push하고 독립 감사를 받은 뒤
다음 family로 전진한다. 별도 family branch를 cherry-pick하지 않는다. six-family 완료 뒤 Tool의 exact
prepared attach/no-I/O와 Legacy↔Typed rollback을 작은 후속 checkpoint로 재검증한다.

G10 통합 종료 조건에는 Renderer와 Presentation의 raw `eKind`, raw SourceRecipe/module/default scan 0을
포함한다. compiled family enum과 typed render packet 이외의 dispatch는 v14 Product path에서 거부한다.

## Gate 10B. Actual Artist Derived Artifact 단일 생성

G05-P1은 schema/publisher code만 만들었으므로 actual Artist artifact 생성은 Source semantic,
Geometry/Material contract와 resource binding, G06 execution compiler, G07-G10 handler/renderer의 final
hash가 모두 동결된 뒤 Integration이 한 번만 수행한다.

생성 순서:

1. final v14 Source Contract와 Source semantic reconstruction receipt를 읽는다.
2. final Geometry/Material contract와 실제 Resources binding hash를 결합한다.
3. G06 offline compiler가 v14-derived immutable execution IR과 compiled receipt를 생성한다.
4. G05-P1 generator가 같은 six-hash identity를 가진 v13 derived authoring/Assembly carrier를 생성한다.
5. publisher가 v13 carrier와 compiled artifact의 identity/revision 일치를 검증해 catalog format 3을
   임시 stage하되 Product bit와 기존 catalog publish는 계속 false/미변경으로 둔다.

실행 의미의 단일 권위는 3번의 compiled IR이다. v13 carrier의 raw Assembly/SourceRecipe는 Catalog ID,
Effect Tool Draft/inspection과 provenance 연결에만 사용하며 Playback/Renderer가 실행하지 않는다.
execution blocker, unconsumed handler, resource hash mismatch 또는 six-hash 불일치가 하나라도 있으면
두 artifact와 catalog stage를 전부 rollback하고 이전 Product를 유지한다. 일부 hash만 바꿔 반복
재생성하지 않고 최종 입력 묶음 전체를 한 transaction으로 생성한다.

Gate 10B 종료 조건:

- actual Artist v13 carrier와 compiled IR/receipt의 six-hash identity가 동일
- 399/1,434/1,572/629 consumption receipt와 renderer 13/16/3/1/1/1이 compiled artifact에 고정
- actual Resources 7 WModel과 required DDS expected identity 일치
- raw v13 runtime semantic consumer 0
- Product false, 기존 published catalog/cache/targets 미변경

## G11. Artist F Admission Oracle와 Product publish — remaining

Integration이 새로운 feature를 만들지 않고 전체 oracle만 실행한다. expected 값은 compiler/renderer가
자기 출력으로 다시 만든 값이 아니라 raw evidence, 독립 decoder, hand-authored immutable golden 또는
native numeric oracle에서 온다.

각 35 occurrence row는 다음을 기록한다.

- source fidelity와 원본 blocker
- artifact binding integrity
- execution admission과 evaluator ID/version
- source/compiled document identity
- tuning overlay SHA와 해당 occurrence user decision
- geometry/material binding과 hash
- selected LOD와 ordered opcode coverage
- renderer family
- fixed seed/timestep/ActionCue input/sample time
- expected/actual/tolerance
- Product blocker와 최종 decision

전체 consumption gate는 분모를 축소하지 않는다.

```text
ordered module reference 399/399
top-level property       1,434/1,434 consumed or verified-irrelevant
primitive leaf           1,572/1,572 consumed or verified-irrelevant
distribution             629/629 compiled or verified-irrelevant
unknown opcode           0
unknown property         0
unconsumed property      0
silent ignored native/default field 0
```

최종 자동 gate:

```text
Source contract shallow/deep and all mutation tests
Geometry actual 7-carrier Debug/Release C++ oracle and legacy corpus
Material 27/34 shallow/deep raw UPK/DDS/manifest oracle
Compiler/Executor Debug/Release actual candidate oracle
Six renderer family 35-occurrence numeric packet/state oracle
Effect Tool Legacy/Typed mode, read-only IR, overlay seal/recompile, revision and no-I/O transaction oracle
legacy Product cue projection 101/101 and all non-target Catalog entry delta 0
Legacy <-> Typed mode switch failure rollback preserves prior selection/prepared pointer
Engine Debug/Release
UpdateLib Debug/Release
ClientFrontendHarness Debug/Release
Client Debug/Release
focused Artist ProjectAudit checks all PASS
full Invoke-ProjectAudit executed with the canonical physical Resources root
all Artist/Effect-related full-audit failures fixed
unrelated baseline failure ID/detail delta 0
JSON/XML parse
git diff --check
conflict marker and unmerged path 0
residual Client/Server process and listener check
```

Product predicate가 35/35 true일 때만 Artist F cue를 Catalog에 atomic publish한다. partial Product나
missing occurrence 축소는 허용하지 않는다. publish 뒤 exact revision의 load/prewarm/Spawn을 다시 실행하고
실패하면 이전 catalog/cache/targets를 유지한다.

publish 대상은 Gate 10B의 verified v13 derived authoring/Assembly identity carrier와 runtime catalog
format 3 compiled entry다. v14 Source Contract를 drawable로 바꾸거나 Catalog가 v14를 직접 읽게 하지
않으며 v13 raw semantics도 실행하지 않는다. publisher, catalog stage, prepared compiler와 runtime
Spawn은 같은 six-hash identity와 compiled IR pointer를 확인해야 한다.

G00에서 canonical Resources root를 지정해 full ProjectAudit baseline report를 생성하고 failure ID/detail을
고정한다. Artist/Effect와 직접 관련된 baseline failure는 G11까지 전부 닫는다. Map/Character 등 이 기능과
무관한 baseline failure가 남으면 full audit 전체를 PASS라고 쓰지 않고, exact same failure 집합이며 신규
failure가 0이라는 비교 증거를 남긴다.

## Build lease와 commit 규칙

Integration Captain이 전역 full-build lease 하나를 관리한다.

- Engine, UpdateLib, Client full build는 한 번에 한 worktree만 실행한다.
- Debug와 Release도 직렬화한다.
- Engine public 변경은 같은 SHA/worktree에서 `Engine -> UpdateLib -> Client`를 실행한다.
- 다른 worktree의 `EngineSDK`, `.lib`, generated header를 복사해 PASS를 만들지 않는다.
- Python/unit은 output path가 겹치지 않을 때만 병렬 실행한다.
- `__pycache__`, temp JSON, build/intermediate, ignored EngineSDK를 stage하지 않는다.
- lane commit은 하나의 검증 단위이며 frozen audit PASS 뒤 rebase하지 않는다.
- correction은 같은 lane의 후속 commit으로 만들고 새 SHA를 다시 감사한다.
- lane branch는 checkpoint push 용도이며 main에는 최종 Integration PR 하나만 연다.

## 도화가 F 이후 확장

G11의 Product 35/35와 동일 prepared runtime path가 닫히기 전에는 다른 class와 Valtan occurrence를
Product로 확장하지 않는다. 완료 뒤에는 production code에 Artist-specific count/switch를 추가하지 않고
같은 generic compiler, material evaluator, geometry binding, six-family renderer와 Tool contract를 사용한다.

확장 순서:

```text
도화가 F golden fixture
-> Artist 9-skill canary (31470 제외)
-> 차원술사
-> 워로드
-> 창술사
-> 발탄 Particle/Decal/Trail/Material/Camera occurrence
-> 전체 corpus regression과 공통 renderer 최적화
```

Artist의 `7/35/399/629`, renderer `13/16/3/1/1/1`, material `27/34`는 fixture harness에만 남긴다.
production compiler/renderer/catalog에 이 숫자를 hardcode하지 않는다.

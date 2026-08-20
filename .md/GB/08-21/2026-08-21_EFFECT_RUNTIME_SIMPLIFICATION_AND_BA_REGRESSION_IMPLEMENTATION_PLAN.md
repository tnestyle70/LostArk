# 2026-08-21 Effect runtime simplification and BA regression implementation plan

## 0. 문서 상태

- 문서 종류: `IMPLEMENTATION_PLAN`
- 대상: Character Select Effect 준비 병목 제거, Debug Effect Tool/Save Hot Reload/approval-admission-SHA 계약 제거, 차원술사 BA 연계 회귀 복구, R 검격 occurrence와 sprite 경계 복원, T `Play All` 분할 현상 진단과 잘못된 surface override 원복
- 구현 범위: `Client runtime -> Effect publisher/runtime catalog -> Server BA authority -> Client input -> focused harness/build`
- 제외 범위: 게임에서 Effect cue를 재생하는 기능, renderer/GPU resource 자체, 사용자 Authored Effect JSON의 시각 값, Client/UI 자율 실행
- 완료 판정: 자동 검증과 Client/Server 빌드를 통과한 뒤 사용자가 새 Server/Client에서 BA와 Character Select 체감을 확인한다.

## 1. 현재 문제와 원인

### 1.1 Character Select Effect 병목

- Character Select loader는 map/model loading이 모두 끝난 뒤에야 선택 class의 Effect 준비를 시작한다. 서로 겹칠 수 있는 두 작업이 직렬화되어 activation 지연이 그대로 합산된다.
- 기본 선택 class인 Lance Master는 현재 direct-authored Effect 41개, sealed JSON 약 29.3MB를 한 번에 요구한다.
- direct document 한 건마다 runtime에서 파일 read, SHA-256, JSON parse, document validation, drawable validation, dependency 재수집/비교를 반복한 뒤 같은 main thread에서 texture/model/GPU 준비까지 수행한다.
- activation gate는 선택 class target뿐 아니라 전체 background prewarm queue가 비워질 때까지 기다린다. Character Select에 필요하지 않은 Effect가 activation을 막을 수 있다.
- 현재 runtime catalog에는 approval/admission sidecar가 활성화되어 있지 않다. 따라서 approval/admission은 현재 병목의 직접 원인은 아니지만, Save Hot Reload와 publisher/runtime rollback 계약을 크게 복잡하게 만든다.
- GPU resource 생성과 최소 JSON 타입/경로 검사는 실제 재생과 메모리 안전에 필요하므로 삭제 대상이 아니다. 대신 필요한 target만 준비하고 loading과 겹치며, 중복 무결성 검사는 제거한다.

### 1.2 차원술사 BA 연계 회귀

- 현재 Client는 LMB가 올라올 때마다 `C2S_RELEASE_SKILL`을 보내고, Server는 COMBO release를 아직 실행되지 않은 `hasBufferedComboInput` 취소로 해석한다.
- 연타 사용자는 BA1 input window 안에서 두 번째 press를 정상 buffer해도 곧바로 두 번째 mouse-up을 보낸다. 이 release가 방금 buffer한 BA2를 취소하므로 반복 클릭 연계가 구조적으로 불가능하다.
- 기존 Server harness는 이 취소를 성공 계약으로 고정했기 때문에 실제 사용자 조작을 재현하지 못했다.
- BA1 `4000 -> 1400ms`, stage별 `comboAdvanceMs`, pending MOVE/SKILL의 full-duration boundary commit은 별도 개선이며 유지한다.

### 1.3 차원술사 R 검격 occurrence 회귀

- R은 `skillId 2050180`, clip `pc_sp_m_00_sk_sk_foldcut`, Product Effect `effect.dimensionmaster.skill.2050180.unified`이다.
- source receipt에는 검격 발생이 0.5초, 0.7초, 1.05초에 각각 분리되어 있으나 현재 세 mesh row는 `execution.enabled=false`, `authoringApproximate=true`, `failClosed=true`라서 source mesh/DDS 경로가 전부 억제되고 approximation만 보인다.
- 세 row는 공통으로 `fm_h_swing_05.wmodel`, `fx_m_trail_007.dds`, `fx_w_pa_spritewave_01_05_tr`을 참조한다. 문서를 세 벌 복제하지 않고 occurrence별 timing/count/orientation만 분리해야 한다.
- 사용자 기준은 1타와 2타가 검격 한 개, 3타가 검격 세 개다. 따라서 동일 mesh/material 자산을 0.5초/0.7초/1.05초 occurrence가 각각 `1/1/3` spawn으로 재사용해야 한다.
- 초기 `fm_d_helix_015_1.wmodel`이 원형으로 도는 현상은 임의 revolution이 아니라 source mesh rotation/type-data 축, pre-rotation, local-space 변환을 한 번만 적용하는지 확인해 교정한다.
- cooked DXBC는 exact vertex layout, resource slot, material identity가 일치할 때만 사용한다. 불일치하는 native blob을 억지로 admission하지 않고 현재 HLSL renderer에서 같은 source 수식을 재현한다.

### 1.4 차원술사 T `Play All` 분할 현상과 잘못된 surface override

- T는 `skillId 2050500`, model cue `dimension_summon`, `DimensionMaster_DimensionSummon.wmodel`을 사용한다.
- 재확인 결과 현재 Authored와 HEAD 기준 summon cue는 모두 기존 `MASKED` 계약이며, summon model/material 자체를 OPAQUE 또는 별도 character pass로 바꿀 근거가 없다. 사용자가 손튜닝한 정상 element도 `Play All`에서 두 조각처럼 중복되어 보였으므로 앞선 summon surface 진단은 철회한다.
- GPU pipeline query로 model-only, elements-only, full `Play All`을 분리한 결과 model-only와 full의 IA primitive/VS/PS invocation이 정확히 같고 elements-only 제출은 0이었다. source/product, base/projected, model cue의 이중 제출은 없었다.
- 잘못 추가됐던 materializer의 `OPAQUE` 강제는 diffuse alpha cutout을 제거해 숨어야 할 면을 노출하는 회귀다. 이를 기존 `MASKED` + 0.333 cutoff와 `RAW_UE3_PARENT_BLEND_MASKED_EXACT` 계약으로 되돌린다.
- Effect Tool은 제품/Debug build에서 제거하므로 Tool 전용 `Play All` 경로도 함께 사라진다. gameplay cue 경로에는 동일 `(owner, actionStart, occurrenceId)`가 정확히 한 번만 준비·제출되는 계약을 유지한다.
- 사용자가 계속 손튜닝 중인 T Authored JSON은 이 작업에서 rewrite하거나 alpha mode를 바꾸지 않는다.

### 1.5 차원술사 바닥 sprite의 사각 경계

- `fx_k_fluidtile_01`, `fx_j_mirnoise_01`, `fx_i_noise_03`, `fx_m_tilelinenoise_01`은 모두 alpha가 1인 opaque DDS다. 이름에 noise/tile이 붙은 텍스처는 원본 material에서 UV 변형·전이·emission 입력으로 쓰이며, 그 자체를 coverage alpha로 쓰는 카드가 아니다.
- 따라서 텍스처 파일명만 보고 모든 카드에 원형 mask를 강제하면 원본의 사각 flipbook/tile carrier까지 잘못 자른다. 경계 처리는 `source material family + named texture role + exact carrier kind`로 분류한다.
- `fx_j_mirnoise_01`이 상대적으로 자연스러운 것은 texture alpha가 경계를 자르기 때문이 아니라 내부 luminance가 반복 가능한 noise라 quad 경계 대비가 덜한 경우다. 이를 공통 coverage 정본으로 승격하지 않는다.
- R의 `fx_w_pa_master_01_05_dt_tr` sprite는 compact 과정에서 E/D/F/C 역할명이 사라져 `fx_i_noise_03`이 base coverage처럼 보인다. 원본 ParticleMaster 계약에서는 C(`fx_a_cloud_021`)가 coverage, D가 warp, E/F가 emission이므로 profile 19 lane을 복원해야 한다.
- R 검격의 `fx_w_pa_spritewave_01_05_tr`도 `maintex`, dissolve, UV noise, inherited emissive 역할을 복원한 profile 20을 사용한다. 이 profile의 source sphere mask가 opaque DDS 배경의 사각 경계를 자른다.

## 2. 고정할 제품 계약

1. Effect Tool, All Effects authoring UI, Save Hot Reload, selected publish transaction은 Client 제품/Debug 실행 경로와 build에서 제거한다.
2. Product cue approval/admission token, sidecar, content SHA와 dependency hash equality는 runtime/publisher 계약에서 제거한다.
3. runtime Effect catalog는 stable `effectAssetId -> document path/payload kind/version`만 소유한다. JSON 타입, 지원 version, Resources-relative path와 document identity 같은 크래시 방지 검사는 유지한다.
4. Character Select는 선택 class target만 activation 조건으로 삼는다. 다른 class/background queue는 진입을 막지 않는다.
5. 선택 class Effect CPU parse/preparation은 loading 시작과 함께 등록해 map/model loading과 겹친다. GPU object commit은 기존 main-thread renderer 경계를 유지한다.
6. runtime spawn은 계속 prepared-only/no-I/O 경로를 사용한다. 준비되지 않은 cue 하나는 격리하되 게임 전체를 중단하지 않는다.
7. BA는 짧은 단일 click이면 BA1만 실행하고, 반복 click이면 다음 stage를 buffer하며, LMB hold도 authored window 안에서 계속 연계된다.
8. COMBO mouse-up은 이미 승인된 continuation을 취소하지 않는다. `C2S_RELEASE_SKILL`은 HOLD skill release 용도로만 사용한다.
9. 우클릭 MOVE/다른 SKILL은 현재 BA stage animation의 `actionDurationMs` 경계에서 continuation보다 우선한다.
10. R의 source 검격은 하나의 mesh/DDS/material 자산을 세 occurrence가 공유하며, 발생 시점과 spawn 수는 `0.5s/1`, `0.7s/1`, `1.05s/3`이다.
11. R helix는 source local/type-data rotation을 exactly-once 적용하고 임의 billboard/revolution fallback을 사용하지 않는다.
12. T summon은 기존 MASKED material과 exact pass 3의 0.333 cutoff를 유지한다. OPAQUE 강제는 금지하고, 하나의 cue/document occurrence는 정확히 한 번만 stage·submit한다.
13. 바닥 sprite 경계는 texture 이름이 아니라 source material family와 named lane으로 결정한다. ParticleMaster는 C coverage, SpriteWave는 maintex와 source sphere mask를 사용하며 generic card 전체에 radial mask를 강제하지 않는다.

## 3. 구현 단위

### G00. 실패 계약과 비용 기준 고정

- Character Select selected target 수/byte, catalog parse, direct document parse, resource prepare, activation wait를 구분한 timing probe를 Debug log/harness에 둔다.
- BA `press -> release -> second press -> release -> stage boundary`를 직접 재현해 BA2가 취소되는 현재 상태를 실패로 고정한다.
- Effect/BA 변경 전 사용자 Authored JSON과 다른 세션 변경의 SHA/status를 기록하고 수정 대상에서 제외한다.

### G01. Effect Tool과 Hot Reload 제거

- `CMainApp`의 Effect Tool 소유, update/render/F1 생성/reset 호출을 제거한다.
- `Client.vcxproj/.filters`에서 `Effect_Tool.cpp/.h`와 `ProductCueApprovals.json` 등록을 제거한다.
- ClientFrontendHarness가 Effect Tool header/UI owner option에 의존하는 테스트를 제거하거나 runtime-owned enum으로 축소한다.
- `PublishSelectedDirect`, transaction receipt, Commit/Rollback hot-load helper와 자동 Save hook은 제품 경로에서 제거한다.
- 공유 dirty worktree의 source 파일은 즉시 삭제하지 않아 다른 세션 변경을 잃지 않고, build/runtime 소비만 완전히 끊는다. 최종 정리 시 소유권이 확인된 tool-only 파일만 별도 삭제 후보로 기록한다.

### G02. approval/admission/SHA/dependency 검증 제거

- cue의 admission token과 `CEffectCatalog::Admit/Validate_ProductCueBinding` 호출을 제거하고 stable EffectAssetId 존재 여부만 사용한다.
- catalog snapshot/load/clear에서 admission sidecar state를 제거한다.
- publisher는 approvals/admissions sidecar를 생성·복사·rollback하지 않는다.
- direct runtime catalog row에서 `contentSha256`와 dependency 배열을 제거한다.
- runtime direct loader는 SHA 계산, dependency 재수집/equality, parse 뒤 두 번째 drawable full scan을 제거한다.
- codec parse의 필수 JSON shape/type/range와 renderer가 요구하는 drawable admission은 한 번만 실행한다.

### G03. Character Select target-only overlapped preparation

- loading target이 정해지는 즉시 선택 class cue ID를 등록하고 existing prewarm queue를 시작한다.
- loader progress와 Effect progress를 같은 loading 기간에 병행한다.
- activation readiness는 `catalog revision + selected target pending/failed`만 검사하고 global queue pending count는 보지 않는다.
- background target은 activation 뒤 budget 안에서 계속 준비하거나 필요 시 lazy priority queue로 남긴다.
- 문서 parse는 같은 target을 revision마다 한 번만 수행하고, resource prepare는 기존 cached prepared target을 재사용한다.

### G04. BA 입력 의미 복구

- Client basic-attack poller는 physical down edge와 hold repeat를 구분한다.
- 첫 press는 즉시 전송한다. hold repeat는 tap과 구분되는 짧은 threshold 뒤 authored window를 놓치지 않는 간격으로 전송한다.
- mouse-up은 local hold-repeat gate만 rearm하며 COMBO release packet을 보내지 않는다.
- Server `Release`는 HOLD만 처리하고 COMBO continuation cancellation을 제거한다.
- 반복 click의 두 번째 press가 buffer된 뒤 release되어도 BA2가 stage boundary에서 시작되는지 검증한다.
- 단일 tap, 반복 click, hold, hold 중 RMB, release/repress, stale sequence를 여섯 class에 대해 회귀한다.

### G05. 차원술사 R exact occurrence와 T surface override 원복

- R의 세 suppressed source mesh row를 exact resource identity가 맞을 때만 drawable하게 만들고 approximation 대체 row와 중복 재생하지 않는다.
- source timing은 각 row의 `sourcePresentation.sourceTimeSeconds`/document-local delay로 고정하고 count는 1/1/3으로 분리한다.
- R helix의 mesh rotation/type-data module을 추적해 source 축 변환을 한 번만 적용하고, 회전 값이 없는 축에는 synthetic revolution을 만들지 않는다.
- R slash와 helix의 mesh/DDS/material slot을 cooked shader contract와 대조하고, 실제 Product renderer가 소비하는 HLSL/DXBC path와 input layout을 harness에서 고정한다.
- R의 ParticleMaster 바닥 sprite는 compact된 E/D/F/C binding을 원본 named lane 순서로 복구해 profile 19를 사용한다. SpriteWave 검격은 main/dissolve/noise/inherited-emissive lane을 복구해 profile 20의 sphere-mask 경계를 사용한다.
- 다른 스킬은 동일 material family와 named lane 계약이 모두 확인될 때만 같은 evaluator를 공유한다. texture filename만 같은 row, 원본이 사각 carrier인 row, 역할 증거가 없는 row는 generic fallback을 유지한다.
- T cue의 Authored alpha mode와 texture slot은 변경하지 않는다. materializer의 잘못된 OPAQUE 강제를 기존 MASKED provenance로 원복한다.
- model-only, generic MASKED, OPAQUE contrast, elements-only, full `Play All`을 각각 렌더해 pipeline statistics를 비교한다. full과 model-only가 동일하고 elements-only가 0인지, exact MASKED와 OPAQUE coverage가 구분되는지 검증한다.
- Tool 제거 뒤에도 gameplay 경로에서 T model cue 하나가 한 occurrence에 한 번만 stage·submit되고 duplicate occurrence가 거부되는지 검증한다.

### G06. publish, build, handoff

- Effect runtime catalog를 단순 schema로 publish한다. R occurrence count처럼 이번 요청에 직접 해당하는 최소 Authored 값만 바꾸고, T를 포함한 다른 사용자 튜닝은 보존한다.
- gameplay balance publisher는 BA timing data를 다시 publish하지 않는 한 Validate만 수행한다.
- focused harness, Server contract, Client Debug/Release isolated-output build를 실행한다.
- 실행 중 Client/Server는 종료하거나 교체하지 않는다. 사용자가 재시작할 binary/data와 수동 확인 경로를 결과 문서에 적는다.

## 4. 자동 검증

1. JSON/XML/PowerShell/Python parse와 scoped `git diff --check`.
2. Effect pipeline runtime-only publish/validate: no approval/admission/SHA/dependency fields, exact path/ID/version rejection, partial commit rollback.
3. ClientFrontendHarness: selected target activation ignores unrelated pending target, revision mismatch fails, direct doc parses once, Effect Tool dependency 없음.
4. Server contract: tap BA1, repeated-click BA2, hold chain, explicit MOVE/SKILL priority, repeat-hit preservation, reset paths.
5. Client input helper harness: first down, tap release, second down/release, hold repeat, RMB suppression/rearm.
6. DimensionMaster R focused harness: exact stable rows, mesh/DDS/material, `0.5/0.7/1.05s`, `1/1/3` spawn, helix rotation exactly-once, approximate duplicate 없음.
7. DimensionMaster T focused harness: model-only와 full `Play All`의 IA/VS/PS 계수가 동일하고 elements-only 제출이 0이며, summon exact MASKED material과 OPAQUE contrast가 구분된다.
8. DimensionMaster material-family boundary harness: ParticleMaster C coverage/profile19, SpriteWave maintex+sphere mask/profile20, wrong material/role/texture tuple fail-closed, generic square-card 비확장.
9. Client and Server x64 Debug/Release isolated-output build/link.
10. 정본 regression은 다른 세션의 미완성 Effect/Valtan artifact와 분리해 이번 변경 때문에 생긴 실패만 판정한다.

## 5. 사용자 수동 확인

- 새 Server/Client에서 차원술사 LMB 한 번은 BA1만 약 1.4초 실행되는지 확인한다.
- LMB를 연타하면 BA1 -> BA2 -> BA3 -> BA4가 이어지는지, 누르고 있어도 연계되는지 확인한다.
- BA 중 RMB/quick-slot을 누르면 현재 한 동작이 끝난 직후 BA 연계보다 먼저 실행되는지 확인한다.
- Character Select 재진입/클래스 변경에서 loading 정지 구간이 줄고 선택 class Effect가 첫 재생부터 보이는지 확인한다.
- R은 첫 두 타에 검격 하나씩, 마지막 타에 검격 세 개가 각 cue 시점에 보이는지와 helix가 원형으로 임의 회전하지 않는지 확인한다.
- 새 build의 실제 T skill cue에서 summon이 잘못된 OPAQUE 면을 노출하지 않는지, 두 조각처럼 보였던 현상이 원본 MASKED 실루엣인지 별도 중복인지 사용자가 최종 확인한다. 자동 검증은 이중 제출이 없음을 보장하지만 시각 충실도 판정을 대신하지 않는다.

Client 실행과 visual/조작감 PASS 판정은 사용자가 수행한다.

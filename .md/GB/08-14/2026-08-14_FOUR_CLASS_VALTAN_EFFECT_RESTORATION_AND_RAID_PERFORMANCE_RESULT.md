# 2026-08-14 4캐릭터·발탄 이펙트 복원과 레이드 성능 구현 결과

## 1. 이번 변경의 결론

Artist F의 사용자 visual 승인을 기준으로 Track A를 제품 런타임과 병행하는 두 번째 경로로
확장하지 않고, authored schema v13 문서 하나를 ordinary `Stage_Document`로 재생하는 방향을
유지했다.

이번 변경에서 자동 검증까지 닫힌 범위는 다음과 같다.

- 공용 Track A element import transaction과 rollback 계약
- executable Track A 12 BA stage와 Warlord 17000 BA1 canary를 위한 13-stage batch seam
- Valtan 420633 Whirlwind active stage의 exact source mapping, authored v13 canary, Model View join
- Valtan animation/effect의 authoritative Server timeline 전달
- Effect Tool과 runtime의 반복 validation, document copy, frame allocation, anchor rebuild 일부 제거
- owner/world aggregate effect budget과 remote cosmetic suppression
- Server ingress/outbound backpressure, slow reader 격리와 reliable terminal flush

4직업 전체 intake는 51 skills/74 stages다. 13-stage seam은 Track A executable packet이 실제로
존재하는 첫 자동 materialization 분모이지 전체 복원 완료 분모가 아니다. 나머지 stage는 기존
Legacy tuning 또는 generic starter를 새 `.unified` 후보로 이관하되, visual 승인 전에 기존 제품
문서와 mapping을 바꾸지 않는다.

Valtan Whirlwind canary는 9개 first-LOD carrier 중 3개만 ordinary authored v13으로 실행 가능하다.
따라서 Model View 검증 대상으로는 준비됐지만 제품 catalog/animevent mapping은 계속 차단했다.

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

## 5. 4직업 batch seam

첫 batch는 다음 13 stage를 대상으로 한다.

- Artist 31000 BA1~BA4
- DimensionMaster 2050010 BA1~BA4
- LanceMaster 34010 BA1~BA4
- Warlord 17000 BA1 canary

Artist F 31470은 이미 승인된 별도 vertical slice이므로 이 batch에서 제외한다. batch는 79개의
element plan을 저장하며 Track A selected row 70개, Lance supplemental row 4개, Warlord selected
carrier 5개로 구성한다. 기존 Track A fail-closed/excluded 63개는 분모에서 삭제하지 않는다.

material disposition은 `TYPED_EXECUTION`, `ADMITTED_SOURCE_PROFILE`, `FAIL_CLOSED` 중 정확히
하나다. 현재 첫 batch는 admitted source profile 16, fail-closed 63이며 typed execution은 0이다.
Warlord canary 5개는 source material packet이 닫히지 않아 전부 fail-closed다.

각 target은 기존 제품 ID/path가 아니라 별도 `.unified` candidate ID/path를 사용한다. 최초 생성은
`MUST_NOT_EXIST`, 재import는 exact candidate baseline SHA가 일치할 때만 허용한다. 기존 product
document, EffectCatalog, animevent mutation count는 0이어야 한다.

## 6. Valtan Whirlwind vertical slice

### 6.1 authoritative identity

- encounter pattern: `VALTAN_WHIRLWIND`
- source action: `420633`
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

정량 분모는 first-LOD carrier 9, portable recipe evidence 5/9, executable 3/9,
fail-closed 6/9다. source occurrence 4개 중 완전 visible은 notify-006 한 개다.

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

- Valtan boss-pattern validator PASS
- Valtan canary builder `--check` PASS
- Valtan Python unit test 14/14 PASS
- Valtan authored v13 C++ codec parse/canonical SHA PASS
- `ClientFrontendHarness --skill-binding-fast` 24/24 PASS 후 latest mapping 교차 계약 추가
- `ClientFrontendHarness --effect-runtime-fast` 24/24 PASS
- Server Debug temp build와 contract test PASS
- Server Release build와 contract test PASS
- graceful flush/hard-stop overtake 포함 Server contract test 10회 연속 failures 0
- Release 4-player Valtan network harness: initial 4/4, fifth ROOM_FULL, replacement reconvergence,
  second generation 4/4 PASS

후속 publisher/materializer 변경을 포함한 현재 전체 source의 Client Debug/Release build와 focused
harness 최종 재실행 결과는 아래 완료 시점에 추가한다.

## 10. 사용자 수동 검증 경로

에이전트는 Client/UI를 실행하거나 visual fidelity를 대신 판정하지 않았다.

Valtan 수동 검증 경로는 Effect Tool에서 authored
`effect.valtan.pattern.420633.active`를 열고 Model View의 `Boss_Valtan`과
`mesh_att_battle_20_03`이 자동 선택되는지 확인하는 것이다. 재생 시 현재 visible 분모는
notify-006 WWind의 Sprite 1 + Mesh 2다. Trail, Dust, Light가 보이지 않는 것은 현재 명시
fail-closed 상태와 일치한다.

4직업 `.unified` 후보는 skill/stage별 Model View, Play All, Family, Solo를 확인한 뒤에만 product
mapping을 전환한다. Candidate가 존재한다는 사실은 visual PASS가 아니다.

## 11. 남은 경계

- 51 skills/74 stages 전체 중 Track A executable packet이 없는 61 stage의 Legacy/generic starter
  candidate materialization
- admitted material packet이 없는 Warlord canary와 다수 source row
- Lance Ribbon과 AnimationTrail의 서로 다른 typed runtime
- Valtan Trail, Dust material/resource, Light와 unresolved Effect/PawnMaterialParam/ViewShake
- Valtan product catalog/mapping과 실전 4-client authoritative-age visual 승인
- MeshParticle instancing, trail ring/incremental upload, thumbnail async pipeline
- GPU/CPU p95/p99, frame allocation, draw/upload, UI stat count의 실측 telemetry
- bounded session API 뒤의 IOCP/overlapped transport 교체
- 실행 중인 Debug Server가 이전 binary이면 사용자 재시작 후 새 queue/metrics 정책 적용

따라서 이번 결과는 복원과 성능의 첫 공용 실행 경계를 구현한 상태다. 4직업 전체 visual 복원,
Valtan Whirlwind 제품 승인과 4-client 60 FPS는 아직 완료로 기록하지 않는다.

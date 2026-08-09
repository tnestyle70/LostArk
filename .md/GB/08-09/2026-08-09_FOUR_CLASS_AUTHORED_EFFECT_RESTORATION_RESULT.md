# 2026-08-09 4직업 Authored 전투 이펙트 복원 결과

## 1. 결과

차원술사, 창술사, 도화가, 워로드의 현재 전투 입력을 원본 Cascade 근거에서 Authored
standalone Mesh/Sprite 제품으로 연결했다. 제품 재생은 기존
`Character animevent -> CEffectPresentationService -> CEffectCatalog -> CEffectObject`
경로 하나만 사용한다.

| 직업 | 전투 스킬 | Server action/combo stage | 제품 clip cue |
|---|---:|---:|---:|
| `DIMENSIONMASTER` | 12 | 15 | 19 |
| `LANCE_MASTER` | 17 | 27 | 41 |
| `ARTIST` | 9 | 15 | 14 |
| `WARLORD` | 13 | 17 | 27 |
| 합계 | 51 | 74 | 101 |

74 stage는 effect-bearing 73개와 원본상 시각 notify가 없는 Artist 31210 한 stage로
구분된다. 실제 animation binding은 113 clip occurrence이며 visual 102, silent 11이다.
visual occurrence 54개가 기존 stage target 53개로 귀결되고, 다중 clip stage에서 파생한
clip-local target 48개를 더해 최종 Authored target/cue는 101개다.

## 2. Source와 Authored 경계

- Imported, normalized Cascade graph, source receipt는 읽기 전용 원본 근거로 유지한다.
- stage provenance target은 unique 72개다. generated 71개와 보호된
  `effect.dimensionmaster.skill.2050210.authored-baseline` 1개이며 Artist 반복 stage 1건은
  검증된 target을 공유한다.
- generated 71문서의 2,136 element는 Mesh 872, Sprite 1,264다.
- 보호된 차원술사 A는 Mesh 20, Sprite 4를 그대로 유지한다.
- 최종 runtime 합계는 Mesh 892, Sprite 1,268, Particle 0, enabled SourceRecipe 0이다.
- Sprite `-90도`는 전역 코드가 아니라 각 Authored Sprite element에 저장된다.
- slash/impact attachment는 Player root의 occurrence 시점과 facing을 `follow=false`로
  snapshot한다. outer product cue는 root를 follow한다.

FINAL4 drawable 결정은 다음과 같다.

- generated Mesh 872: 기존 Base 716, 자기 source texture Base alias 74,
  같은 Cascade group의 hash-pinned donor Base alias 82
- generated Sprite 1,264: 기존 Base 1,176, 같은 source 기반 Base alias 88
- missing Base 0, `useModelMaterial=true` 0, cross-group/외부 placeholder 0
- `acceptedStandardApproximation` 2,120 element는 disabled source profile을 사용한다.
- 검증된 `sourceMaterialPreserved` 16과 보호된 차원술사 baseline 24는 executable
  source profile 및 product-gate hash를 유지한다.

Particle-derived carrier의 emitter delay, finite burst envelope, base/seeded lifetime,
raw 3D size 축 변환과 size-over-life endpoint는 standalone timing/scale로 굽고 선택 근거와
손실 disposition을 receipt에 보존했다.

## 3. Animation timing과 프레임 드랍 경계

다중 clip stage를 첫 clip의 재생률로 진행하지 않는다. 시각 요소가 있는 각 animation clip이
clip-local Authored cue를 소유하고 기존 Character가 계산하는 `playMs`, `playRate`, HOLD loop,
Server snapshot age와 late-catch-up을 그대로 소비한다. clip 종료 뒤에 남는 emitter/burst tail은
해당 clip cue의 natural lifetime으로 유지한다.

Character gameplay 준비 시 실제 승인된 animevent cue target만 catalog revision 단위로
transactional prewarm한다. Product Spawn은 prepared bundle만 붙이며 전투 Update에서 shader core,
model, DDS, vector-field를 로드하거나 document를 동기 stage하지 않는다. 준비 실패는 기존
catalog/cache를 유지하고 prepared miss는 fail-closed한다.

## 4. Character Select 오디션

제품 밸런스 JSON은 변경하지 않았다. `_DEBUG`의
`WORLD_ID::CHARACTER_SELECT_ARENA`에서만 성공한 skill action의 cooldown을 Server가
`Add_ServerTicksSkippingReservedZero(actionStartTick, 90)`으로 설정한다. Server 30 Hz에서
예약된 tick 0과 wrap을 포함해 정확히 3초이며, resource는 오디션용으로 전량 복구한다.
Release의 authored cooldown, action-running, combo, aim, root motion, damage와 snapshot 권위는
변경하지 않았다.

## 5. 제품 publish 결과

- Data Effect catalog와 네 class animevent: exact product ID 101개
- Runtime Effect catalog: Effect 101, Component 555, Emitter 2,160
- Runtime component reference 누락/고아 0
- UTF-8 Display Name 제한 위반 0, 최대 63 bytes
- 프로젝트 등록: files 1,348, filters 172
- Runtime catalog SHA-256:
  `b0510eaa5c0c176ba86a814afffc4acb07a9279c5ba705f02ae231ce0211ae3f`
- Rollout receipt SHA-256:
  `989211bf830c6037e685376d0aa39db4354f0514ee5f752b6d8628f44333eff1`
- Authoring catalog SHA-256:
  `98df66f186cbcc4b75b45dea667eeef6238b42702a344bbe5c4990c7ae9f746c`
- Stage coverage SHA-256:
  `e87c9bfa18df7d0f9d4f5945070ea9da795f27e55d52d7f579a67ba353072f97`

## 6. 자동 검증

PASS:

- source intake unit 7
- Authored approximation unit 33
- DimensionMaster baseline unit 13
- strict representative materializer unit 30
- product publisher unit 11
- component builder unit 28
- explicit generated refresh 29와 재실행 pending 0
- `Publish-Effects.ps1 -Mode Validate`와 `-Mode Publish`
- publisher authoring/runtime exact check 101/555
- `Test-FourClassAuthoredRollout.ps1`
- `Test-EffectRuntimePrewarm.ps1`
- `Test-CharacterActionPresentationTiming.ps1`
- `Test-EffectToolFinal.ps1`: documents 101
- Debug `ClientFrontendHarness --skill-binding-fast`: failures 0
- Debug `ClientFrontendHarness --effect-executor-fast`: failures 0
- Debug/Release `ClientFrontendHarness --effect-runtime-fast`: failures 0
- Debug Client build/link 및 PhysX runtime 배포
- Debug Server build와 `Server.exe --contract-test`: failures 0
- JSON 1,348개와 project/filter XML parse
- scoped `git diff --check`, transaction temporary/backup debris 0
- 독립 최종 비평: P0/P1 없음

전체 `Invoke-ProjectAudit.ps1`에서 이번
`effect.four-class-authored-clip-product-exact101` gate는 PASS했다. 전체 exit code 1은 공유
워크트리의 다른 lane 두 건이다.

1. `maps.product-editor-visual-scope`: Bern/Valtan/MapTool preview 계약
2. `rendering.profile-parser-contract`: authored
   `globalQuality.fxaaEdgeThreshold`가 허용 범위 `[0.0312, 0.333]`보다 작음

이 두 항목은 Effect 제품 publish나 Character timing 실패로 처리하지 않았다.

## 7. 수동 저작 경계

최초 seed 뒤 F1 All Effects의 실제 Product cue가 여는 101개 Authored 문서가 수동 정본이다.

- derived 48문서는 이전 rollout receipt의 file SHA와 다르면 기본 `--write`도 교체 전에 실패한다.
- 의도적 재생성은 `--migrate-managed-projections --write`를 명시해야 한다.
- retained 53문서는 materializer의 기본 실행이 수동 drift를 덮지 않으며, 검증된 이전 pair와
  `--refresh-generated`를 명시한 migration에서만 교체한다.
- Imported/SourceRecipe와 보호된 차원술사 2050210 baseline은 계속 자동 덮어쓰기 대상이 아니다.

따라서 F1에서 위치, scale, tiling, material, Sprite roll을 저장한 뒤에는 mass generator를
일반 갱신 명령처럼 사용하지 않는다. 제품 반영은 기존 Effect builder/publisher/reload 경로를 사용한다.

## 8. 수동 검증 미완료

자동 사진·스크린샷 비교는 실행하지 않았다. 다음 항목은 사용자가 동일 Character Select 카메라와
렌더 프로필에서 직접 판정해야 한다.

1. `Server + Client -> Character Select -> class 선택 -> Server Play -> F6 Follow`
2. 네 직업의 모든 현재 전투 키를 첫 입력과 3초 뒤 반복 입력으로 확인
3. combo/HOLD/COUNTER의 clip 전환과 이펙트 cue timing 확인
4. Player 이동·facing 기준 occurrence anchor 확인
5. F1 Profiler에서 새 JSON을 저장해 과거 약 5초 stall 제거와 실제 FPS 확인
6. F1 All Effects에서 silhouette, 위치, scale, tiling, material, Sprite garnish를 원작 PNG와
   눈으로 비교해 수동 튜닝

새 profiler와 실제 화면 확인 전에는 “프레임 드랍 해결 완료” 또는 “원작 외형 합격”으로 판정하지 않는다.

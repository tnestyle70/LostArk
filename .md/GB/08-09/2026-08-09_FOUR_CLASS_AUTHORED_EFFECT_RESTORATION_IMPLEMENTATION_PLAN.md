# 2026-08-09 4직업 Authored 전투 이펙트 복원 구현 계획

## 0. 2026-08-09 최종 실행 계약

이 절은 아래의 대표 4개 선행 조사보다 우선하는 현재 실행 계약이다. 대표 4개만 완벽하게
닫을 때까지 나머지를 막지 않고, Character Select Server Arena에서 현재 연결된 네 직업의
모든 전투 스킬과 콤보 단계를 프레임 드랍 없이 사용할 수 있는 저점을 먼저 완성한다.

### 0.1 완료 범위

| 직업 | 현재 전투 스킬 | 실제 action/combo stage |
|---|---:|---:|
| `DIMENSIONMASTER` | 12 | 15 |
| `LANCE_MASTER` | 17 | 27 |
| `ARTIST` | 9 | 15 |
| `WARLORD` | 13 | 17 |
| 합계 | 51 | 74 |

`SPACE`, stance 전환, identity/control 입력은 제외한다. 완료 단위는 숫자 skill ID만이 아니라
현재 `PlayerSkills.json` 전투 슬롯과 실제 `*.skillbindings.json`이 소유하는 action/combo
stage다. 실측 범위는 74 stage와 그 안의 정확한 113 clip occurrence다. 이 중 102개 clip에는
시각 carrier가 있고 11개 clip은 source 기준 시각적으로 비어 있다. 제품 timing은 stage 첫 clip의
rate로 전체 문서를 억지 재생하지 않는다. 각 시각 clip이 자신의 `playMs`, `playRate`, loop와
late-catch-up을 그대로 소비하는 clip-level Authored cue를 소유한다. 반복 clip 1건의 검증된 공유를
제외한 최종 제품 target/cue는 101개다. 74 stage roll-up은 effect-bearing 73, 원본상 의도적으로
silent인 Artist 31210 한 stage다.

### 0.2 G0 — 전투 중 동기 로드 제거

스킬 입력 시점의 shader compile, model load, DDS load를 먼저 0으로 만든다. 실제 animation
product cue가 참조하는 effect asset 집합을 Character Select gameplay 준비 전에 transactional
prewarm하고, gameplay `Spawn`은 준비된 bundle만 붙인다. 준비되지 않은 cue는 전투 Update에서
동기 로드하지 않고 fail-closed한다. Effect shader core는 device당 한 번 공유하고, immutable
non-animated model/SRV는 bundle에서 공유하며, animation pose와 particle/trail mutable state는
인스턴스별로 유지한다. catalog reload 준비가 실패하면 기존 catalog와 prepared revision을 함께
보존한다. 기존 cue timing을 늘리거나 frame delta clamp로 stall을 숨기지 않는다.

### 0.3 Mesh-first Authored approximation

첫 제품 복원은 Particle evaluator를 사용하지 않는다. Imported와 source receipt는 원본 truth로
그대로 보존하고, 별도 Authored output에서 다음 규칙으로 standalone carrier를 만든다.

- stage에 원본 Cascade Mesh renderer가 있으면 standalone Mesh를 본체로 선택한다. occurrence당
  최대 5개를 허용하고 source model, texture, material/profile, notify timing을 유지한다.
- Particle 동작이 필요한 Mesh renderer도 검격 본체로 유용하면 `conversionDecision`을
  `reviewedStandaloneApproximation`으로 기록한 뒤 standalone Mesh로 변환한다. 원본 eligibility,
  reason code, source element hash와 제거되는 모든 module의 `acceptedApproximation` 근거는 지우지 않는다.
- Mesh가 있는 stage의 Sprite는 실루엣에 기여하는 원본 source element 1~3개만 보조로 선택한다.
  Mesh가 전혀 없는 stage는 standalone Sprite 1~3개를 본체 대체로 사용한다.
- Sprite `-90°`는 전역 하드코딩하지 않고 각 Authored Sprite element의
  `billboardRollDegrees`에 저장한다.
- source Mesh와 Sprite가 모두 없고 원본 clip의 visual notify 자체가 0인 stage는
  `sourceIntentionallySilent`로 기록하며 빈 effect나 다른 스킬의 generic placeholder를 만들지 않는다.
  visual notify가 있는데 runtime resource를 resolve할 수 없는 stage만 완료 게이트에서 실패시킨다.
- source renderer가 ribbon처럼 이번 제품 carrier와 다르지만 model/texture/material 근거가 정확히
  resolve된 예외는 자동 승격하지 않는다. stage manifest가 `reviewedRendererApproximation`을 명시하고
  원본 renderer subtype, UNKNOWN reason과 제거 module 전부를 보존할 때만 standalone Sprite seed로
  근사할 수 있다.
- source numeric skill ID가 현재 gameplay skill ID와 달라도 숫자를 복사해 추측하지 않는다.
  현재 binding의 exact clip을 source animnotify/receipt에 join하고 source/product identity를 따로 기록한다.
- `acceptedStandardApproximation` 재질만 `effect.standard`와 disabled `sourceProfile`을 사용하고
  원본 material/profile hash는 receipt에 보존한다. 검증된 `sourceMaterialPreserved`와 보호 중인
  차원술사 2050210 수동 baseline은 executable source profile과 product-gate hash를 그대로 유지한다.
- base texture가 없는 Mesh/Sprite는 먼저 같은 source element의 resolved texture를 deterministic base
  alias로 사용하고, 없으면 같은 Cascade group 안에서만 hash-pinned donor를 선택한다. donor element/order/
  slot/asset/file hash를 receipt에 남기며 다른 group·스킬이나 generic placeholder resource를 차용하지 않는다.
- particle-derived standalone의 lifetime은 finite burst carrier면 `ParticleModuleLifetime`의 검증된
  최대 수명, continuous carrier면 emitter window를 사용한다. base/seeded alias와 결정 근거를 receipt가 소유한다.
- particle-derived standalone의 scale은 원본 `ParticleModuleSize`를 Mesh `(X,Z,Y)` 또는 Sprite
  `(X,Y)` 축으로 변환해 굽고, size-over-life가 있을 때만 `endScale` lerp를 만든다. 현재 양수 scale
  schema가 표현하지 못하는 mirror sign 손실과 선택한 deterministic sample은 accepted approximation으로 기록한다.

standalone slash/impact는 outer cue가 Player root를 follow하고, 각 occurrence의 inner attachment가
시작 시점의 Player root와 facing을 `follow=false`로 snapshot한다. class/skill/stage별 transform과
timing은 서로 복사하지 않고 각 Authored 문서가 소유한다. 최초 자동 seed 뒤에는 F1 All Effects에서
실제 product cue를 선택하여 손으로 위치, scale, tiling, material과 Sprite roll을 다듬는다.

### 0.4 Character Select 오디션 규칙

제품 밸런스 JSON의 cooldown은 바꾸지 않는다. `_DEBUG`의
`WORLD_ID::CHARACTER_SELECT_ARENA`에서만 Server가 성공한 skill action의 cooldown 종료 tick을
예약된 tick `0`을 건너뛰는 `Add_ServerTicksSkippingReservedZero(actionStartTick, 90)`으로 덮어써
wrap 구간까지 정확히 3초로 만든다.
command, action-running, combo stage,
resource, aim, root motion과 snapshot 권위는 기존 Server 경로를 그대로 사용한다.

검증 경로는 `Server + Client -> Character Select -> class 선택 -> Server Play 승인 -> F6 Follow -> 전투 키`다.
빌드와 Client 실행은 허용한다. 자동 사진·스크린샷 비교는 합격 증거로 사용하지 않으며, 자동 검증은
compile/load counter, cue timing, JSON/catalog/cue 계약과 profiler frame time으로 닫고 최종 외형은
사용자가 같은 카메라와 렌더 프로필에서 직접 판정한다.

## 1. 목표

차원술사, 창술사, 도화가, 워로드의 현재 제품 전투 스킬 바인딩을 원본 Cascade 근거에서 복원한다.
첫 단계 제품 출력은 standalone Mesh와 Sprite만 사용한다. Cascade Mesh/Sprite Particle의 원본
분류와 module 근거는 Imported/receipt에 보존하고, 명시적으로 승인한 carrier만 별도 Authored
standalone 근사로 변환한다. 제품 문서의 Particle과 enabled SourceRecipe는 모두 0이다.

이 작업은 별도 이펙트 런타임을 만들지 않는다. 최종 제품 재생은 계속
`Character animevent -> CEffectPresentationService -> CEffectCatalog -> CEffectObject -> CEffectPlayback`
한 경로만 사용한다. Imported/SourceRecipe는 읽기 전용 원본이고 수동 보정은 별도 Authored 문서가 소유한다.

## 2. 사용자 확정 계약

1. 범위는 전투 스킬만이다. 이동 전용 `SPACE`, 자세 전환과 비전투 identity/control 슬롯은 이번 복원 목록에서 제외한다.
2. 내부 분류는 네 종류를 유지한다.
   - standalone Mesh
   - Cascade Mesh Particle
   - standalone Sprite
   - Cascade Sprite Particle
   UI에서는 뒤의 두 Sprite subtype을 한 그룹 아래에 보여 줄 수 있지만 데이터에서는 합치지 않는다.
3. 차원술사 A 한 타의 기준 구성은 보라색 core Mesh 4개, white-echo Mesh 1개, Sprite 1개다.
4. 검격은 Player root를 바깥 Effect가 계속 따라가고, 각 hit occurrence가 시작되는 순간의 Player root와 facing을 한 번 snapshot한다. 생성된 검격은 이후 world-fixed다.
5. 직업 사이에는 역할, 분류, 저작 절차와 검증 구조만 공통화한다. Transform, timing, occurrence, bone/socket은 직업·스킬·콤보 단계가 각각 소유한다.
6. Sprite `-90도`는 전역 하드코딩이 아니다. 새 Authored Sprite element의 correction 기본값으로 저장하고 element별 override를 허용한다.
7. F1 All Effects의 제품 재생 대상은 Imported source가 아니라 실제 animation cue가 가리키는 Authored Effect다. Imported는 Source/진단 보기로만 남긴다.
8. 대표 네 스킬을 먼저 닫고 Character Select Server Arena에서 하나씩 검증한 뒤 같은 절차를 나머지 전투 스킬에 확장한다.

## 3. 대표 4개 완료 단위

| 순서 | class ID | 입력/skillId | animation 단계 | 첫 완료 기준 |
|---:|---|---|---:|---|
| 1 | `DIMENSIONMASTER` | `A / 2050210` | 1 clip, 내부 4 hit | 기존 5 Mesh + 1 Sprite/타의 Authored baseline과 실제 A cue를 기준으로 계약 고정 |
| 2 | `LANCE_MASTER` | `LMB / 34010` | 4 combo stages | 각 stage의 source occurrence를 따로 분류하고 Authored product cue로 재생 |
| 3 | `ARTIST` | `LMB / 31000` | 4 combo stages | 각 stage의 root/bone, timing, local transform을 도화가 소유값으로 저장 |
| 4 | `WARLORD` | `LMB / 17000` | 3 combo stages | 워로드 action-bound source catalog에서 증명된 occurrence만 사용 |

차원술사 A의 Mesh 숫자와 Transform을 다른 직업에 복사하지 않는다. 대표 네 스킬은 파이프라인의
서로 다른 입력을 검증하는 세로 단위이며, 공통 template은 최종 수치가 아니라 생성·편집 구조의 seed다.

## 4. 현재 실측 기준

### 4.1 차원술사 A

제품 cue는 `DimensionMaster.animevents`에서
`effect.dimensionmaster.skill.2050210.authored-baseline`을 `root/follow`로 재생한다.
Authored 문서는 4 occurrence에 Mesh 20개와 Sprite 4개, Particle 0개를 가진다.
각 occurrence의 모든 layer는 같은 delay와 `root/follow=false` attachment를 사용하므로 해당 시점의
이동된 Player root/facing을 캡처한다. 저장 좌표는 고정 world position이 아니라 Player root-relative local transform이다.

첫 검격이 화면 오른쪽 아래에 보였던 것은 곧바로 anchor 유실을 뜻하지 않는다. 현재 top-down 카메라는
Player의 `+Z` 쪽에서 바라보므로 Player-left `-X`, Player-forward `+Z`가 화면에서 오른쪽/아래로 투영될 수 있고,
스킬 요청 시 mouse ground aim이 Player yaw를 다시 정한다. 최종 판정은 화면 사분면이 아니라 캡처 시점의
`Local * PlayerRootWorld` 계약과 실제 action cue를 기준으로 한다.

### 4.2 창술사와 도화가

두 직업 모두 animation skillbinding, normalized source graph, skill source receipt와 resource source manifest가 있다.
기존 README의 “runtime payload 미추출” 문구는 현재 상태와 맞지 않는다. 다만 source receipt가 존재한다는 사실과
제품 Authored Effect가 완성됐다는 사실은 구분한다. materializer는 receipt가 증명한 package/object와 runtime resource만 사용한다.

### 4.3 워로드

워로드는 과거 unbound-only 상태에서 진전되어 현재 `Warlord.skillbindings.json`, `.animevents`,
`PlayerSkills.json` 전투 바인딩과 action-bound particle resource catalog를 가진다. 과거 결과 문서는 당시 추출 결과로
보존하되, 신규 구현은 현재 action-bound catalog를 정본으로 사용한다. 이름 추측으로 스킬 ownership을 만들지 않는다.

## 5. 분류와 복원 정책

### 5.1 source renderer와 최종 carrier를 구분한다

추가 실측 결과, 대표 네 스킬의 원본 그래프에 처음부터 `kind=mesh` 또는 `kind=sprite`인
standalone 노드는 없다. 원본은 모두 Cascade ParticleSystem이고 Mesh/Sprite는 emitter renderer
shape다. 차원술사 A의 현재 5 Mesh + 1 Sprite도 원본의 선택 emitter를 수동 Authored carrier로
변환한 결과다.

따라서 이 계획에서 “standalone Mesh/Sprite”는 원본 노드 이름이 아니라 최종 Authored 출력의
복원 방식이다. 내부 파이프라인은 다음 두 축을 동시에 보존한다.

- source truth: Cascade Mesh renderer / Cascade Sprite renderer와 module provenance
- authored disposition: single-carrier 변환 / particle evaluator 유지 / 이번 출력 제외

source 이름이나 mesh resource 존재만으로 single-carrier라고 판정하지 않는다. occurrence당 spawn 수,
rate/burst, velocity/orbit, size·rotation random과 lifetime 분포를 검사해 particle 동작 없이 같은 결과를
낼 수 있음이 증명되거나, correction manifest가 source emitter를 명시적으로 선택해야 한다. 그렇지 않으면
candidate/diagnostic으로만 남고 제품 Authored 문서를 생성하지 않는다.

| 원본 분류 | 이번 출력 | 저장 경계 | 비고 |
|---|---|---|---|
| Cascade Mesh renderer, single-carrier로 승인 | 포함 | Authored Mesh element | source model, texture, material/profile 근거 보존 |
| Cascade Mesh renderer, particle 동작 의존 | 승인된 근사만 포함 | Authored Mesh + approximation receipt | source kind/eligibility/module disposition을 지우지 않음 |
| Cascade Sprite renderer, single-carrier로 승인 | 포함 | Authored Sprite element | 기본 roll `-90`, element별 override |
| Cascade Sprite renderer, particle 동작 의존 | 승인된 근사만 포함 | Authored Sprite + approximation receipt | 제품 Particle은 0, Imported source는 그대로 보존 |

분류는 source renderer kind와 최종 carrier kind를 모두 기록해야 한다. “Particle”이라는 이름만으로
Mesh renderer와 Sprite renderer를 합치거나, `Play Mesh Emitters`라는 UI 이름으로 standalone Mesh를 누락하지 않는다.

## 6. 데이터와 파이프라인

```text
normalized Cascade graph / action notify
  -> class skill source inventory
  -> skill source receipt (package/object/occurrence/resource provenance)
  -> four-way classification
  -> Authored correction manifest
  -> fail-closed materializer
  -> Data/Effects/Authored/<stage provenance container>.effect.json
  -> exact binding clip projection (clip-local delay/playMs/playRate receipt)
  -> Data/Effects/Authored/<stable product clip effect>.effect.json
  -> existing component builder
  -> existing Effect publisher
  -> transactional runtime catalog reload
  -> actual animation asset cue
  -> Character Select Server Arena action verification
```

materializer의 필수 조건은 다음과 같다.

- source receipt와 normalized graph의 identity/hash가 맞지 않으면 실패한다.
- runtime model/texture/material source가 정확히 resolve되지 않으면 fallback asset을 만들지 않고 실패한다.
- source graph와 zero-unresolved external module closure가 없으면 single-carrier 판정을 완료하지 않는다.
- 직업·stage별 correction manifest가 승인 emitter와 역할을 명시하지 않으면 빈 generic 제품 Effect를 만들지 않는다.
- particle source는 raw kind와 제거 module disposition을 receipt에 남기고 승인된 standalone carrier로만 출력한다.
- Sprite correction은 전역 코드가 아니라 output element에 명시한다.
- 기존 수동 Authored 출력은 자동 덮어쓰지 않는다. 최초 seed 뒤 F1에서 저장한 101개 Product Authored가
  수동 정본이다. 두 mass generator의 기본 `--write`는 이전 receipt hash와 다른 수동 변경을 fail-closed하고,
  검증된 이전 pair와 명시적 `--refresh-generated` 또는 `--migrate-managed-projections`를 함께 준 migration만
  transaction으로 교체한다. 차원술사 2050210 수동 baseline은 항상 보호한다.
- Imported와 SourceRecipe 문서는 수정하지 않는다.

## 7. F1 All Effects 계약

각 skill row는 최소한 다음 두 대상을 구분해 표시한다.

- `Source`: Imported/canonical effect와 원본 분류·provenance 진단
- `Active Product Cue`: 현재 선택 animation clip/stage의 실제 asset cue가 가리키는 Authored effect

`Play Product`는 `PlayerSkills.effectId`의 canonical ID를 추측하지 않고 실제 animation cue target을 사용한다.
제품 preview는 cue의 root/bone, follow, local transform을 보존하고 선택 pivot을 강제로 Player root로 바꾸지 않는다.
편집 중 unsaved preview는 기존 `CEffectObject` evaluator를 직접 stage할 수 있지만, 최종 합격은 Publish 뒤 실제
catalog와 presentation cue를 통과한 action 재생이다. 둘 다 같은 Effect renderer/evaluator를 사용한다.

## 8. Anchor와 occurrence 규칙

slash/impact의 기본 anchor는 Player root다. 손이나 무기에 붙어 있어야 하는 trail/glow만 해당 직업의 실제 bone/socket을 사용한다.
존재하지 않는 bone은 root로 조용히 fallback하지 않고 문서 또는 cue를 fail-closed한다.

공통 occurrence ID가 없는 현재 문서에서는 한 hit의 모든 standalone layer가 동일 delay와 동일 attachment를 가져야 한다.
이 조건을 builder/audit가 검사한다. 이후 schema에 stable occurrence ID를 추가하더라도 제품 의미는 동일해야 한다.
Cascade Sprite Particle의 원본 local-space/attachment 의미는 Imported/receipt에 보존한다. 이번 제품
standalone carrier는 별도 Authored root snapshot 정책을 명시하며 원본 사실로 표시하지 않는다.

## 9. 구현 순서

### G1. 문서와 source 정본 교정

Artist/LanceMaster/Warlord Imported README를 현재 receipt/catalog 상태로 교정하고 대표 네 스킬의 source coverage matrix를 고정한다.

### G2. 범용 분류·materializer

DimensionMaster 전용 seed 로직에서 공통 검증과 carrier 생성 부분만 범용화한다. 직업별 correction manifest가
source 선택, occurrence, attachment, local transform과 Sprite roll을 소유한다.

### G3. 대표 네 Authored 제품 문서

차원술사 A baseline을 회귀 기준으로 보호하고 네 직업 74 stage의 source 근거를 Authored provenance
container로 생성한다. 원본상 silent인 한 stage는 빈 generic effect로 채우지 않고 명시적으로 기록한다.

### G4. 실제 product cue 연결

각 시각 clip의 `.animevents` asset cue가 clip-local stable Authored effect ID를 가리키게 한다.
다중 clip stage의 carrier를 첫 clip 문서 하나에 합치지 않는다. derived clip 문서는 source semantic을
그대로 복사하고 해당 clip offset만 빼며, `playMs` 뒤 occurrence는 거부한다. `PlayerSkills.effectId`는
입력/카탈로그 탐색용 보조값일 뿐 실제 product target과 timing은 animation cue가 소유한다.

### G5. All Effects 교정

Source와 Active Product Cue를 분리하고, Product Play가 실제 cue의 anchor/follow/transform을 보존하도록 한다.
네 subtype의 count와 제외 상태를 명확히 표시한다.

### G6. Publish와 대표 4개 검증

기존 builder/publisher로만 101개 clip product runtime catalog를 생성한다. 51 skill/74 stage/113 clip
coverage, 102 visual/11 silent, exact 101 cue를 자동 검증한 뒤 Character Select Server Arena에서
대표 네 스킬과 각 직업의 나머지 전투 스킬을 순서대로 수동 검증한다.

## 10. 검증 계약

자동 검증은 다음을 포함한다.

- JSON parse, version, stable ID, duplicate, finite/range 검증
- source identity/hash 불일치, 누락 resource, 잘못된 renderer kind의 fail-closed 검증
- 네 subtype 분류 수와 Mesh Particle 출력 0 검증
- Sprite correction이 element별이며 전역 runtime hardcode가 아님을 검증
- 동일 occurrence layer의 delay/attachment 일치 검증
- 113 clip projection, 102 visual/11 silent, 101 unique target/cue와 clip-local delay 검증
- binding `playMs` 이후 element 거부, per-clip `playRate` 재사용, 반복 clip hash-equivalence 검증
- product Particle 0, enabled SourceRecipe 0, material/lifetime/raw size approximation receipt 검증
- Source와 Active Product Cue가 분리되고 Product Play가 실제 cue target을 선택하는지 검증
- builder/publisher/catalog parse-stage-commit과 실패 시 기존 runtime catalog 보존 검증
- 관련 unit test, Effect Tool audit, ProjectAudit, `git diff --check`

빌드와 Client 실행은 허용하며 컴파일·링크·셰이더·loader·runtime 오류와 profiler 수치를 검증한다.
에이전트는 스크린샷을 찍거나 이미지 유사도를 자동 판정하지 않는다. 원작 PNG 대비 silhouette, 방향,
재질과 최종 렌더 품질의 합격은 사용자가 Character Select에서 직접 한 스킬씩 확인한다.

## 11. 완료와 확장 기준

이번 세로 단위의 구현 완료는 네 직업 51개 전투 스킬의 74 stage를 113 exact clip occurrence로 추적하고,
101개 제품 Authored 문서·실제 cue·runtime catalog·prewarm set이 한 stable ID 체인으로 연결되어
비시각 검증을 통과한 상태다. 수동 시각 합격과 새 profiler frame-time 판정은 별도 상태로 기록한다.

확장 순서는 `대표 4개 승인 -> 각 직업의 나머지 LMB/ACTIVE/COMBO 전투 스킬 -> 다른 현재 바인딩 직업`이다.
Particle Mesh 복원은 별도 Track A가 evaluator와 source semantics를 검증한 뒤, 같은 Authored 제품 문서의
검증된 layer만 교체·추가한다.

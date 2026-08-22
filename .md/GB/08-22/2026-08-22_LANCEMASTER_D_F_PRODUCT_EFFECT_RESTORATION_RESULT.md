# 2026-08-22 창술사 D·F Product Effect 복원 결과

branch: `codex/lance-df-v1-restore` (base `d6684e66` = `origin/main`)

대응 계획: [`구현 계획서`](2026-08-22_LANCEMASTER_D_F_PRODUCT_EFFECT_RESTORATION_IMPLEMENTATION_PLAN.md)

## 1. 구현 상태

| 항목 | 상태 |
|---|---|
| D(34110 반월섬) Product admission | 완료 |
| F(34150 맹룡열파) Product admission | 완료 |
| runtime catalog join | 완료 (`205 -> 207`) |
| published Authored document | 완료 (D 1,592,403 byte / F 3,249,421 byte) |
| typed `material.execution` 승격 | 미착수 (범위 밖) |
| 손튜닝·육안 승인 | `USER_REVIEW_PENDING` |

### 1.1 실제 diff

```text
 .md/GB/08-22/2026-08-22_LANCEMASTER_D_F_PRODUCT_EFFECT_RESTORATION_IMPLEMENTATION_PLAN.md | new
 .md/GB/08-22/2026-08-22_LANCEMASTER_D_F_PRODUCT_EFFECT_RESTORATION_RESULT.md              | new
 Client/Bin/DataFiles/Effect/Authored/effect.lancemaster.skill.34110.unified.<sha>.effect.json | new (publisher)
 Client/Bin/DataFiles/Effect/Authored/effect.lancemaster.skill.34150.unified.<sha>.effect.json | new (publisher)
 Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json                                     | publisher
 Data/Animation/Authored/LanceMaster/LanceMaster.animevents                                 | +2 cue, header 3134 -> 3136
 Data/Effects/Authored/effect.lancemaster.skill.34110.unified.effect.json                   | particleSystem identity
 Data/Effects/Authored/effect.lancemaster.skill.34150.unified.effect.json                   | particleSystem identity
 Data/Effects/EffectCatalog.json                                                            | +2 entry (316 -> 318)
```

C++, HLSL, `.vcxproj`, `.filters` 변경 없음.

### 1.2 원인 확정

두 스킬이 보이지 않던 원인은 material/carrier 결함이 아니라 admission 결손이었다.

```text
Authored document      존재 (D 88 element / F 186 element, v13)
Imported source receipt 존재
EffectCatalog.json     부재  -> Tool addressable 아님
animevents cue          부재  -> Product membership 아님
```

`Publish-Effects.ps1`의 `Get-ActiveProductEffectIds`가 네 class animevents의 `effectref=asset`
payload를 Product membership 정본으로 쓰기 때문에, 문서와 receipt가 아무리 완전해도 이 두 곳이
비면 runtime catalog에 실리지 않는다. 창술사 19개 스킬 중 17개는 등록돼 있고 D·F만 빠져 있었다.

### 1.3 publisher가 잡아낸 실제 결손

catalog·cue를 연결한 첫 Validate가 다음으로 fail-closed했다.

```text
Required property 'uniformScaleMultiplier' has the wrong type.
```

두 Authored document의 `particleSystem`이 `{}`였다. v8 이상 문서는 네 modifier가 필수이므로
live document와 같은 identity 값(`1.0 / 0.0 / 0.0 / 1.0`)을 넣어 닫았다.

## 2. 자동 검증 (실행함)

| 검증 | 결과 |
|---|---|
| `Publish-Effects.ps1 -Mode Validate` | `PASS: validated 207 Effect catalog entries` |
| `Publish-Effects.ps1 -Mode Publish` | `PASS: published 207 Effects, 0 Components` |
| `Test-EffectPipeline.ps1` | `Ran 110 tests ... OK` |
| `Test-EffectDataProjectRegistration.ps1` | `PASS: files=1862 filters=210` |
| `git diff --check` | 경고 없음 |
| resource 존재 검사 | D 157/157, F 417/417 asset이 `Client/Bin/Resources` 아래 실재 |
| admission budget 계산 | 아래 표 |

publish 검증은 274개 element 전부가 profile별 carrier 요구(`grouped-translucent`의 Base/Mask/Emissive,
mesh particle의 `meshModel` 등)를 통과했다는 뜻이다.

### 2.1 admission cost

`CEffectPresentationService::Estimate_DocumentBudget`과 같은 식으로 계산했다.
`OWNER_BUDGET`은 `particles 8192 / meshParticles 2048 / draws 3072`다.

| document | particles | meshParticles | draw submissions | 판정 |
|---|---:|---:|---:|---|
| `34110.unified` (D) | 1,046 | 84 | 153 | owner budget 이내 |
| `34150.unified` (F) | 1,050 | 225 | 336 | owner budget 이내 |
| `34630.clip1.unified` (기존 live 대조군) | 910 | 173 | 234 | owner budget 이내 |

D·F는 이미 live인 `34630.clip1`과 같은 규모다. spawn 시 통째로 거부되는 budget 초과는 없다.

### 2.2 두 문서의 실행 tier

| profile | D | F | runtime 구현 |
|---|---:|---:|---|
| `effect.ue3.grouped-translucent.v1` | 74 | 168 | `Effect_DocumentRenderer.cpp` |
| `effect.ue3.procedural-center-glow.v1` | 8 | 0 | `Effect_DocumentRenderer.cpp` |
| `effect.ue3.circle.v1` | 0 | 18 | `Effect_DocumentRenderer.cpp` |
| `effect.ue3.missiletrail-two-emissive.v1` | 1 | 0 | `Effect_MaterialTemplate.h` |
| sourceProfile 비활성 | 5 | 0 | - |

`material.execution.enabled = true`인 typed row는 D·F 모두 0개다. 즉 두 스킬은 나머지 창술사
live 스킬과 정확히 같은 `V0_COMPATIBILITY` tier로 들어왔고, `V1_TYPED_PRODUCT`가 아니다.

## 3. 수동 검증 (미실행 — 사용자 전용)

에이전트는 Client를 실행·조작하거나 화면을 캡처하지 않았다. 다음은 사용자 판정 대상이다.

```text
Lobby -> Character Select에서 창술사(Lance Master) 선택
긴 창 스탠스에서 D(반월섬), F(맹룡열파) 사용
D: 0ms cast, 96ms spark, 490ms 지면 decal 2종, 676ms mesh trail 4종, 710ms 충격 spark
F: 180~576ms EarthQS trail, 679~776ms Ark, 1383ms HurricaneDust, 1405ms DragonSwing
```

`manual first pixel`, `eye smoke`, `visual PASS`는 사용자의 서면 판정 전까지 기록하지 않는다.

## 4. 남은 경계

### 4.1 원본 대비 실제 반영 범위

`복원`이 어디까지 원본을 반영했는지 field 단위로 분리한다.

**원본에서 그대로 온 것**

| 축 | 근거 |
|---|---|
| carrier | element별 `sourceRecipe.rendererShape`. D는 mesh 19 / sprite 64 / decal 5, F는 mesh 75 / sprite 111 |
| 발생 시각 | `detail.timing.startDelaySeconds`가 animevents `src=orig` EFFECT 시각과 일치. D는 `0/96/490/676/690/710ms`, F는 `180/232/574/575/576/776/1405ms` |
| texture 역할 | `material.sourceProfile.textures`의 `sourceObjectPath`, `group`, addressU/V, colorSpace |
| blend/depth | element별 `renderProfile` (D `alpha_two_sided 32 / additive_two_sided 21 / additive_one_sided 18 / alpha_one_sided 17`) |
| emitter recipe | `sourceRecipe.modules`에 Cascade module literal 보존, bursts / emitterDuration / loops |
| resource | D 157/157, F 417/417 asset이 실재 |

**원본에서 오지 않았거나 아직 승격되지 않은 것**

| 축 | 현재 상태 |
|---|---|
| RT0 equation | 274 element 중 242가 `effect.ue3.grouped-translucent.v1` generic 경로. family 전용 typed RT0 program은 이번 작업에서 만들지 않았다 |
| typed 배선 | `material.execution.enabled = true` row 0개. lane/register/opcode 계약 미승격 |
| conversion status | 두 receipt 모두 `presentationSourceExactEmitterCount = 0`. 전 emitter가 `SOURCE_RECIPE_RUNTIME_PENDING`(D 55 / F 56) 또는 `SOURCE_MATERIAL_RUNTIME_PENDING`(D 33 / F 31) |
| Cascade curve | receipt `manualTuningBoundaries`대로 start/end·min/max로 collapse됨 |
| mesh particle | per-particle mesh rotation, orbit, 3D size curve는 thin path로 축약 |

### 4.2 imported 대비 unified element 결손

unified 문서는 imported 변환의 부분집합이다.

| skill | imported element | unified element | 비율 |
|---|---:|---:|---:|
| 34110 (D) | 127 | 88 | 69% |
| 34150 (F) | 294 | 186 | 63% |

system 단위로 통째로 빠진 원본 emitter는 다음이다.

| skill | 빠진 source system | imported element | animevents 시각 |
|---|---|---:|---|
| D | `fx_cm_02.light.par_mp_light_01` | 5 | 735ms |
| F | `fx_pc_flm_03.par_n_flm_ark_01` | 6 | 679ms |
| F | `fx_pc_flm_03.par_n_flm_earthqs_trail_01_07` | 6 | 233ms |
| F | `fx_pc_flm_03.par_n_flm_hurricanedust_01` | 3 | 1383 / 1386 / 1392ms |
| F | `fx_cm_02.light.par_mp_light_01` | 9 | 1387ms |
| F | `fx_cm_00.dust.par_d_dust_001_pr` | 12 | 1413ms |

animevents `src=orig` 시각 대비 unified 문서가 덮은 시각은 D가 7개 중 6개, F가 14개 중 6개다.
F의 `0ms Par_N_Trail_03_02`는 imported 단계에도 없어 source intake부터 필요하다.
같은 system이 부분만 남은 행(`ark_04 21/42`, `dragonswing_02 24/33`, `spark_24 12/19` 등)도 있다.

이 결손은 이번 admission 변경이 만든 것이 아니라 unified 문서가 이미 가지고 있던 상태다.
master 계획 `4.2 V0에서 삭제한 source element를 복구하는 3-way merge`의 대상이다.

### 4.3 attachment

F의 186 element 중 `actionCueAttachment.enabled`인 행은 0개다. 전부 cue root transform에서 스폰된다.
D는 5개(decal)만 root anchor를 사용한다.

### 4.4 같은 admission 결손이 남아 있는 다른 slot

`skillbindings` clip 대 `effectref=asset` cue를 대조한 감사 결과다. D·F는 이 목록에서 사라졌다.

| class | slot | skillId | displayName | 결손 clip | Authored 문서 |
|---|---|---|---|---|---|
| Artist | R | 31210 | 필법 : 콩콩이 | 1/4 | ba1, ba4 존재 |
| Artist | X | 31110 | 떠오르는 해 | 1/1 | 없음 |
| Artist | SPACE | 31020 / 31030 | 흘리기 / 기상기 | 1/1 | 없음 |
| Warlord | R | 17110 | 리프 어택 | 1/3 | clip2, clip3 존재 |
| Warlord | T | 17240 | 풀배럴 캐넌 | 2/5 | ba1~ba3 존재 |
| Warlord | Z | 17800 / 17810 | 방어 태세 / 해제 | 1/1 | 없음 |
| Warlord | SPACE | 17020 / 17025 | 돌격 / 기상기 | 1/1 | 없음 |
| LanceMaster | A | 34140 | 선풍참혼 | 1/4 | ba1/ba2 존재 |
| LanceMaster | E | 34560 | 굉열파 | 2/4 | clip2, clip3 존재 |
| LanceMaster | R | 34570 | 유성강천 | 1/3 | clip1, clip2 존재 |
| LanceMaster | Z / SPACE | 34000, 34500, 34020, 34030, 34520 | 무기 변경·탄영·기상기·돌파 | 1/1 | 없음 |
| DimensionMaster | E | 2050160 | 건너 찌르기 | 2/5 | clip2~clip4 존재 |
| DimensionMaster | W | 2050120 | 분절 | 1/3 | clip2, clip3 존재 |
| DimensionMaster | SPACE | 2050020 / 2050030 | 공간 도약 / 기상기 | 1/1 | 없음 |

`Authored 문서 존재 + cue 결손`인 행은 이번 D·F와 같은 방식으로 즉시 복원 가능하다.
`Authored 문서 없음`인 행은 먼저 source intake와 conversion이 필요하다.

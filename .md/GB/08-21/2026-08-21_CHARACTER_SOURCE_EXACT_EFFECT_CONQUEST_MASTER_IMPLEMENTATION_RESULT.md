# 2026-08-21 Character Source-Exact Effect Conquest Master 구현 결과

기준 커밋: `812d23a4`
구현 브랜치: `codex/effect-family-conquest`
상태: 진행 중 living RESULT
최종 화면 판정자: 사용자

이 문서는 마스터 PLAN의 실제 반영 상태만 기록한다. 도화가·워로드·창술사·차원술사
전체 복원은 아직 진행 중이며, 이 버전에서는 family inventory와 action-facing 첫 수직 슬라이스를
구현했다.

## 1. 자동 완료된 범위

### 1.1 family-first restoration inventory

- `CarrierVariant × MaterialVariant(nullable) × RenderVariant × CompositionVariant`를 제품
  execution closure로 고정했다.
- provenance, evidence, executor, runtime admission, user review를 다른 축으로 보존했다.
- 도화가 F 17행, 차원술사 A 4행, 워로드 W 1행 총 22 occurrence를 machine-readable
  contract로 봉인했다.
- grayscale/DXT1을 암묵적 `base.a`로 coverage에 연결하거나, 없는 emissive lane에
  intensity만 설정하거나, default SRV로 required lane을 대체하는 계약을 validator가 거부한다.

### 1.2 차원술사 A `2050210`

- 기존 사용자 튜닝 9행을 ordered canonical identity로 보존했다.
- 현재 `.25s` `a58f...` 행을 유지하고 source occurrence `.60/.90/1.30s` 세 행만
  selective append해 보이는 MakeFlow 검격을 정확히 네 번으로 만들었다.
- 원본 `.25s` base occurrence는 evidence-only로 남기고 현재 튜닝 행을 중복 spawn하지 않았다.
- outer cue는 `follow=follow`, 각 occurrence는 시작 시점 snapshot으로 구성했다.

### 1.3 공용 `ACTION_FACING`

- animevent format v6에 `orientation=anchor|action_facing`을 추가했다.
- position follow/snapshot과 orientation authority를 분리했다.
- `action_facing`은 Server snapshot의 스킬 edge yaw를 actionStartTick과 함께 한 번만 캡고,
  actor translation/positive scale을 유지하며 local transform을 정확히 한 번 합성한다.
- mirrored/non-finite action-facing anchor는 실패 즉시 기존 prepared state를 보존한다.
- 차원술사 A와 워로드 W `17060`이 이 계약을 소비한다.
- 현재 admission은 ACTIVE 스킬만 대상이다. HOLD action은 Server-latched historical facing
  field가 추가되기 전에 이 모드로 승격하지 않는다.

### 1.4 워로드 A `17090` authored subset save

- immutable Track-A 증거는 계속 Mesh 14/Sprite 2와 chain06 8/chain07 4를 정확히 검증한다.
- 일반 authored Load/Save에서는 exact count를 강제하지 않고, 남겨 둔 chain 행 각각의 stable ID,
  mesh, material profile, source recipe, 회전과 DynamicParameter identity를 검증한다.
- 따라서 Effect Tool에서 합법적인 네 행 subset을 저장하면 atomic temporary reload가 더 이상
  원본 12행 cardinality를 이유로 거부하지 않는다.
- 중복 ID, source allowlist 밖 ID, 알 수 없는 mesh, 변조된 burst recipe는 저장을 거부하고 기존
  디스크 문서를 그대로 유지한다.

## 2. 실행한 검증

| 검증 | 결과 |
|---|---|
| Dimension A selective materializer | 9 tests PASS, `--check` PASS |
| family restoration inventory | 23 tests PASS, 22 occurrence/check PASS |
| legacy cue projection | 7 tests PASS |
| rollout orientation focused test | PASS |
| level component animevent v6 parser | 29 tests PASS |
| Debug Engine → UpdateLib → ClientFrontendHarness | PASS |
| Debug `--effect-action-facing-fast` | 8/8 PASS |
| Debug Client | 0 errors PASS |
| Release Engine → UpdateLib → ClientFrontendHarness | PASS |
| Release `--effect-action-facing-fast` | 8/8 PASS |
| Release Client | exit 0, errors 0, 00:09:28.08 |
| Debug/Release Warlord authored subset save | 각 7/7 PASS |
| Debug/Release Client after subset codec change | 각 errors 0 PASS |
| `git diff --check` | PASS |

Release Client 경고 2173건은 기존 FXC X4717/X4000, C4819, DirectXTK LNK4099 계열이며
`ACTION_FACING` 컴파일/링크 오류는 없었다.

## 3. 아직 완료로 올리지 않는 경계

- checked-in runtime catalog는 아직 차원 A의 기존 9행 sealed document를 가리킨다. 신규
  세 행은 `AUTHORING_ONLY/AUTHORED_NOT_PUBLISHED`이다.
- full Effect publisher Validate는 현재 main의 `effect.valtan.red-blade-wave.active` 중복 소유자
  검증에서 중단된다. 이 세션은 해당 Valtan 데이터를 수정하지 않았다.
- 실제 Client의 동/서/남/북 cast 방향, 검격 형태·타이밍·색감은 사용자 화면 판정 대기다.
- 워로드 W의 `fm_a_hemisphere_012` 하나가 보이는 방패 세 판을 포함하는지는 사용자
  화면 판정 후 독립 좌/중/우 cohort 추가 여부를 결정한다.

## 4. 다음 구현 단위

1. 도화가 A 8행 `uv_noise_velue=0` semantic override
2. 도화가 D effectref/catalog/runtime join
3. 도화가 R true LocalDecal + revolution/fade
4. 워로드 T RGBNoise 제거 + exact decal 4행 selective restore
5. typed color/coverage family를 기반으로 glass/dragon/attractor 고난도 family 구현

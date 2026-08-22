# 2026-08-23 Effect Tuple Cohort Inventory 구현 결과

branch: `codex/effect-tuple-cohort-inventory`

base: `origin/main@0d199aac942fe8c39ced3a6adb6fce2bee7402ed` (`PR #169` merge, `PR #168` Binding 0 포함)

plan: `2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_IMPLEMENTATION_PLAN.md`

작업 lane: `Track B — 대량 확장을 위한 evidence inventory`

## 1. 결론

4캐릭터와 Valtan의 authored Effect `416`개 문서, `7,566`개 occurrence를 전수 조사해 다음 여섯 축을
서로 독립적으로 판정하는 결정적 계약을 만들었다.

```text
Occurrence
  × Program evidence
  × Layout evidence
  × Adapter evidence
  × Descriptor evidence
  × Composition reachability
  × Product/user review
```

이 결과는 화면을 그리는 runtime 구현이나 Product 승격이 아니다. `Program × Layout identity × Adapter`가
같은 정적 재사용 후보만 `340`개 cohort, `2,024`개 occurrence로 묶었다. compiled draw와 사용자 A/B가
없는 상태를 과장하지 않도록 모든 cohort의 `runtimeVerified`와
`runtimeDescriptorExpansionEligible`은 `false`다.

도화가 F의 두 Sprite golden occurrence는 같은 tuple cohort로 구조적 dual-resolve된다. 이는 수평
공용화가 가능한 형태라는 정적 증거이고, 실제 Binding/Adapter 경로의 화면 동등성 증거는 Track A와
사용자 A/B에서 별도로 닫는다.

## 2. 구현한 계약

### 2.1 생성 도구와 테스트

- `Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py`
- `Tools/EffectPipeline/test_build_effect_tuple_cohort_inventory.py`
- `Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json`

생성 도구는 strict JSON, duplicate/identity/hash 검증, input snapshot, stage 후 atomic replace,
`--check`, artifact self hash를 제공한다. 입력 파일 집합이나 내용이 생성 중 바뀌면 기존 output을
보존하고 실패한다.

최신 main의 runtime catalog v4는 root 필드 순서를
`schema, formatVersion, materialPrograms, components, effects`로 고정하고 공식
`validate_direct_authored_effect_runtime.validate_runtime_catalog`로 nested material-program registry와
direct-authored publish 계약을 함께 검증한다. 현재 Binding 0 registry는 유효하지만 binding 행이 없으므로
Track B occurrence를 runtime-verified로 승격하지 않는다.

### 2.2 축 분리로 교정한 오해

source material 값이 없는 `4,675`행을 Program도 없는 행으로 취급하지 않는다. material path로
Program/Layout 증거를 독립 resolve하고 Descriptor만 `RESOURCE_ONLY_NO_MATERIAL_VALUES` 또는 `MISSING`으로
남긴다.

DXBC occurrence-exact도 실행 가능한 번역물 존재 여부를 분리했다.

- `DXBC_OCCURRENCE_EXACT`: exact DXBC와 literal HLSL translation이 함께 있음
- `DXBC_OCCURRENCE_EXACT_UNTRANSLATED`: exact DXBC는 있으나 실행 Program 후보는 없음
- `DXBC_FAMILY_REPRESENTATIVE_ONLY`: family 근거는 있으나 현재 occurrence 식이라고 단정할 수 없음

exact variant의 native wire, named ABI, source parameter name도 Layout 증거일 뿐 typed runtime packet이나
Descriptor closure로 자동 승격하지 않는다. child-parent receipt가 `BLOCKED`인 행은 guessed parent나 leaf
검색으로 보완하지 않고 원 blocker를 보존한다.

## 3. 전체 실측

### 3.1 분모

| domain | document | occurrence |
|---|---:|---:|
| Artist | 35 | 615 |
| DimensionMaster | 60 | 2,169 |
| LanceMaster | 102 | 2,588 |
| Warlord | 61 | 1,414 |
| Valtan | 158 | 780 |
| 합계 | 416 | 7,566 |

### 3.2 Program

| status | occurrence |
|---|---:|
| `TYPED_RUNTIME_PROGRAM_DECLARED` | 50 |
| `DXBC_OCCURRENCE_EXACT` | 2,148 |
| `DXBC_OCCURRENCE_EXACT_UNTRANSLATED` | 8 |
| `DXBC_FAMILY_REPRESENTATIVE_ONLY` | 3,671 |
| `BOUNDED_SOURCE_PROFILE_ONLY` | 525 |
| `NO_PROGRAM_EVIDENCE` | 1,055 |
| `NOT_APPLICABLE_PRESENTATION` | 109 |

### 3.3 Layout

| status | occurrence |
|---|---:|
| `TYPED_PACKET_CLOSED` | 50 |
| `EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION` | 20 |
| `NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS` | 2,430 |
| `NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION` | 3,153 |
| `SOURCE_NAMES_ONLY` | 673 |
| `UNRESOLVED` | 1,131 |
| `NOT_APPLICABLE_PRESENTATION` | 109 |

### 3.4 Adapter와 Descriptor

Adapter는 typed static candidate `50`, render-profile static candidate `7,385`, unresolved `22`, 별도
presentation `109`다. 이 수는 compiled runtime dispatch 증거 수가 아니다.

Descriptor는 typed values closed `50`, source values present but unpacked `2,732`, resource-only `4,575`,
missing `100`, 별도 presentation `109`다.

### 3.5 Composition과 Product

현재 Product consumer는 character `99`, Valtan pattern `44`, boss visual `2`, 합계 `145` assets다.
runtime catalog의 `145` assets / `2,554` authored occurrences와 정확히 일치한다. 기술 상태는
`PRODUCT_JOIN_CLOSED 2,554`, `CATALOG_NOT_PUBLISHED 2,258`, `AUTHORED_NOT_CATALOGED 2,754`로 분리했다.

정상적인 Product 증감은 생성 실패가 아니라 snapshot drift로 나타난다. 현재 composition projection
SHA-256은 `9729794c95502fb81c853d404e4fb4e7d946fecad310ddd82bc461e5b53dcfd1`이다.

## 4. cohort와 canary 결과

| 항목 | 결과 |
|---|---:|
| 전체 tuple cohort | 340 |
| cohort occurrence | 2,024 |
| `TYPED_EXECUTION_COHORT` | 19 |
| `NATIVE_EVIDENCE_COHORT` | 321 |
| runtime verified cohort | 0 |
| runtime Descriptor expansion eligible cohort | 0 |

G00 source evidence `269`행은 sealed secondary projection과 일치한다. 도화가 F의
`sprite.2b3dc6842507e910`, `sprite.c65181324417a1a8`은 동일 tuple cohort로 resolve하며 상태는
`STRUCTURAL_DUAL_RESOLVE_PENDING_USER_A_B`다. 기존 17개 legacy golden review를 horizontal V1 승인으로
재사용하지 않았고 전체 horizontal V1 review는 `NOT_RECORDED`다.

## 5. 자동 검증

```text
python Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py
  WROTE documents=416 occurrences=7566 cohorts=340

python Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py --check
  PASS

python Tools/EffectPipeline/test_build_effect_tuple_cohort_inventory.py
  Ran 30 tests, OK

python -m py_compile
  PASS

JSON parse / artifact self-hash / deterministic regeneration
  PASS
```

최종 artifact는 `31,837,272` bytes, file SHA-256
`8439efcb98b759b81ac0687937393f91c28ac132a43c235a72354e9be095289e`, canonical self hash
`4f6b0dea9727c14532ce48246bf3fbe4d7cba9528b336d7048e3ed6d4c4aaaa6`다.

최신 `origin/main` 재통합 뒤 runtime catalog v4의 Binding 0 material-program registry를 정본 validator로
검사하고 같은 검증을 다시 실행한다. Python/JSON evidence만 바뀌므로 Engine/Client build를 이번 Track B
PR의 증거로 기록하지 않는다.

## 6. 미실행과 남은 lane

### Track A — 화면에 실제로 연결하는 runtime spine

Binding 0 registry 최소 구현은 별도 PR로 main에 들어갔다. 같은 별도 작업에서 도화가 F 한 Binding의
exact dual-resolve, actual Sprite Adapter draw, Debug/Release build와 focused execution evidence를 닫는다.
Track B artifact는 그 결과를 입력 계약으로 소비하기 전까지 runtime proof를 추론하지 않는다.

### Track C — Product 복원

Track A 빌드가 준비되면 사용자가 다음을 직접 판정한다.

1. 도화가 F golden control의 기존 경로와 registry 경로 A/B
2. SOLO first pixel, 색·크기·방향·수명·blend 동등성
3. 실제 F cue의 timing·attachment 동등성
4. 같은 tuple을 쓰는 다른 캐릭터 또는 Valtan occurrence 한 개의 공용성

사용자 서면 관찰 전에는 manual first pixel, visual PASS, Product admission을 기록하지 않는다. 공용성
canary가 통과하면 Track B의 같은 tuple cohort에서 Descriptor만 추가하며 확대하고, equation·ABI·carrier/VF/pass
중 하나가 다르면 각각 Program·Layout·Adapter의 새 capability로 연다.

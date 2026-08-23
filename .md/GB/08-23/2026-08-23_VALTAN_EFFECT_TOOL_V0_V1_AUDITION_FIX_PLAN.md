# Valtan Effect Tool V0/V1 Audition Fix 계획

## 목표

발탄 All Effects가 빈 legacy stage shell을 재생 가능한 Product Effect처럼 열어 기존 Current Effect를
지우는 문제를 막고, 실제 Product cue의 V0와 optional `.v1.unified`를 같은 clip/timing으로 나란히
Open/Play할 수 있게 한다.

## 실측 기준

- 첨부 대상 `effect.valtan.center-grab-counter-64.center`는 `elements: []`, `modelCues: []`인 퇴역 shell이다.
- 실제 Product cue 44개는 catalog 등록·non-empty 상태다.
- 현재 V1 alias는 휠윈드 2, 도넛 3, 방어구 파괴 돌진 1의 6쌍이다.
- `FRONT_BACK_FRONT` V0는 108 elements지만 V1 파일/alias는 없다.

## 구현

1. `VALTAN_PRODUCT_EFFECT_CUE_VIEW`에 optional V1 asset ID를 전달한다.
2. Saved Unified Effects에 같은 Product source/timing을 쓰는 `[V0]`와 `[V1]` 행을 각각 만든다.
3. Product/Reference Open·Play 전에 document를 lazy parse하고 `Validate_Drawable`을 fail-closed로 확인한다.
4. 빈 shell이면 이전 Current Effect와 preview를 보존하고 정확한 non-drawable 이유를 표시한다.
5. 한 번 non-drawable로 판정된 행은 Open/Play를 비활성화한다.
6. 캐릭터 direct-authored index 수정이 main에 병합되면 그 커밋을 rebase-consume한다.

## 검증

- Valtan Saved Rows focused tests
- 빈 CENTER Open/Play의 이전 document 보존 계약
- V0/V1 alias 6쌍과 동일 Product source projection
- Effect publisher Validate/Publish
- Debug/Release Client build 및 focused Effect render harness
- 사용자가 휠윈드/도넛/돌진 V0·V1을 직접 Open/Play하여 육안 판정

## 범위 밖

- 퇴역 shell 57개의 과거 요소 일괄 복원
- `FRONT_BACK_FRONT` 108행의 새 V1 materialization
- 사용자 관찰 전 visual/Product PASS 선언

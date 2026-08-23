# Valtan Effect Tool V0/V1 Audition Fix 결과

## 결론

발탄 Product V0는 V1 작업으로 삭제되거나 교체되지 않았다. 선언된 44개 Product cue를 전수
검사한 결과 source/runtime catalog와 published document가 모두 일치하며 44개 문서가 전부
non-empty다. Effect Tool에서 아무것도 보이지 않았던
`effect.valtan.center-grab-counter-64.center`는 Product V0가 아니라 element가 제거된 퇴역
stage evidence shell이었다.

Effect Tool은 이제 실제 Product cue를 `[V0]`로 표시하고 typed optional alias가 있는 Debug 행을
`[V1]`로 나란히 표시한다. 두 행은 같은 clip occurrence, source-local timing, transform,
attachment를 사용하되 실제로 여는 Effect document ID만 다르다. 빈 shell은 document load 전에
drawability를 fail-closed로 검사하므로 기존 Current Effect와 preview를 덮어쓰지 않는다.

## V0 전수 감사

- 44 cues / 24 patterns / 41 unique pattern-stage / 44 unique V0 Effect IDs
- authored JSON 누락 또는 ID 불일치: 0
- source/runtime EffectCatalog 누락 또는 중복: 0
- runtime hashed document 누락 또는 source/runtime semantic mismatch: 0
- zero-renderable Product cue: 0
- 총 661 elements: visible 657, hidden presentation 4
- V1 생성 이후 V0 source 변경: 0
- V1 receipt가 봉인한 V0 6개 source hash mismatch: 0
- All Effects의 33패턴 base Saved-row 투영: 106 rows = 49 non-empty candidates +
  57 literal empty shells. 이 수치는 전체 authored corpus 수가 아니라 해당 UI 투영 inventory다.

전체 encounter 33패턴 중 Product cue가 없는 coverage gap은 다음 9개다.

- `VALTAN_ARENA_BREAK_33`
- `VALTAN_ARENA_BREAK_84`
- `VALTAN_BIND_CHARGE_SMASH`
- `VALTAN_CENTER_GRAB_COUNTER_64`
- `VALTAN_DASH_CHARGE`
- `VALTAN_ENTRANCE_WHIRLWIND`
- `VALTAN_FOUR_PILLARS_105`
- `VALTAN_MAGIC_ORB_STAGGER_76`
- `VALTAN_STOMP`

이 목록은 V1 회귀가 아니라 boss-root Product cue coverage backlog다. 별도 reference/combat-object
연출까지 전혀 없다는 뜻은 아니다. legacy stage shell 97 IDs와 실제 44 Product IDs의 교집합은
0이다.

## 구현

- `VALTAN_PRODUCT_EFFECT_CUE_VIEW`가 typed `strV1EffectAssetId`를 보존한다.
- Pattern tree는 raw cue JSON 재해석 대신 `CValtanPatternEffectCueDocument::Load_Source`를 사용해
  Debug optional alias sidecar를 같은 typed 계약으로 적용한다.
- Saved Unified Effects가 V0 row와 V1 alias row를 같은 Product source 아래 투영한다.
- V1 Open/Play는 원본 cue를 복제하고 선택한 V1 asset ID를 복제 cue에 기록하므로 reload/restore의
  active-document identity도 일치한다.
- Product/reference Open과 Play는 `Refresh_UnifiedEffectCache`의 valid/drawable 판정을 document
  load와 preview mutation보다 먼저 수행한다.
- 명시적 Refresh는 이전 invalid/non-drawable 관찰 cache를 비워 수정된 문서를 다시 검사한다.

## 현재 V1 범위

현재 optional V1 alias는 정확히 6쌍이다.

- Whirlwind 2
- Magic Choice(도넛) 3
- Armor Break Opening 돌진 1

`FRONT_BACK_FRONT` 3연 공격은 V0 108 elements가 존재하지만 V1 alias/document는 아직 없다.

## 자동 검증

- `py -3 Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`: 14 tests PASS
- Client x64 Debug build: PASS
- Client x64 Release build: PASS
- `EffectRenderContractHarness` x64 Debug/Release: PASS
  - material bindings 171/171
  - representative V1 registry bindings 131
  - actual compiled-adapter draws 8
  - representative V1 actual draws 6
- 물리 정본 `C:\Users\user\Desktop\LostArk`의
  `Publish-Effects.ps1 -Mode Validate`: PASS
  - Effect catalog 156 entries
  - material-program bindings 171
  - registry-bound audition effects 5
- 이 브랜치의 별도 worktree checkout에서는 기존 4캐릭터
  `effect.dimensionmaster.skill.2050180.v1.unified` source seal이 줄바꿈 byte 차이로 중단됐지만,
  같은 revision의 물리 정본 검증은 위 수치로 통과했다. 이번 발탄 UI 변경은 Effect data를
  수정하지 않는다.
- 기존 Valtan generated-artifact byte-current tests 2개는 Windows CRLF checkout 차이로 실패했으며
  semantic/data 감사 결과와 분리한다.

공용 Effect Tool direct-authored index 수정 PR #182와 그 물리 정본 빌드 증거 PR #184를 먼저
소비한 최신 `main` 위에서 이 변경을 재검증했다.

## 수동 검증 대기

Client와 UI는 사용자가 직접 실행하고 최종 visual fidelity를 판정한다. 병합·정본 빌드 뒤
All Effects에서 실제 Product 패턴을 열어 `[V0]`와 `[V1]`을 각각 Open/Play하고, 빈
`CENTER_GRAB_COUNTER_64 / CENTER` 행을 눌러도 이전 Current Effect가 유지되는지 확인한다.

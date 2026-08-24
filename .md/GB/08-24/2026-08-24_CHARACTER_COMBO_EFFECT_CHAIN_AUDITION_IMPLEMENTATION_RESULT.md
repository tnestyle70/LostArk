# 캐릭터 COMBO Effect 전체 연계 오디션과 차원술사 BA 재사용 연결 구현 결과

## 1. 완료 상태

차원술사 LMB `2050010`의 네 Server combo stage를 `_01 / _01 / _03 / _01`로 연결했고,
Product Effect cue는 `ba2 / ba2 / ba3 / ba2`로 정리했다. 도화가 R `31210`의 기존 세 Server input
stage는 변경하지 않고, Effect Tool에서 실제 입력 경계를 압축해 세 공격 occurrence를 한 타임라인으로
검증하는 `Play Buffered Combo Audition`을 추가했다.

제품 런타임의 combo stage 권위는 계속 Server에 있다. 이 변경은 Client가 LMB 또는 R 한 번으로 다음
stage를 자동 구매하게 만들지 않는다. 실제 월드에서는 차원술사 LMB 네 번, 도화가 R 세 번이 각각
Server 승인 stage를 진행한다.

## 2. 차원술사 데이터 결과

| Server stage | 애니메이션 | Product Effect | buffered 시작 시각 |
|---|---|---|---:|
| BA1 | `pc_sp_m_00_sk_att_battle_1_01` | `effect.dimensionmaster.skill.2050010.ba2.unified` | 0 ms |
| BA2 | `pc_sp_m_00_sk_att_battle_1_01` | `effect.dimensionmaster.skill.2050010.ba2.unified` | 276 ms |
| BA3 | `pc_sp_m_00_sk_att_battle_1_03` | `effect.dimensionmaster.skill.2050010.ba3.unified` | 545 ms |
| BA4 | `pc_sp_m_00_sk_att_battle_1_01` | `effect.dimensionmaster.skill.2050010.ba2.unified` | 1039 ms |

비최종 경계 `276 + 269 + 494ms`와 최종 stage `1700ms`를 합친 전체 저작 오디션은 `2739ms`다.
BA1·BA2·BA4의 `_01`은 vector에서 서로 다른 occurrence로 유지되므로 같은 clip 이름이 합쳐지지 않는다.
기존 ba1/ba4 authored Effect 파일은 이번 변경에서 삭제하지 않았고 Product cue reference만 제거했다.

## 3. 도화가 R 결과

기존 binding은 다음과 같다.

```text
R stage 1: sdm_sk_skykongkong_01 -> sdm_sk_skykongkong_03
R stage 2: sdm_sk_skykongkong_01
R stage 3: sdm_sk_skykongkong_02
```

Buffered 오디션은 비최종 stage를 각각 `600ms`에서 자르므로 첫 stage의 no-next-input recovery `_03`에
들어가지 않고 `_01 -> _01 -> _02`가 `1733ms` 동안 이어진다. 실제 제품에서는 R을 세 번 입력해야
동일한 세 Server stage가 진행된다.

## 4. Effect Tool 동작

All Effects의 COMBO Skill 최상위 행에 `Play Buffered Combo Audition`이 추가됐다.

- 기존 `Play Full Effect`: 선택 Product cue와 그 stage clip 하나만 정확히 재생한다.
- `Play Buffered Combo Audition`: 모든 combo stage 애니메이션을 Server `comboAdvanceMs`와 최종
  `actionDurationMs`로 이어서 재생한다.
- 선택 Effect는 하나의 occurrence로 유지한다. 대신 선택 cue의 stage/clip 누적 offset을 적용해 BA3
  Effect가 BA1 시점에 뜨지 않도록 했다.
- 마지막 애니메이션 pose는 전체 combo 끝에서 멈추고, Effect가 더 길면 Tool wall clock이 자연 tail을
  끝까지 진행한다. 전역 Loop는 그 뒤 전체 Effect 타임라인을 다시 시작한다.
- ACTION_FACING은 숨겨진 stage 1에서 고정하지 않고 선택 occurrence가 처음 보이는 pose에서 확정한다.
- 시작 실패 시 이전 target model, Product/Valtan preview, element/group isolation, action-facing,
  All Effects class/authoring domain, Artist F reconstructed preview와 재생 시간을 복원한다.

`PlayerSkillCatalog`은 `comboStages`의 다섯 timing field를 임시 catalog에 exact parse·validate하고 전체
문서가 유효할 때만 commit한다. Effect lifetime으로 Server 입력 경계를 추측하지 않는다.

## 5. 자동 검증

다음 검증을 실행했고 모두 통과했다.

- Focused Python unittest: 18 tests PASS
  - 차원술사 반복 clip과 두 Product Effect cue
  - PlayerSkillCatalog combo timing parse/rollback 구조
  - 도화가 R 세 stage, occurrence offset, 마지막 Effect tail, preview rollback
- `Publish-GameplayBalance.ps1 -Mode Validate`
  - 6 profiles, 231 skills, 109 damage profiles, 34 Valtan patterns, 131 stages, 67 debug occurrences
- `Publish-Effects.ps1 -Mode Validate`
  - 161 Effect catalog entries, 171 material-program bindings, 5 registry-bound audition effects
- Client x64 Debug build PASS
- Client x64 Release build PASS
- `git diff --check` PASS

빌드에는 기존 C4819 인코딩 경고와 Release DirectXTK LNK4099 PDB 경고가 남지만 신규 오류는 없다.

## 6. 사용자 수동 검증 경로

1. Server + Client를 사용자가 직접 실행한다.
2. F1 → Effect Tool → All Effects로 이동한다.
3. `Dimension Master` → LMB 기본 공격 또는 `Artist` → R 필법:콩콩이를 펼친다.
4. 원하는 Product occurrence의 Saved Effect를 연다.
5. Skill 최상위의 `Play Buffered Combo Audition`을 누른다.
6. 차원술사는 `_01 -> _01 -> _03 -> _01`, 도화가는 `_01 -> _01 -> _02` 순서와 선택 Effect의 실제
   stage 시각을 확인한다.
7. 실제 월드 제품 동작은 차원술사 LMB 네 번, 도화가 R 세 번을 입력해 별도로 확인한다.

Client 화면의 최종 visual fidelity는 사용자 판정 대기 상태다. 에이전트는 자동 실행이나 육안 PASS를
기록하지 않았다.

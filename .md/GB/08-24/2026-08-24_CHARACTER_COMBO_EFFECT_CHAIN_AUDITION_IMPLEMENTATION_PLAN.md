# 캐릭터 COMBO Effect 전체 연계 오디션과 차원술사 BA 재사용 연결 구현 계획

## 1. 현재 정본과 이번 변경 경계

기준선은 `origin/main@af8a8905`다. 제품 전투의 COMBO stage 전환은 계속 Server가
`PlayerSkills.json.comboStages`와 반복 입력으로 결정한다. Effect Tool이 한 번의 Product Play로
다음 stage를 구매하거나 Client가 combo stage를 자체 증가시키지 않는다.

이번 변경은 다음 두 계약만 수정한다.

1. 차원술사 `2050010`은 BA1·BA2·BA4에서
   `pc_sp_m_00_sk_att_battle_1_01`과
   `effect.dimensionmaster.skill.2050010.ba2.unified`를 재사용하고, BA3만
   `pc_sp_m_00_sk_att_battle_1_03`과
   `effect.dimensionmaster.skill.2050010.ba3.unified`를 사용한다.
2. Effect Tool의 Skill 최상위 행에 `Play Buffered Combo Audition`을 추가한다. 이 버튼만
   비최종 stage를 `comboAdvanceMs`, 최종 stage를 `actionDurationMs`까지 재생하는 저작용 전체
   연계를 만든다. 기존 `Play Full Effect`는 선택한 cue의 한 stage만 재생한다.

도화가 R `31210`은 이미 Server COMBO 3단계와 세 번의 R 입력을 사용한다. Stage 1의
`_01 -> _03`, Stage 2의 `_01`, Stage 3의 `_02` Product binding은 변경하지 않는다. 빠른 R 입력
오디션에서는 두 비최종 stage가 각각 `600ms`에 넘어가므로 Stage 1 recovery `_03`은 생략되고,
`_01`, `_01`, `_02`의 세 action occurrence가 순서대로 보인다.

BA1/BA4 authored Effect 문서는 이번 변경에서 삭제하지 않는다. cue reference만 빠진 상태로 남기며,
향후 reference-aware Effect 퇴역 작업에서 별도로 처리한다.

## 2. 수정 파일과 한 줄 책임

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json` | BA1·2·4의 ordered clip을 `_01` 재사용으로 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents` | `_01 -> ba2`, `_03 -> ba3` Product cue 두 개만 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Client/Public/PlayerSkillCatalog.h` | Server 저작 combo stage timing의 Client read-only projection 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Client/Private/PlayerSkillCatalog.cpp` | comboStages를 exact parse·validate한 뒤 catalog와 함께 commit |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Client/Public/Effect_Tool.h` | 명시적 buffered combo audition 상태와 함수 계약 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Client/Private/Effect_Tool.cpp` | Skill-root 버튼, synthetic clip plan, 기존 Product Play 격리 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Tools/EffectPipeline/test_dimensionmaster_2050010_stage_split.py` | 반복 clip·두 Product cue·기존 Server timing을 고정 |
| 추가 | `C:/Users/user/Desktop/LostArk-character-combo-chain/Tools/EffectPipeline/test_player_skill_catalog_combo_timings.py` | timing projection과 fail-closed parser 구조 검증 |

새 C++ 파일은 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## 3. PlayerSkillCatalog 계약

`PLAYER_COMBO_STAGE_TIMING`은 다음 다섯 값을 읽기 전용으로 보존한다.

```text
actionDurationMs
hitTimeMs
comboAdvanceMs
inputOpenMs
inputCloseMs
```

이 값은 Effect Tool의 synthetic audition에만 사용하며 `CCharacter`, 입력 처리, Server snapshot의
combo stage를 변경하지 않는다. Parser는 기존 전체 문서를 임시 vector에 구성한 뒤 성공할 때만
`g_Skills`와 교체하므로 한 stage가 malformed여도 기존 catalog를 유지한다.

## 4. Effect Tool 전체 연계 호출 흐름

```text
All Effects Skill root
-> Play Buffered Combo Audition
-> 현재 class/skill의 PlayerSkills comboStages 확인
-> skillbindings의 stage별 ordered clip 확인
-> 비최종 stage는 comboAdvanceMs까지만 clip source window를 소비
-> 최종 stage는 actionDurationMs까지 소비
-> 반복 clip 이름도 stage occurrence별 vector row로 유지
-> 선택 Product cue를 앞선 Server stage/clip 누적 offset에 배치
-> synchronized animation sequence 시작
-> 마지막 pose를 고정하고 선택 Effect의 자연 tail이 끝날 때까지 Tool wall clock 진행
```

기존 cue 행의 `Play Full Effect`는 계속 `Try_SelectProductCue`로 들어가 한 Product cue와 한 ordered
clip만 재생한다. 전체 연계 상태는 별도 bool/class/skill/duration으로 소유하며, 실패 시 이전 preview를
깨뜨리지 않고 status에 원인을 남긴다. 이전 preview의 target, isolation, ACTION_FACING, Artist F
reconstructed runtime도 rollback 범위다. 이 기능은 Server Play가 아니라 `Synthetic / Buffered` 저작용
오디션이다. 선택 Effect 문서는 한 occurrence만 재생하지만 BA3처럼 뒤 stage의 cue는 실제 누적 stage
offset 이후에 보인다.

## 5. 데이터 연결 결과

```text
DimensionMaster LMB stage 1 -> _01 -> ba2
DimensionMaster LMB stage 2 -> _01 -> ba2
DimensionMaster LMB stage 3 -> _03 -> ba3
DimensionMaster LMB stage 4 -> _01 -> ba2

Artist R input 1 -> existing stage 1
Artist R input 2 -> existing stage 2
Artist R input 3 -> existing stage 3
```

차원술사 `comboAdvanceMs 276/269/494`와 최종 `actionDurationMs 1700`, 도화가 R의
`600/600/533` 빠른 연계 경계는 이번 변경에서 수정하지 않는다.

## 6. 검증

1. JSON parse와 animevents declared row count를 확인한다.
2. `test_dimensionmaster_2050010_stage_split.py`를 실행한다.
3. PlayerSkillCatalog timing focused test와 Effect Tool audition focused test를 실행한다.
4. `Publish-GameplayBalance.ps1 -Mode Validate`와 `Publish-Effects.ps1 -Mode Validate`를 실행한다.
5. Client x64 Debug와 Release를 빌드한다.
6. `git diff --check`를 실행한다.
7. 사용자가 Server + Client를 직접 실행해 차원술사 LMB 네 번과 도화가 R 세 번, Effect Tool의
   `Play Buffered Combo Audition`을 육안 확인한다. 에이전트는 visual PASS를 대신 기록하지 않는다.
